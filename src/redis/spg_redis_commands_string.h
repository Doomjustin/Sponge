#ifndef SPONGE_REDIS_COMMAND_STRING_H
#define SPONGE_REDIS_COMMAND_STRING_H

#include <spg_redis_alias.h>
#include <spg_redis_database.h>
#include <spg_redis_reply.h>

namespace sponge::redis {

struct string_commands {
    string_commands() = delete;

    // set key value [EX seconds]
    // If the command is called with 3 arguments, it sets the key to the specified value without an
    // expiration time. If the command is called with 5 arguments and the fourth argument is "EX",
    // it sets the key to the specified value and sets the expiration time to the number of seconds
    // specified in the fifth argument.
    static auto set(Database& db, Commands commands) -> Reply;

    // get key
    // If the key exists and holds a string value, it returns the value of the key.
    // If the key does not exist, it returns a Null Bulk String.
    // If the key exists but does not hold a string value, it returns a WRONGTYPE error.
    static auto get(Database& db, Commands commands) -> Reply;
};

} // namespace sponge::redis

#endif // SPONGE_REDIS_COMMAND_STRING_H