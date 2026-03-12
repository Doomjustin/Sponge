#ifndef SPONGE_REDIS_CHANNEL_H
#define SPONGE_REDIS_CHANNEL_H

#include <spg_redis_alias.h>
#include <spg_redis_commands.h>

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>

#include <cstddef>
#include <memory>

namespace sponge::redis {

template<typename Executor, typename Func>
using ConcurrentChannel = boost::asio::experimental::concurrent_channel<Executor, Func>;

using AnyExecutor = boost::asio::any_io_executor;

using ErrorCode = boost::system::error_code;

using ResponseChannel = ConcurrentChannel<AnyExecutor, void(ErrorCode, commands::Response)>;

struct Request {
    using Index = std::size_t;
    using Arguments = commands::Arguments;

    std::weak_ptr<ResponseChannel> response_channel;
    Index index;
    Arguments arguments;
};

using RequestChannel = ConcurrentChannel<AnyExecutor, void(ErrorCode, Request)>;

} // namespace sponge::redis

#endif // SPONGE_REDIS_CHANNEL_H
