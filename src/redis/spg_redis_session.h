#ifndef SPONGE_REDIS_SESSION_H
#define SPONGE_REDIS_SESSION_H

#include "spg_redis_command_runtime.h"
#include <spg_redis_channel.h>
#include <spg_redis_resp.h>

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>

#include <array>
#include <memory>

namespace sponge::redis {

class Session {
public:
    using Arguments = commands::Arguments;
    using Response = commands::Response;
    using Index = std::size_t;
    using Size = std::size_t;
    using Socket = boost::asio::ip::tcp::socket;

    explicit Session(Socket socket, CommandRuntime& runtime);

    auto run() -> boost::asio::awaitable<void>;

private:
    using ResponseChannelPtr = std::shared_ptr<ResponseChannel>;

    static constexpr std::size_t BUFFER_SIZE = 8192;

    Socket socket_;
    CommandRuntime& runtime_;
    std::array<char, BUFFER_SIZE> buffer_{};
    Size write_idx_ = 0;
    Index index_ = 0;
    RESPParser parser_{};

    auto write_response(const Response& response) -> boost::asio::awaitable<bool>;

    auto handle_command(Request& request, ResponseChannel& response_channel, Arguments arguments)
        -> boost::asio::awaitable<bool>;

    auto consume_buffer(Request& request, ResponseChannel& response_channel, std::span<const char> buffer_view)
        -> boost::asio::awaitable<bool>;

    auto preserve_remaining_buffer(std::span<const char> buffer_view) -> void;

    auto select(const Arguments& args) -> Response;
};

} // namespace sponge::redis

#endif // SPONGE_REDIS_SESSION_H
