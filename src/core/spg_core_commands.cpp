#include "spg_core_commands.h"

#include <spg_base_utility.h>
#include <spg_core_application_context.h>
#include <spg_core_database.h>
#include <spg_core_string_view_hash.h>

#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <format>

namespace {

using namespace spg;
using namespace spg::core;
using Command = RedisController::Command;

struct base_commands {
    base_commands() = delete;

    static auto get(ApplicationContext& context, const Command& command, std::string& response)
        -> boost::asio::awaitable<void>
    {
        if (command.size() != 2) {
            response.append("-ERR wrong number of arguments for 'GET' command\r\n");
            co_return;
        }

        auto key = command[1];
        auto key_hash = hash(key);
        auto& db = context.db(key_hash, by_hash);

        // 切换到对应的io_context所在的线程，确保后续对数据库的访问是线程安全的
        co_await boost::asio::dispatch(context.io_context(key_hash, by_hash), boost::asio::use_awaitable);

        auto value_opt = db.get(key);

        if (value_opt)
            response.append(std::format("${}\r\n{}\r\n", value_opt->size(), *value_opt));
        else
            response.append("$-1\r\n");
    }

    static auto set_ex(ApplicationContext& context, const Command& command, std::string& response)
        -> boost::asio::awaitable<void>
    {
        auto key = command[1];
        auto value = command[2];
        auto ttl_str = command[4];
        auto ttl_opt = base::numeric_cast<std::chrono::seconds::rep>(ttl_str);

        if (!ttl_opt) {
            response.append("-ERR invalid TTL value\r\n");
            co_return;
        }

        auto ttl = std::chrono::seconds{ *ttl_opt };

        auto key_hash = hash(key);
        auto& db = context.db(key_hash, by_hash);

        // 切换到对应的io_context所在的线程，确保后续对数据库的访问是线程安全的
        co_await boost::asio::dispatch(context.io_context(key_hash, by_hash), boost::asio::use_awaitable);

        db.set(key, value, ttl);
        response.append("+OK\r\n");
    }

    static auto set_base(ApplicationContext& context, const Command& command, std::string& response)
        -> boost::asio::awaitable<void>
    {
        auto key = command[1];
        auto value = command[2];
        auto key_hash = hash(key);
        auto& db = context.db(key_hash, by_hash);

        // 切换到对应的io_context所在的线程，确保后续对数据库的访问是线程安全的
        co_await boost::asio::dispatch(context.io_context(key_hash, by_hash), boost::asio::use_awaitable);

        db.set(key, value, spg::core::persist);
        response.append("+OK\r\n");
    }

    static auto set(ApplicationContext& context, const Command& command, std::string& response)
        -> boost::asio::awaitable<void>
    {
        if (command.size() == 3)
            co_return co_await set_base(context, command, response);

        if (command.size() == 5) {
            auto option = command[3];
            if (option == "EX")
                co_return co_await set_ex(context, command, response);
            else
                response.append("-ERR syntax error\r\n");
        }

        response.append("-ERR wrong number of arguments for 'SET' command\r\n");
    }

    static auto dbsize(ApplicationContext& context, const Command& command, std::string& response)
        -> boost::asio::awaitable<void>
    {
        if (command.size() != 1) {
            response.append("-ERR wrong number of arguments for 'DBSIZE' command\r\n");
            co_return;
        }

        // 计算所有分片中键值对的总数
        std::size_t total_size = 0;
        for (std::size_t i = 0; i < context.size(); ++i) {
            auto& db = context.db(i);

            co_await boost::asio::dispatch(context.io_context(i), boost::asio::use_awaitable);
            total_size += db.size();
        }

        response.append(":").append(std::to_string(total_size)).append("\r\n");
    }

    static auto flushdb(ApplicationContext& context, const Command& command, std::string& response)
        -> boost::asio::awaitable<void>
    {
        if (command.size() != 1) {
            response.append("-ERR wrong number of arguments for 'FLUSHDB' command\r\n");
            co_return;
        }

        for (std::size_t i = 0; i < context.size(); ++i) {
            co_await boost::asio::dispatch(context.io_context(i), boost::asio::use_awaitable);
            auto& db = context.db(i);
            db.clear();
        }

        response.append("+OK\r\n");
    }

    static auto del(ApplicationContext& context, const Command& command, std::string& response)
        -> boost::asio::awaitable<void>
    {
        if (command.size() < 2) {
            response.append("-ERR wrong number of arguments for 'DEL' command\r\n");
            co_return;
        }

        for (std::size_t i = 1; i < command.size(); ++i) {
            auto key = command[i];
            auto key_hash = hash(key);
            auto& db = context.db(key_hash, by_hash);

            co_await boost::asio::dispatch(context.io_context(key_hash, by_hash), boost::asio::use_awaitable);
            auto deleted_count = db.erase(key);
            response.append(":").append(std::to_string(deleted_count)).append("\r\n");
        }
    }

