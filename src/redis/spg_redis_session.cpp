#include "spg_redis_session.h"

#include "spg_redis_channel.h"
#include <spg_base_utility.h>
#include <spg_redis_alias.h>
#include <spg_redis_command_runtime.h>
#include <spg_redis_commands.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/detail/error_code.hpp>

#include <cstring>

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
        if (ec)
            co_return;

        auto total_len = write_idx_ + n;
        std::span<const char> buffer_view{ buffer_.data(), total_len };

        if (!(co_await consume_buffer(request, *response_channel, buffer_view)))
            co_return;
    }
}

auto Session::write_batch_response(BatchReply batch_reply) -> boost::asio::awaitable<bool>
{
    for (auto& reply : batch_reply)
        write_context_.append_reply(reply);

    auto [ec, _] = co_await asio::async_write(socket_, write_context_.buffers(), use_nothrow_awaitable);
    write_context_.reset();
    if (ec)
        co_return false;

    co_return true;
}

auto Session::send_request(Request& request, ResponseChannel& response_channel) -> boost::asio::awaitable<bool>
{
    request.index = index_;
    request.batch_commands = std::move(batch_commands_);
    batch_commands_.clear();
    // 向DB线程发起请求
    auto& request_channel = runtime_.get_channel(index_);
    auto [send_ec] = co_await request_channel.async_send(ErrorCode{}, request, use_nothrow_awaitable);
    if (send_ec)
        co_return false;

    // 拿到返回的结果
    auto [recv_ec, batch_reply] = co_await response_channel.async_receive(use_nothrow_awaitable);
    if (recv_ec)
        co_return false;

    co_return co_await write_batch_response(std::move(batch_reply));
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

        // 如果是select命令，直接在Session线程处理，不发给DB线程
        // 如果此时有积累的命令，先把它们发给DB线程处理，再切换DB
        if ((*res)[0] == "select") {
            if (!batch_commands_.empty()) {
                if (!co_await send_request(request, response_channel))
                    co_return false;
            }

            assert(batch_commands_.empty());
            write_context_.append_reply(select(*res));
        }
        else {
            batch_commands_.emplace_back(std::move(*res));
        }

        parser_.reset();
    }

    if (!batch_commands_.empty()) {
        if (!co_await send_request(request, response_channel))
            co_return false;
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

auto Session::select(std::span<std::string> args) -> Reply
{
    if (args.size() != 2)
        return wrong_number_of_arguments_error("select");

    auto select_index = base::numeric_cast<Index>(args[1]);
    if (!select_index)
        return out_of_range;

    if (*select_index >= CommandRuntime::db_count)
        return out_of_range;

    index_ = *select_index;
    return ok;
}

} // namespace sponge::redis
