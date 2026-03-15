#include "spg_redis_command_runtime.h"

#include "spg_redis_reply.h"
#include <spg_redis_alias.h>
#include <spg_redis_channel.h>
#include <spg_redis_commands.h>

using namespace boost;

namespace spg::redis {

CommandRuntime::CommandRuntime()
{
    for (auto& shard : shards_) {
        shard.thread = std::jthread{ [this, &shard] { shard.context.run(); } };
        co_spawn(shard.context, shard.do_command(), asio::detached);
    }
}

auto CommandRuntime::Shard::do_command() -> boost::asio::awaitable<void>
{
    while (true) {
        auto [ec, request] = co_await request_channel.async_receive(use_nothrow_awaitable);
        if (ec)
            co_return;

        BatchReply batch_reply;
        batch_reply.reserve(request.batch_commands.size());

        auto& db = database[local_db_index(request.index)];
        for (auto& command : request.batch_commands)
            batch_reply.emplace_back(commands::dispatch(db, command));

        if (auto response_channel = request.response_channel.lock())
            auto [_] =
                co_await response_channel->async_send(ErrorCode{}, std::move(batch_reply), use_nothrow_awaitable);
    }
}

} // namespace spg::redis
