#ifndef SPONGE_HTTP_SERVER_H
#define SPONGE_HTTP_SERVER_H

#include <spg_base_io_context_pool.h>
#include <spg_http_config.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <cstddef>
#include <cstdint>

namespace sponge::http {

class HttpServer {
public:
    using Context = boost::asio::io_context;
    using ThreadPool = boost::asio::thread_pool;
    using Acceptor = boost::asio::ip::tcp::acceptor;
    using Port = std::uint16_t;
    using Size = std::size_t;
    using Endpoint = boost::asio::ip::tcp::endpoint;
    using Socket = boost::asio::ip::tcp::socket;
    using SSLContext = boost::asio::ssl::context;
    using SSLSocket = boost::asio::ssl::stream<Socket>;
    using Strand = boost::asio::strand<Context>;
    static constexpr auto use_nothrow_awaitable = boost::asio::as_tuple(boost::asio::use_awaitable);

    HttpServer(const Config& config);

    void run();

private:
    Context main_context_;
    SSLContext ssl_context_;
    base::IOContextPool io_context_pool_;
    ThreadPool worker_pool_;
    Acceptor acceptor_;

    auto do_accept() -> boost::asio::awaitable<void>;

    auto do_session(Socket socket) -> boost::asio::awaitable<void>;
};

} // namespace sponge::http

#endif // SPONGE_HTTP_SERVER_H
