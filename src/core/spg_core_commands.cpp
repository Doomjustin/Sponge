#include "spg_core_commands.h"

#include <spg_core_database.h>

namespace {

using Command = spg::core::RedisController::Command;
using spg::core::Database;

void get(Database& db, const Command& command, std::string& response)
{
    if (command.size() != 2) {
        response.append("-ERR wrong number of arguments for 'GET' command\r\n");
        return;
    }

    auto key = command[1];
    auto value_opt = db.get(key);
    if (value_opt)
        response.append("$").append(std::to_string(value_opt->size())).append("\r\n").append(*value_opt).append("\r\n");
    else
        response.append("$-1\r\n");
}

void set(Database& db, const Command& command, std::string& response)
{
    if (command.size() != 3) {
        response.append("-ERR wrong number of arguments for 'SET' command\r\n");
        return;
    }

    auto key = command[1];
    auto value = command[2];
    db.set(key, value);
    response.append("+OK\r\n");
}

void dbsize(Database& db, const Command& command, std::string& response)
{
    if (command.size() != 1) {
        response.append("-ERR wrong number of arguments for 'DBSIZE' command\r\n");
        return;
    }

    auto size = db.size();
    response.append(":").append(std::to_string(size)).append("\r\n");
}

void config(Database& db, const Command& command, std::string& response) { response.append("+OK\r\n"); }

} // namespace

namespace spg::core {

RedisController::RedisController()
{
    handlers_["GET"] = { .type = Type::Read, .handler = get };
    handlers_["SET"] = { .type = Type::Write, .handler = set };
    handlers_["DBSIZE"] = { .type = Type::Read, .handler = dbsize };
    handlers_["CONFIG"] = { .type = Type::Read, .handler = config };
}

void RedisController::execute(Database& db, const Command& command, std::string& response)
{
    if (command.empty()) {
        response.append("-ERR empty command\r\n");
        return;
    }

    std::array<char, 16> upper_cmd{};
    std::string_view cmd{ command[0] };
    if (cmd.size() >= sizeof(upper_cmd)) {
        response.append("-ERR command too long\r\n");
        return;
    }

    for (std::size_t i = 0; i < cmd.size(); ++i)
        upper_cmd[i] = cmd[i] & ~0x20; // 转大写

    std::string_view search_key{ upper_cmd.data(), cmd.size() };

    auto it = handlers_.find(search_key);
    if (it != handlers_.end())
        it->second.handler(db, command, response);
    else
        response.append("-ERR unknown command\r\n");
}

auto RedisController::instance() -> RedisController&
{
    static RedisController instance;
    return instance;
}

} // namespace spg::core