    static auto ttl(ApplicationContext& context, const Command& command, std::string& response)
        -> boost::asio::awaitable<void>
    {
        if (command.size() != 2) {
            response.append("-ERR wrong number of arguments for 'TTL' command\r\n");
            co_return;
        }

        auto key = command[1];
        auto key_hash = hash(key);
        auto& db = context.db(key_hash, by_hash);

        co_await boost::asio::dispatch(context.io_context(key_hash, by_hash), boost::asio::use_awaitable);

        auto expire_at_opt = db.expire_at(key);
        if (!expire_at_opt) {
            response.append(":-2\r\n"); // key不存在
            co_return;
        }

        if (core::is_persist(*expire_at_opt)) {
            response.append(":-1\r\n"); // key存在但没有过期时间
            co_return;
        }

        auto now = Database::Clock::now();
        auto ttl = std::chrono::duration_cast<std::chrono::seconds>(*expire_at_opt - now).count();
        response.append(":").append(std::to_string(ttl)).append("\r\n");
    }

    static auto expire(ApplicationContext& context, const Command& command, std::string& response)
        -> boost::asio::awaitable<void>
    {
        if (command.size() != 3) {
            response.append("-ERR wrong number of arguments for 'EXPIRE' command\r\n");
            co_return;
        }

        auto key = command[1];
        auto ttl_str = command[2];

        auto ttl_opt = base::numeric_cast<std::chrono::seconds::rep>(ttl_str);
        if (!ttl_opt) {
            response.append("-ERR invalid TTL value\r\n");
            co_return;
        }

        auto key_hash = hash(key);
        auto& db = context.db(key_hash, by_hash);

        co_await boost::asio::dispatch(context.io_context(key_hash, by_hash), boost::asio::use_awaitable);

        auto ttl = std::chrono::seconds{ *ttl_opt };
        auto value_opt = db.get(key);
        if (!value_opt) {
            response.append(":0\r\n"); // key不存在
            co_return;
        }

        db.ttl(key, ttl);
        response.append(":1\r\n"); // 成功设置了过期时间
    }

    static auto persist(ApplicationContext& context, const Command& command, std::string& response)
        -> boost::asio::awaitable<void>
    {
        if (command.size() != 2) {
            response.append("-ERR wrong number of arguments for 'PERSIST' command\r\n");
            co_return;
        }

        auto key = command[1];
        auto key_hash = hash(key);
        auto& db = context.db(key_hash, by_hash);

        co_await boost::asio::dispatch(context.io_context(key_hash, by_hash), boost::asio::use_awaitable);

        auto value_opt = db.get(key);
        if (!value_opt) {
            response.append(":0\r\n"); // key不存在
            co_return;
        }

        db.set(key, core::persist);
        response.append(":1\r\n"); // 成功移除了过期时间
    }
};

} // namespace

namespace spg::core {

RedisController::Table RedisController::handlers_{
    { "GET", base_commands::get },         { "SET", base_commands::set },         { "DBSIZE", base_commands::dbsize },
    { "FLUSHDB", base_commands::flushdb }, { "DEL", base_commands::del },         { "TTL", base_commands::ttl },
    { "EXPIRE", base_commands::expire },   { "PERSIST", base_commands::persist },
};

RedisController::RedisController(ApplicationContext& context, std::size_t id)
    : id_{ id }
    , context_{ context }
{
}

auto RedisController::execute(const std::vector<Command>& batch_commands, std::string& response)
    -> boost::asio::awaitable<void>
{
    auto this_context = co_await boost::asio::this_coro::executor;
    for (auto command : batch_commands) {
        if (command.empty()) {
            response.append("-ERR empty command\r\n");
            continue;
        }

        std::array<char, 16> upper_cmd{};
        std::string_view cmd{ command[0] };
        if (cmd.size() >= sizeof(upper_cmd)) {
            response.append("-ERR command too long\r\n");
            continue;
        }

        for (std::size_t i = 0; i < cmd.size(); ++i)
            upper_cmd[i] = cmd[i] & ~0x20; // 转大写

        std::string_view search_key{ upper_cmd.data(), cmd.size() };
        auto it = handlers_.find(search_key);
        if (it != handlers_.end()) {
            co_await it->second(context_, command, response);
        }
        else {
            response.append("-ERR unknown command\r\n");
        }
    }

    co_await boost::asio::dispatch(this_context, boost::asio::use_awaitable);
}

} // namespace spg::core
