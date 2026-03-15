#ifndef SPONGE_REDIS_SERVER_H
#define SPONGE_REDIS_SERVER_H

#include "spg_redis_command_runtime.h"
#include <spg_base_io_context_pool.h>
#include <spg_redis_commands.h>

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>

#include <cstddef>
#include <cstdint>

namespace spg::redis {

class Server {
public:
    using Context = boost::asio::io_context;
    using ThreadPool = boost::asio::thread_pool;
    using Acceptor = boost::asio::ip::tcp::acceptor;
    using Port = std::uint16_t;
    using Size = std::size_t;
    using Endpoint = boost::asio::ip::tcp::endpoint;
    using Socket = boost::asio::ip::tcp::socket;

    static constexpr auto use_nothrow_awaitable = boost::asio::as_tuple(boost::asio::use_awaitable);
    static constexpr int default_port = 26379;
    static constexpr int io_threads = 4;
    static constexpr int worker_threads = 4;
    static constexpr std::string_view default_address = "127.0.0.1";

    explicit Server(std::string_view address = default_address, Port port = default_port);

    void run();

private:
    Context context_{ 1 };
    base::IOContextPool io_context_pool_{ io_threads };
    ThreadPool worker_pool_{ worker_threads };
    Acceptor acceptor_{ context_ };
    CommandRuntime command_runtime_;

    auto do_accept() -> boost::asio::awaitable<void>;

    auto do_session(Socket socket) -> boost::asio::awaitable<void>;
};

} // namespace spg::redis

#endif // SPONGE_REDIS_SERVER_H
