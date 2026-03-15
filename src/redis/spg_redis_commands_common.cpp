#include "spg_redis_commands_common.h"

#include <spg_base_utility.h>
#include <spg_redis_alias.h>

namespace spg::redis {

auto common_commands::ping(Database& db, Commands commands) -> Reply
{
    if (commands.size() == 1)
        return simple_string("PONG");

    if (commands.size() == 2)
        return simple_string(commands[1]);

    return wrong_number_of_arguments_error("ping");
}

auto common_commands::keys(Database& db, Commands commands) -> Reply
{
    if (commands.size() < 2)
        return wrong_number_of_arguments_error("keys");

    auto pattern = std::span{ commands.begin() + 1, commands.end() };
    auto res = db.keys(pattern);

    std::vector<Reply> result;
    result.reserve(res.size());
    for (auto& ptr : res)
        result.emplace_back(std::move(ptr));

    return result;
}

auto common_commands::mget(Database& db, Commands commands) -> Reply
{
    if (commands.size() < 2)
        return wrong_number_of_arguments_error("mget");

    auto keys = std::span{ commands.begin() + 1, commands.end() };
    auto res = db.mget(keys);

    std::vector<Reply> result;
    result.reserve(res.size());

    for (auto& ptr : res)
        result.emplace_back(std::move(ptr));

    return result;
}

auto common_commands::del(Database& db, Commands commands) -> Reply
{
    if (commands.size() < 2)
        return wrong_number_of_arguments_error("del");

    auto keys = std::span{ commands.begin() + 1, commands.end() };
    auto erased_count = db.erase(keys);

    return integral_string(erased_count);
}

auto flushdb_async(Database& db, Commands commands) -> Reply
{
    db.flush_async();
    return ok;
}

auto flushdb_sync(Database& db, Commands commands) -> Reply
{
    db.flush();
    return ok;
}

auto common_commands::flushdb(Database& db, Commands commands) -> Reply
{
    if (commands.size() == 1)
        return flushdb_sync(db, commands);

    if (commands.size() == 2 && base::strings::to_lowercase(commands[1]) == "async")
        return flushdb_async(db, commands);

    if (commands.size() == 2 && base::strings::to_lowercase(commands[1]) == "sync")
        return flushdb_sync(db, commands);

    return wrong_number_of_arguments_error("flushdb");
}

auto common_commands::dbsize(Database& db, Commands commands) -> Reply
{
    if (commands.size() != 1)
        return wrong_number_of_arguments_error("dbsize");

    return static_cast<std::int64_t>(db.size());
}

auto common_commands::expire(Database& db, Commands commands) -> Reply
{
    if (commands.size() != 3)
        return wrong_number_of_arguments_error("expire");

    auto seconds = base::numeric_cast<std::uint64_t>(commands[2]);
    if (!seconds)
        return out_of_range;

    const auto& key = commands[1];
    return db.expire_at(key, *seconds) ? 1 : 0;
}

auto common_commands::ttl(Database& db, Commands commands) -> Reply
{
    if (commands.size() != 2)
        return wrong_number_of_arguments_error("ttl");

    const auto& key = commands[1];
    auto res = db.ttl(key);
    if (!res) {
        if (db.contains(key))
            return -1;

        return -2;
    }

    return static_cast<std::int64_t>(*res);
}

auto common_commands::persist(Database& db, Commands commands) -> Reply
{
    if (commands.size() != 2)
        return wrong_number_of_arguments_error("persist");

    const auto& key = commands[1];

    if (db.persist(key))
        return 1;

    return 0;
}

auto common_commands::exists(Database& db, Commands commands) -> Reply
{
    if (commands.size() < 2)
        return wrong_number_of_arguments_error("exists");

    std::int64_t count = 0;
    for (std::size_t i = 1; i < commands.size(); ++i) {
        if (db.contains(commands[i]))
            ++count;
    }

    return count;
}

} // namespace spg::redis