#include "spg_http_ssl_session.h"

#include <print>

#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

using namespace boost;

namespace sponge::http {

SSLSession::SSLSession(ThreadPool& worker_pool, SSLSocket socket)
    : worker_pool_{ worker_pool }
    , socket_{ std::move(socket) }
{
}

auto SSLSession::run() -> asio::awaitable<void>
{
    auto [ec] = co_await socket_.async_handshake(asio::ssl::stream_base::server, use_nothrow_awaitable);
    if (ec) {
        std::println("SSL 连接异常: {}", ec.what());
        co_return;
    }

    std::println("SSL 握手成功，安全通道建立");

    beast::tcp_stream stream{ std::move(socket_) };
    beast::flat_buffer buffer{};
    using namespace std::literals;
    stream.expires_after(30s);

    beast::http::request<beast::http::string_body> request{};
    auto [read_ec, n] = co_await beast::http::async_read(stream, buffer, request, use_nothrow_awaitable);
    if (read_ec) {
        std::println("读取 HTTP 请求异常: {}", read_ec.what());
        co_return;
    }
}

} // namespace sponge::http
