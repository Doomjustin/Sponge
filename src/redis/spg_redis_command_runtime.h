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

    CommandRuntime();

    auto get_channel(Index index) -> RequestChannel& { return shards_[shard_index(index)].request_channel; }

private:
    struct Shard {
        IOContext context{ 1 };
        RequestChannel request_channel{ context, 128 };
        WorkGuard work_guard{ context.get_executor() };

        std::jthread thread;
        std::array<Database, 4> database;

        auto do_command() -> boost::asio::awaitable<void>;
    };

    static constexpr int db_count = 16;
    static constexpr int shard_count = 4;

    std::array<Shard, shard_count> shards_;

    static auto shard_index(Index index) -> std::size_t { return index % shard_count; }

    static auto local_db_index(Index index) -> std::size_t { return index % db_count; }
};

} // namespace sponge::redis

#endif // SPONGE_REDIS_COMMAND_RUNTIME_H
