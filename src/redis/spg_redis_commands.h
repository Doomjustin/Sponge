#ifndef SPONGE_REDIS_COMMANDS_H
#define SPONGE_REDIS_COMMANDS_H

#include <spg_redis_alias.h>
#include <spg_redis_database.h>
#include <spg_redis_reply.h>

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>

namespace sponge::redis {

struct commands {
    commands() = delete;

    static auto dispatch(Database& db, Commands command) -> Reply;
};

} // namespace sponge::redis

#endif // SPONGE_REDIS_COMMANDS_H
