#ifndef SPONGE_CORE_SESSION_H
#define SPONGE_CORE_SESSION_H

#include "spg_base_io_context_pool.h"
#include <spg_core_application_context.h>
#include <spg_core_database.h>

#include <boost/asio.hpp>

#include <chrono>

namespace spg::core {

class Session {
public:
    using Socket = boost::asio::ip::tcp::socket;
    using Size = std::size_t;

    Session(Socket socket, ApplicationContext& context, Size id);

    auto work() -> boost::asio::awaitable<void>;

private:
    using TimePoint = std::chrono::steady_clock::time_point;
    using Clock = std::chrono::steady_clock;
    using Seconds = std::chrono::seconds;

    static constexpr Seconds TIMEOUT{ 10 };

    Socket socket_;
    ApplicationContext& context_;
    Size id_;
    TimePoint deadline_;

    auto do_work() -> boost::asio::awaitable<void>;

    auto watchdog() -> boost::asio::awaitable<void>;

    auto execute_pipeline(std::vector<std::vector<std::string_view>>& commands, std::string& response)
        -> boost::asio::awaitable<void>;

    auto shard(Size id) -> Database& { return context_.shards[id]; }

    auto shard() -> Database& { return context_.shards[id_]; }

    auto io_context() -> base::IOContextPool::Context& { return context_.io_context_pool[id_]; }

    auto io_context(Size id) -> base::IOContextPool::Context& { return context_.io_context_pool[id]; }
};

} // namespace spg::core

#endif // SPONGE_CORE_SESSION_H