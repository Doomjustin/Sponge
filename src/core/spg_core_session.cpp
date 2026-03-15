#include "spg_core_session.h"

#include <spg_base_io_context_pool.h>
#include <spg_core_application_context.h>
#include <spg_core_commands.h>
#include <spg_core_resp_parser.h>

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/beast.hpp>

#include <exception>
#include <print>

using namespace boost;

namespace spg::core {

Session::Session(Socket socket, ApplicationContext& context, Size id)
    : socket_{ std::move(socket) }
    , context_{ context }
    , id_{ id }
{
    for (int i = 0; i < BUFFER_COUNT; ++i)
        // buffers_[i].prepare(BUFFER_SIZE);
        buffers_[i] = boost::beast::flat_buffer{ BUFFER_SIZE };
}

auto Session::work() -> asio::awaitable<void>
{
    using namespace asio::experimental::awaitable_operators;

    deadline_ = Clock::now() + TIMEOUT;

    // 初始时通知执行命令的协程所有buffer都可用
    for (Size i = 0; i < BUFFER_COUNT; ++i)
        free_channel_.try_send(ErrorCode{}, i);

    co_await (reader() || writer() || watchdog());
}

auto Session::reader() -> boost::asio::awaitable<void>
{
    try {
        auto active_index = co_await free_channel_.async_receive(asio::use_awaitable);

        while (true) {
            auto& active_buffer = buffers_[active_index];

            // 贪婪读
            int read_budget = 8;
            while (read_budget-- > 0) {
                deadline_ = Clock::now() + TIMEOUT;

                auto n = co_await socket_.async_read_some(active_buffer.prepare(ONCE_READ_SIZE), asio::use_awaitable);
                active_buffer.commit(n);

                if (active_buffer.size() <= READ_ONCE_MAX_SIZE || socket_.available() == 0)
                    break;
            }

            auto buffer_seq = active_buffer.data();
            std::string_view query{ static_cast<const char*>(buffer_seq.data()), buffer_seq.size() };
            auto [commands, consumed_bytes] = parse_resp_batch(query);

            // 半包的话，下次继续沿着这个buffer读，直到读到完整的命令为止
            if (commands.empty())
                continue;

            // 交换buffer，避免拿到当前仍在使用的buffer索引
            auto next_index = co_await free_channel_.async_receive(asio::use_awaitable);
            while (next_index == active_index) {
                co_await free_channel_.async_send(ErrorCode{}, next_index, asio::use_awaitable);
                next_index = co_await free_channel_.async_receive(asio::use_awaitable);
            }

            auto& next_buffer = buffers_[next_index];
            auto leftover_bytes = active_buffer.size() - consumed_bytes;
            if (leftover_bytes > 0) {
                auto active_data = active_buffer.data();
                const char* leftover_data = static_cast<const char*>(active_data.data()) + consumed_bytes;
                // 将剩余的半包数据移动到下一个buffer的开头
                boost::asio::buffer_copy(next_buffer.prepare(leftover_bytes),
                                         asio::buffer(leftover_data, leftover_bytes));
                next_buffer.commit(leftover_bytes);
            }

            co_await ready_channel_.async_send(ErrorCode{}, std::move(commands), active_index, asio::use_awaitable);
            active_index = next_index;
        }
    }
    catch (const boost::system::system_error& e) {
        if (e.code() != asio::error::operation_aborted && e.code() != asio::error::eof &&
            e.code() != asio::error::connection_reset && e.code() != asio::error::broken_pipe &&
            e.code() != asio::experimental::error::channel_cancelled)
            std::println("Reader error: {}", e.what());
    }
    catch (std::exception& e) {
        std::println("Reader error: {}", e.what());
    }
}

auto Session::writer() -> boost::asio::awaitable<void>
{
    try {
        std::string response;
        response.reserve(1024 * 1024 * 1024); // 预留一些空间，避免每次append都扩容
        while (true) {
            auto [batch_commands, index] = co_await ready_channel_.async_receive(asio::use_awaitable);

            // 处理命令的结果会写到response里，最后一次性写回客户端
            co_await redis_controller_.execute(batch_commands, response);

            // for test
            // for (Size i = 0; i < batch_commands.size(); ++i)
            //     response.append("+OK\r\n"); // --- IGNORE ---

            // 这个buffer之前是reader在用的，现在已经处理完了，可以重置了
            buffers_[index].consume(buffers_[index].size());
            // 处理完一批命令后，通知reader协程对应的buffer可用
            co_await free_channel_.async_send(ErrorCode{}, index, asio::use_awaitable);
            co_await asio::async_write(socket_, asio::buffer(response), asio::use_awaitable);
            response.clear(); // 写完了就清空，准备下一批命令的结果了
        }
    }
    catch (const boost::system::system_error& e) {
        if (e.code() != asio::error::operation_aborted && e.code() != asio::error::eof &&
            e.code() != asio::error::connection_reset && e.code() != asio::error::broken_pipe &&
            e.code() != asio::experimental::error::channel_cancelled)
            std::println("Writer error: {}", e.what());
    }
    catch (std::exception& e) {
        std::println("Writer error: {}", e.what());
    }
}

auto Session::watchdog() -> asio::awaitable<void>
{
    asio::steady_timer timer{ socket_.get_executor() };
    while (true) {
        timer.expires_at(deadline_);
        co_await timer.async_wait(asio::use_awaitable);

        if (deadline_ <= Clock::now()) {
            socket_.shutdown(asio::ip::tcp::socket::shutdown_both);
            socket_.close();
            co_return;
        }
    }
}

} // namespace spg::core
