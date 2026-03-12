#ifndef SPONGE_REDIS_COMMANDS_H
#define SPONGE_REDIS_COMMANDS_H

#include <spg_base_io_context_pool.h>
#include <spg_redis_database.h>

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>

#include <string>
#include <vector>

namespace sponge::redis {

struct commands {
    commands() = delete;

    using Arguments = std::vector<std::string>;
    using Response = std::string;

    static auto dispatch(Database& db, const Arguments& arguments) -> Response;
};

} // namespace sponge::redis

#endif // SPONGE_REDIS_COMMANDS_H
