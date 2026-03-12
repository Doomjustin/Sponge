#include "spg_redis_session.h"

#include <spg_base_utility.h>
#include <spg_redis_alias.h>
#include <spg_redis_command_runtime.h>
#include <spg_redis_commands.h>

#include <boost/asio/write.hpp>
#include <boost/system/detail/error_code.hpp>

#include <cstring>
#include <print>

using namespace boost;

namespace sponge::redis {

Session::Session(boost::asio::ip::tcp::socket socket, CommandRuntime& runtime)
    : socket_{ std::move(socket) }
    , runtime_{ runtime }
{
}

auto Session::run() -> boost::asio::awaitable<void>
{
    ResponseChannelPtr response_channel = std::make_shared<ResponseChannel>(socket_.get_executor(), 8);
    Request request{ .response_channel = response_channel };

    while (true) {
        auto socket_buffer = asio::buffer(buffer_.data() + write_idx_, buffer_.size() - write_idx_);
        auto [ec, n] = co_await socket_.async_read_some(socket_buffer, use_nothrow_awaitable);
        if (ec) {
            std::println("socket read closed.");
            co_return;
        }

        auto total_len = write_idx_ + n;
        std::span<const char> buffer_view{ buffer_.data(), total_len };

        if (!(co_await consume_buffer(request, *response_channel, buffer_view)))
            co_return;
    }
}

auto Session::write_response(const Response& response) -> boost::asio::awaitable<bool>
{
    auto [write_ec, writes] = co_await asio::async_write(socket_, asio::buffer(response), use_nothrow_awaitable);
    if (write_ec) {
        std::println("socket write error");
        co_return false;
    }

    co_return true;
}

auto Session::handle_command(Request& request, ResponseChannel& response_channel, Arguments arguments)
    -> boost::asio::awaitable<bool>
{
    if (arguments[0] == "select")
        co_return co_await write_response(select(arguments));

    request.index = index_;
    request.arguments = std::move(arguments);

    // 向DB线程发起请求
    auto& request_channel = runtime_.get_channel(index_);
    auto [send_ec] = co_await request_channel.async_send(ErrorCode{}, request, use_nothrow_awaitable);
    if (send_ec) {
        std::println("request channel send error");
        co_return false;
    }

    // 拿到返回的结果
    auto [recv_ec, response] = co_await response_channel.async_receive(use_nothrow_awaitable);
    if (recv_ec) {
        std::println("response channel receive error");
        co_return false;
    }

    co_return co_await write_response(response);
}

auto Session::consume_buffer(Request& request, ResponseChannel& response_channel, std::span<const char> buffer_view)
    -> boost::asio::awaitable<bool>
{
    while (!buffer_view.empty()) {
        auto res = parser_.parse(buffer_view);
        if (!res) {
            if (res.error() == Status::Waiting) {
                preserve_remaining_buffer(buffer_view);
                co_return true;
            }

            co_return false;
        }

        if (!(co_await handle_command(request, response_channel, std::move(*res))))
            co_return false;

        parser_.reset();
    }

    write_idx_ = 0;
    co_return true;
}

auto Session::preserve_remaining_buffer(std::span<const char> buffer_view) -> void
{
    if (buffer_view.empty()) {
        write_idx_ = 0;
        return;
    }

    std::memmove(buffer_.data(), buffer_view.data(), buffer_view.size());
    write_idx_ = buffer_view.size();
}

auto Session::select(const Arguments& args) -> Response
{
    if (args.size() != 2)
        return "-ERR wrong number of arguments for 'select' command\r\n";

    auto select_index = base::numeric_cast<Index>(args[1]);
    if (!select_index)
        return "-ERR invalid argument for 'select' command\r\n";

    index_ = *select_index;
    return "+OK\r\n";
}

} // namespace sponge::redis
