#ifndef SPONGE_CORE_SESSION_H
#define SPONGE_CORE_SESSION_H

#include <spg_base_io_context_pool.h>
#include <spg_core_application_context.h>
#include <spg_core_commands.h>
#include <spg_core_database.h>

#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/beast.hpp>

#include <chrono>

namespace spg::core {

class Session {
public:
    using Socket = boost::asio::ip::tcp::socket;
    using Size = std::size_t;

    Session(Socket socket, ApplicationContext& context, Size id);

    auto work() -> boost::asio::awaitable<void>;

private:
    using TimePoint = std::chrono::steady_clock::time_point;
    using Clock = std::chrono::steady_clock;
    using Seconds = std::chrono::seconds;
    using ErrorCode = boost::system::error_code;
    using BatchCommand = std::vector<std::vector<std::string_view>>;
    using ReadyChannel = boost::asio::experimental::channel<void(ErrorCode, BatchCommand, std::size_t)>;
    using FreeChannel = boost::asio::experimental::channel<void(ErrorCode, std::size_t)>;

    static constexpr Seconds TIMEOUT{ 10 };
    static constexpr Size BUFFER_SIZE = 1024 * 1024 * 1024; // 1GB，足够应付绝大多数命令了
    // 一次性读取的最大字节数
    static constexpr Size ONCE_READ_SIZE = 128 * 1024;
    // 512MB，超过这个大小就先处理命令，避免一次性读取过多数据导致内存占用过高
    static constexpr Size READ_ONCE_MAX_SIZE = 512 * 1024 * 1024;
    // 512个buffer，减少协程之间争抢buffer的情况，同时也能应付pipeline中命令较多的情况
    static constexpr Size BUFFER_COUNT = 1024;

    Socket socket_;
    ApplicationContext& context_;
    Size id_;
    TimePoint deadline_;
    std::array<boost::beast::flat_buffer, BUFFER_COUNT> buffers_;
    // 此时id_已经赋值成功，所以我们可以直接用id_来索引
    // 用于通知执行命令的协程有新命令了
    ReadyChannel ready_channel_{ io_context(), BUFFER_COUNT };
    // 用于通知执行命令的协程有命令执行完了，可以继续执行下一批命令了
    FreeChannel free_channel_{ io_context(), BUFFER_COUNT };
    // 处理Redis命令的控制器，负责解析命令并调用对应的处理函数
    RedisController redis_controller_{ context_, id_ };

    // auto do_work() -> boost::asio::awaitable<void>;
    auto reader() -> boost::asio::awaitable<void>;

    auto writer() -> boost::asio::awaitable<void>;

    auto watchdog() -> boost::asio::awaitable<void>;

    auto io_context() -> base::IOContextPool::Context& { return context_.io_context(id_); }
};

} // namespace spg::core

#endif // SPONGE_CORE_SESSION_H