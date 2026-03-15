#ifndef SPONGE_HTTP_SSL_SESSION_H
#define SPONGE_HTTP_SSL_SESSION_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace spg::http {

class SSLSession {
public:
    using Context = boost::asio::io_context;
    using Socket = boost::asio::ip::tcp::socket;
    using SSLSocket = boost::asio::ssl::stream<Socket>;
    using Strand = boost::asio::strand<Context>;
    using ThreadPool = boost::asio::thread_pool;
    static constexpr auto use_nothrow_awaitable = boost::asio::as_tuple(boost::asio::use_awaitable);

    SSLSession(ThreadPool& worker_pool, SSLSocket socket);

    auto run() -> boost::asio::awaitable<void>;

private:
    ThreadPool& worker_pool_;
    SSLSocket socket_;
};

} // namespace spg::http

#endif // SPONGE_HTTP_SSL_SESSION_H
