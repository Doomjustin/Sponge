#include "spg_core_session.h"

#include <spg_base_io_context_pool.h>
#include <spg_core_application_context.h>
#include <spg_core_commands.h>
#include <spg_core_resp_parser.h>

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/beast.hpp>

#include <print>

using namespace boost;

namespace spg::core {

Session::Session(Socket socket, ApplicationContext& context, Size id)
    : socket_{ std::move(socket) }
    , context_{ context }
    , id_{ id }
{
}

auto Session::work() -> asio::awaitable<void>
{
    using namespace asio::experimental::awaitable_operators;

    deadline_ = Clock::now() + TIMEOUT;
    co_await (do_work() || watchdog());
}

auto Session::do_work() -> asio::awaitable<void>
{
    // 确保协程内生命周期。这样执行命令的过程中，command可以使用string_view来传递参数，而不用担心内存被释放了
    beast::flat_buffer query_buffer{ 8 * 1024 * 1024 }; // 8MB，足够应付绝大多数命令了
    std::string response_buffer;

    try {
        while (true) {
            using namespace std::literals;
            deadline_ = Clock::now() + TIMEOUT;

            // 贪婪读取数据，尽可能多地将数据读入buffer中，以减少系统调用次数和上下文切换次数，提高性能
            int read_budget = 4;
            while (read_budget-- > 0) {
                if (query_buffer.size() > 0 && socket_.available() == 0)
                    break;

                auto n = co_await socket_.async_read_some(query_buffer.prepare(1024 * 1024), asio::use_awaitable);
                query_buffer.commit(n);
            }

            auto buffer_seq = query_buffer.data();
            std::string_view query{ static_cast<const char*>(buffer_seq.data()), buffer_seq.size() };
            auto [commands, consumed_bytes] = parse_resp_batch(query);

            // 半包
            if (commands.empty())
                continue;

            // 处理命令
            co_await execute_pipeline(commands, response_buffer);
            // 消耗掉已经解析完的数据。此时里面应该还剩下半包数据，等下一轮数据到来时继续解析
            query_buffer.consume(consumed_bytes);

            co_await asio::async_write(socket_, asio::buffer(response_buffer), asio::use_awaitable);
            response_buffer.clear();
        }
    }
    catch (const system::system_error& e) {
        if (e.code() != asio::error::operation_aborted && e.code() != asio::error::eof &&
            e.code() != asio::error::connection_reset)
            std::println("Session error: {}", e.what());
    }
    catch (const std::exception& e) {
        std::println("Session error: {}", e.what());
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

auto Session::execute_pipeline(std::vector<std::vector<std::string_view>>& commands, std::string& response)
    -> boost::asio::awaitable<void>
{
    if (commands.empty())
        co_return;

    std::vector<std::string> results(commands.size());

    auto coro_num = context_.io_context_pool.size();
    std::vector<std::vector<std::size_t>> tasks(coro_num); // 每个协程负责执行哪些命令

    for (Size i = 0; i < commands.size(); ++i) {
        const auto& cmd = commands[i];

        if (cmd.size() < 2) {
            tasks[id_].push_back(i);
        }
        else {
            auto target_coro_id = std::hash<std::string_view>{}(cmd[1]) % coro_num;
            tasks[target_coro_id].push_back(i);
        }
    }

    Size active_coro_num = 0;
    for (const auto& task : tasks)
        if (!task.empty())
            ++active_coro_num;

    asio::experimental::channel<void(boost::system::error_code)> barrier{ io_context(), active_coro_num };

    for (Size i = 0; i < coro_num; ++i) {
        if (tasks[i].empty())
            continue;

        auto execute_commands = [&commands, &results, &barrier, this, target = i,
                                 indices = std::move(tasks[i])] -> boost::asio::awaitable<void> {
            if (target != id_)
                co_await boost::asio::post(io_context(target), asio::use_awaitable);

            for (size_t idx : indices)
                RedisController::instance().execute(shard(target), commands[idx], results[idx]);

            if (target != id_)
                co_await boost::asio::post(io_context(id_), asio::use_awaitable);

            barrier.try_send(boost::system::error_code{});
        };

        boost::asio::co_spawn(io_context(), execute_commands, asio::detached);
    }

    for (size_t i = 0; i < active_coro_num; ++i)
        co_await barrier.async_receive(asio::use_awaitable);

    for (const auto& res : results)
        response.append(res);
}

} // namespace spg::core
