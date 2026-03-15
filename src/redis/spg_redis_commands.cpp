#include "spg_redis_commands.h"

#include "spg_redis_alias.h"
#include "spg_redis_reply.h"
#include <spg_base_utility.h>
#include <spg_redis_commands_common.h>
#include <spg_redis_commands_string.h>

namespace {

using namespace spg::redis;

enum class Type : std::uint8_t { Read, Write };

struct Command {
    using Handler = std::function<Reply(Database& db, Commands command)>;

    Type type;
    Handler handler;
};

using CommandRegistry = std::unordered_map<std::string, Command>;

CommandRegistry commands_registry = {
    { "ping", { .type = Type::Read, .handler = common_commands::ping } },
    { "keys", { .type = Type::Read, .handler = common_commands::keys } },
    { "mget", { .type = Type::Read, .handler = common_commands::mget } },
    { "del", { .type = Type::Write, .handler = common_commands::del } },
    { "flushdb", { .type = Type::Write, .handler = common_commands::flushdb } },
    { "dbsize", { .type = Type::Read, .handler = common_commands::dbsize } },
    { "expire", { .type = Type::Write, .handler = common_commands::expire } },
    { "ttl", { .type = Type::Read, .handler = common_commands::ttl } },
    { "persist", { .type = Type::Write, .handler = common_commands::persist } },
    { "exists", { .type = Type::Read, .handler = common_commands::exists } },
    { "set", { .type = Type::Write, .handler = string_commands::set } },
    { "get", { .type = Type::Read, .handler = string_commands::get } },
    // { "hget", { .type = Type::Read, .handler = hash_table_commands::get } },
    // { "hgetall", { .type = Type::Read, .handler = hash_table_commands::get_all } },
    // { "hset", { .type = Type::Write, .handler = hash_table_commands::set } },
    // { "lpush", { .type = Type::Write, .handler = list_commands::push } },
    // { "lpop", { .type = Type::Write, .handler = list_commands::pop } },
    // { "lrange", { .type = Type::Read, .handler = list_commands::range } },
    // { "zadd", { .type = Type::Write, .handler = sorted_set_commands::add } },
    // { "zrange", { .type = Type::Read, .handler = sorted_set_commands::range } },
};

} // namespace

namespace spg::redis {

auto commands::dispatch(Database& db, Commands commands) -> Reply
{
    const auto& command = base::strings::to_lowercase(commands[0]);
    auto it = commands_registry.find(command);
    if (it == commands_registry.end())
        return unknown_command(commands[0]);

    // AOF日志
    // if (it->second.type == Type::Write && !application_context::replaying_aof) {
    //     if (last_db_index != index) {
    //         auto select_args = Arguments{ "SELECT", std::to_string(index) };
    //         application_context::aof_logger.append(resp::serialize(select_args));
    //         last_db_index = index;
    //     }

    //     application_context::aof_logger.append(resp::serialize(args));
    // }

    return it->second.handler(db, commands);
}

} // namespace spg::redis
