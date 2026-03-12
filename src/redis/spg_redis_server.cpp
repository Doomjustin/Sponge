#include "spg_redis_server.h"

#include "spg_redis_session.h"
#include <spg_redis_resp.h>

#include <boost/asio.hpp>

using namespace boost;

namespace sponge::redis {

Server::Server(std::string_view address, Port port)
{
    asio::ip::tcp::resolver resolver(context_);
    asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, std::to_string(port)).begin();
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
}

void Server::run()
{
    co_spawn(context_, do_accept(), asio::detached);
    context_.run();
}

auto Server::do_accept() -> asio::awaitable<void>
{
    while (true) {
        auto [ec, socket] = co_await acceptor_.async_accept(use_nothrow_awaitable);
        if (ec)
            co_return;

        co_spawn(io_context_pool_.get_io_context(), do_session(std::move(socket)), asio::detached);
    }
}

auto Server::do_session(Socket socket) -> asio::awaitable<void>
{
    Session session{ std::move(socket), command_runtime_ };
    co_await session.run();
}

} // namespace sponge::redis
