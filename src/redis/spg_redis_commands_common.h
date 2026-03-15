#ifndef SPONGE_REDIS_COMMAND_COMMON_H
#define SPONGE_REDIS_COMMAND_COMMON_H

#include <spg_redis_alias.h>
#include <spg_redis_database.h>
#include <spg_redis_reply.h>

namespace spg::redis {

struct common_commands {
    common_commands() = delete;

    // ping [message]
    // If no argument is provided, it returns a simple PONG response.
    // If an argument is provided, it returns the same message as a response.
    static auto ping(Database& db, Commands commands) -> Reply;

    // keys pattern
    // return all keys matching pattern
    static auto keys(Database& db, Commands commands) -> Reply;

    // mget key [key ...]
    // return the values of all specified keys.
    // For every key that does not hold a string value or does not exist, the special value nil is
    // returned. Because of this, the operation never fails
    static auto mget(Database& db, Commands commands) -> Reply;

    // del key [key ...]
    // Removes the specified keys. A key is ignored if it does not exist.
    // The command returns the number of keys that were removed.
    static auto del(Database& db, Commands commands) -> Reply;

    // flushdb [ASYNC|SYNC]
    // If called without arguments, it synchronously deletes all keys in the current database and
    // returns OK.
    // If called with the ASYNC argument, it initiates an asynchronous flush operation that deletes
    // all keys in the current database in the background and returns OK immediately. The actual
    // deletion of keys is performed asynchronously, allowing the server to continue processing
    // other commands without blocking. Once the asynchronous flush operation is complete,
    static auto flushdb(Database& db, Commands commands) -> Reply;

    // dbsize
    // return the number of keys in the currently-selected database.
    static auto dbsize(Database& db, Commands commands) -> Reply;

    // expire key seconds
    // Set a timeout on key. After the timeout has expired, the key will automatically be deleted.
    // If the key does not exist, it returns 0. Otherwise, it returns 1.
    static auto expire(Database& db, Commands commands) -> Reply;

    // ttl key
    // Return the remaining time to live of a key that has an expiration time.
    // If the key does not exist, it returns -2. If the key exists but has no associated expiration
    // time, it returns -1.
    // Otherwise, it returns the remaining time to live of the key in seconds.
    static auto ttl(Database& db, Commands commands) -> Reply;

    // persist key
    // Remove the existing timeout on key.
    // If the key does not exist, it returns 0.
    // If the key exists and the timeout was removed, it returns 1.
    // If the key exists but does not have an associated timeout, it returns 0.
    static auto persist(Database& db, Commands commands) -> Reply;

    // exists key [key ...]
    // Returns the number of keys that exist among the ones specified as arguments.
    // For every key that does not exist, zero is contributed to the result.
    static auto exists(Database& db, Commands commands) -> Reply;
};

} // namespace spg::redis

#endif // XIN_REDIS_COMMAND_COMMON_H