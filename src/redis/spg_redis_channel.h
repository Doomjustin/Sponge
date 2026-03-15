#ifndef SPONGE_REDIS_CHANNEL_H
#define SPONGE_REDIS_CHANNEL_H

#include "spg_redis_reply.h"
#include <spg_redis_alias.h>
#include <spg_redis_commands.h>

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>

#include <cstddef>
#include <memory>

namespace spg::redis {

template<typename Executor, typename Func>
using ConcurrentChannel = boost::asio::experimental::concurrent_channel<Executor, Func>;

using AnyExecutor = boost::asio::any_io_executor;

using ErrorCode = boost::system::error_code;

using ResponseChannel = ConcurrentChannel<AnyExecutor, void(ErrorCode, BatchReply)>;

struct Request {
    using Index = std::size_t;

    Index index;
    std::vector<Arguments> batch_commands;
    std::weak_ptr<ResponseChannel> response_channel;
};

using RequestChannel = ConcurrentChannel<AnyExecutor, void(ErrorCode, Request)>;

} // namespace spg::redis

#endif // SPONGE_REDIS_CHANNEL_H
