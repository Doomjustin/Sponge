#ifndef SPONGE_REDIS_COMMAND_RUNTIME_H
#define SPONGE_REDIS_COMMAND_RUNTIME_H

#include <spg_redis_channel.h>
#include <spg_redis_database.h>

#include <boost/asio.hpp>

#include <thread>

namespace sponge::redis {

class CommandRuntime {
public:
    using Index = std::size_t;
    using IOContext = boost::asio::io_context;
    using WorkGuard = boost::asio::executor_work_guard<IOContext::executor_type>;
    static constexpr std::size_t db_count = 16;

    CommandRuntime();

    auto get_channel(Index index) -> RequestChannel& { return shards_[shard_index(index)].request_channel; }

private:
    static constexpr std::size_t shard_count = 4;
    static constexpr std::size_t local_db_count = db_count / shard_count;

    struct Shard {
        IOContext context{ 1 };
        RequestChannel request_channel{ context, 128 };
        WorkGuard work_guard{ context.get_executor() };

        std::jthread thread;
        std::array<Database, local_db_count> database;

        auto do_command() -> boost::asio::awaitable<void>;
    };

    std::array<Shard, shard_count> shards_;

    static auto shard_index(Index index) -> std::size_t { return index % shard_count; }

    static auto local_db_index(Index index) -> std::size_t { return index / shard_count; }
};

} // namespace sponge::redis

#endif // SPONGE_REDIS_COMMAND_RUNTIME_H
