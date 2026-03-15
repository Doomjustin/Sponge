#ifndef SPONGE_CORE_SERVER_H
#define SPONGE_CORE_SERVER_H

#include "spg_core_application_context.h"
#include <spg_base_io_context_pool.h>
#include <spg_core_database.h>

#include <boost/asio.hpp>

#include <cstdint>

namespace spg::core {

class Server {
public:
    using Port = std::uint16_t;
    using Socket = boost::asio::ip::tcp::socket;
    using Size = std::size_t;

    Server(std::string_view address, Port port, Size size);

    void start();

private:
    std::string address_;
    Port port_;
    ApplicationContext context_;

    static void graceful_stop(boost::asio::io_context& context);

    auto listener(boost::asio::io_context& context, Size id) -> boost::asio::awaitable<void>;

    auto do_session(Socket socket, Database& shard, Size id) -> boost::asio::awaitable<void>;
};

} // namespace spg::core

#endif // SPONGE_CORE_SERVER_H
