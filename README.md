# SPonGe
我用来验证一些想法和思路的仓库

## 高性能的关键点
1. nothing shard -> 无锁
2. pipeline
3. Scatter-Gather
3. full duplex

1. 使用unordered_flat_map作为存储
非常抽象的是，release版本的SET只有163万QPS左右，但是p50数据提升巨大。等于说处理的越慢，处理的越快

| 数据块 |          | Sponge     | Redis 本体  |
| ------ | ------   | --------  | ---------   |
| 1024   | SET(QPS) | 约255.8万  | 约112.1万   |
| 1024   | GET(QPS) | 约196.7万  | 约185.2万   |
| 不指定 | SET(QPS) | 约425.5万  | 约257.7万   |
| 不指定 | GET(QPS) | 约442.5万  | 约304.9万   |

虽然还没有实现AOF/RDB，性能量级来说已经和redis相当，我们有了一个坚实的基础

详细测试数据见 [doc/test_result.md](doc/test_result.md)

## 处理的越快，处理的越慢
都是用的同样的测试命令

```bash
throughput summary: 786163.56 requests per second
latency summary (msec):
     avg       min       p50       p95       p99       max
    0.178     0.056     0.095     0.799     1.087     1.911
```

```bash
Summary:
  throughput summary: 1908397.00 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.212     0.048     0.167     0.471     1.319     2.679
```

## release 编译
```bash
CC=/usr/local/bin/clang CXX=/usr/local/bin/clang++ cmake -S . -B release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/home/doom/dependency/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux && cmake --build release
```