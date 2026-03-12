#include "spg_http_server.h"

#include <spg_http_ssl_session.h>

#include <boost/asio/detached.hpp>
#include <boost/asio/strand.hpp>

using namespace boost::asio;

namespace sponge::http {

HttpServer::HttpServer(const Config& config)
    : ssl_context_{ config.ssl.enable_tls_v13 ? ssl::context::tlsv13_server : ssl::context::tlsv12_server }
    , io_context_pool_{ config.common.io_threads }
    , acceptor_{ main_context_ }
    , worker_pool_{ config.common.worker_threads }
{
    ssl_context_.use_certificate_chain_file("server.crt");
    ssl_context_.use_private_key_file("server.key", ssl::context::pem);

    ip::tcp::resolver resolver{ acceptor_.get_executor() };
    std::string_view address = config.network.address;
    auto port = std::to_string(config.network.port);
    ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
}

void HttpServer::run()
{
    co_spawn(main_context_, do_accept(), detached);
    main_context_.run();
}

auto HttpServer::do_accept() -> awaitable<void>
{
    while (true) {
        auto [ec, socket] = co_await acceptor_.async_accept(use_nothrow_awaitable);
        if (ec)
            co_return;

        co_spawn(io_context_pool_.get_io_context(), do_session(std::move(socket)), detached);
    }
}

auto HttpServer::do_session(Socket socket) -> boost::asio::awaitable<void>
{
    SSLSocket secure_stream{ std::move(socket), ssl_context_ };
    SSLSession session{ worker_pool_, std::move(secure_stream) };
    co_await session.run();
}

} // namespace sponge::http
