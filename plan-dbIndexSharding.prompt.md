## Plan: 业务线程与 API 抽象

推荐把“业务处理”抽象成按 DB 分区串行执行的 BusinessShard，再在其外层放一个 BusinessRuntime/Dispatcher 负责按 selected_db 路由。这样实现上仍然是“业务状态不和 IO 混跑”，同时天然贴合你现在的模型：16 个 DB 固定分配给 4 个业务线程，命令集全局共享，只有 SELECT 会切换连接当前绑定的 DB。

**Steps**
1. 明确并发边界：IO 层只负责 accept、read、parse、write；业务层只负责 command dispatch、状态访问、业务计算、生成 response。这个边界应固定在 Session 与 BusinessRuntime 之间。
2. 定义统一消息模型：把 session 发给业务层的内容收敛成 RequestEnvelope，至少包含 request_id、session_id、index_、command、arguments 和 reply sink。这里的 index_ 就是连接当前选中的 DB。SELECT 这类会改变 DB 绑定的命令需要显式标记，不能当成普通数据命令处理。reply sink 不建议复用当前共享 channel，而应是 per-request 的单次回传句柄。*后续步骤依赖本步*
3. 抽象单个业务执行单元 BusinessShard：每个 shard 自己持有一个 io_context、一个线程、一个入站 channel、以及该 shard 负责的若干 DB 的独占业务状态；它串行消费消息并执行 handler。这个对象就是“单独 context+thread”的落点。*后续步骤依赖本步*
4. 抽象外层 BusinessRuntime：提供 submit/enqueue 接口，内部根据 index_ 做固定路由。对 16 个 DB / 4 个线程 的场景，可直接用静态映射，如 db_index / 4 或预生成 db_to_shard 表。后续即使线程数变化，也不改 session 层 API。*depends on 2,3*
5. 设计命令注册与执行 API：把现有 Commands 从 command -> void handler 提升为 command -> typed handler metadata，至少包含 command type、是否需要 DB 上下文、是否允许改变 session 状态、handler 函数。handler 建议返回 Response/Result，而不是直接操作 socket。*parallel with step 4 after step 2*
6. 设计 reply 回传机制：推荐 Session 在提交请求时创建一次性的 response receiver，然后 await 该请求自己的结果；不要让所有 session 共享一个请求/响应 channel，否则多连接下无法正确配对响应。*depends on 2*
7. 设计 DB 路由与 SELECT 语义：普通命令总是路由到 session 当前 selected_db 所属 shard；SELECT 不访问业务 shard 内部数据，而是先校验参数和目标 DB，再更新 session 本地 selected_db，随后后续请求自然路由到新的 shard。*depends on 4,5*
8. 设计取消、超时和背压：RequestEnvelope 需要 deadline/cancel token；BusinessRuntime 需要 channel 满载时的策略（拒绝、阻塞、降级）；Session 侧需要把业务超时映射为协议响应。*depends on 2,4*
9. 规划状态模型：每个 DB 属于唯一 shard，每个 shard 维护自己负责的若干 DB 容器；命令实现共享一套代码，但执行时显式接收当前 DB 句柄或 DBContext。跨 DB 命令如果后续出现，需要单独定义 coordinator 路径。*depends on 7*
10. 验证演进路径：先实现 4 shard 固定 DB 分配的端到端链路，再验证 SELECT 切库后请求是否进入新 shard，确保 session 层只维护 selected_db，不直接接触业务状态。*depends on 1-9*

**Relevant files**
- /home/doom/code/Sponge/src/redis/spg_redis_server.h — 当前 Server 已区分 accept context、io_context_pool 和 worker 对象，可作为引入 BusinessRuntime 的入口。
- /home/doom/code/Sponge/src/redis/spg_redis_session.cpp — 当前 Session 已负责 parse 和异步读写；这里适合改成“构造 RequestEnvelope + await per-request reply”的调用点。
- /home/doom/code/Sponge/src/redis/spg_redis_commands.h — 当前 Commands::Handler 过于薄，且 Worker 只有共享 channel；这里应升级为命令元数据、业务 runtime 接口和消息结构定义。
- /home/doom/code/Sponge/src/base/spg_base_io_context_pool.h — 已有成熟的 io context 池模式，BusinessShardPool 可以复用类似生命周期管理思路，但业务 shard 通常不建议 round-robin 抢占，而是固定路由。

**Verification**
1. 设计级验证：确认 session 层不直接接触业务状态，也不直接调用具体命令实现。
2. 并发级验证：在 2 个以上连接同时发送请求时，响应必须按 request_id 或私有 reply sink 正确回到各自 session，不能串包。
3. 扩展级验证：把 shard_count 从 1 调到 4 后，Session/Server 对外调用签名不变。
4. 正确性验证：相同 selected_db 的请求总是落到同一业务 shard，并保持串行执行；SELECT 成功后后续请求必须切换到新的 shard。
5. 容量验证：业务 channel 人为打满时，确认背压策略符合预期，客户端不会无限悬挂。

**Decisions**
- 包含范围：业务线程模型、消息边界、命令注册 API、按 DB 路由策略、响应回传模型。
- 明确不包含：具体 Redis 命令实现、HTTP 路由细节、持久化方案。
- 决策：当前业务分发按 selected_db 固定映射到 shard，不按 key 做 hash 分片。
- 决策：命令实现代码全局共享，不与 DB index 耦合；DB index 只决定执行时拿到哪个 DBContext。
- 决策：SELECT 是 session 状态切换命令，会更新连接本地 selected_db；网络编解码仍留在 session 层。
- 决策：响应回传使用 per-request reply handle，不使用当前共享 request/response channel 模式。

**Further Considerations**
1. 如果后续出现跨 DB 命令，需要提前定义 coordinator 路径，否则固定 DB 分区模型会缺少一致语义。
2. 如果未来同时服务 Redis 和 HTTP，两者最好共享同一套 BusinessRuntime，只在 session 层做不同协议适配。
