#include "spg_core_server.h"

#include <spg_core_session.h>

#include <boost/asio/use_awaitable.hpp>

#include <print>

using namespace boost;

namespace spg::core {

Server::Server(std::string_view address, Port port, Size size)
    : context_{ size }
    , address_{ address }
    , port_{ port }
{
}

void Server::start()
{
    for (int i = 0; i < context_.io_context_pool.size(); ++i) {
        auto& context = context_.io_context_pool.get_io_context();
        asio::co_spawn(context, listener(context, i), asio::detached);
    }

    graceful_stop(context_.io_context_pool.get_io_context());
}

void Server::graceful_stop(asio::io_context& context)
{
    asio::signal_set signals{ context, SIGINT, SIGTERM };

    std::promise<void> stop_promise;
    auto on_exit = [&stop_promise](auto, auto) {
        std::println("signal received, stopping...");
        stop_promise.set_value();
    };

    signals.async_wait(on_exit);
    stop_promise.get_future().wait();
}

auto Server::listener(boost::asio::io_context& context, Size id) -> boost::asio::awaitable<void>
{
    using reuse_port = asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT>;
    using Acceptor = asio::ip::tcp::acceptor;
    using Endpoint = asio::ip::tcp::endpoint;
    using Resolver = asio::ip::tcp::resolver;

    spg::core::Database db_shard{};

    Resolver resolver{ context };
    Endpoint endpoint = *resolver.resolve(address_, std::to_string(port_)).begin();

    Acceptor acceptor{ context };
    acceptor.open(endpoint.protocol());
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.set_option(reuse_port(true));
    acceptor.bind(endpoint);
    acceptor.listen();
    std::println("Listening on {}:{}.", endpoint.address().to_string(), endpoint.port());

    while (true) {
        auto socket = co_await acceptor.async_accept(asio::use_awaitable);
        asio::co_spawn(context, do_session(std::move(socket), db_shard, id), asio::detached);
    }
}

auto Server::do_session(Socket socket, Database& shard, Size id) -> boost::asio::awaitable<void>
{
    spg::core::Session session{ std::move(socket), context_, id };
    co_await session.work();
}

} // namespace spg::core
