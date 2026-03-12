#ifndef SPONGE_REDIS_ALIAS_H
#define SPONGE_REDIS_ALIAS_H

#include <boost/asio.hpp>

#include <cstdint>
#include <format>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace sponge::redis {

static constexpr auto use_nothrow_awaitable = boost::asio::as_tuple(boost::asio::use_awaitable);

using Response = std::string;

using Commands = std::span<std::string>;

using Arguments = std::vector<std::string>;

using String = std::string;

using StringPointer = std::shared_ptr<String>;

using Hash = std::unordered_map<std::string, StringPointer>;

using List = std::list<StringPointer>;

constexpr auto bulk_string(std::string_view message) -> std::string
{
    return std::format("${}\r\n{}\r\n", message.size(), message);
}

constexpr auto simple_string(std::string_view message) -> std::string { return std::format("+{}\r\n", message); }

constexpr auto wrong_string(std::string_view message) -> std::string { return std::format("-{}\r\n", message); }

constexpr auto integral_string(std::int64_t number) -> std::string { return std::format(":{}\r\n", number); }

static constexpr std::string_view invalid_integral_error = "-ERR value is not an integer or out of range\r\n";

static constexpr std::string_view wrong_type_error =
    "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";

static constexpr std::string_view ok = "+OK\r\n";

static constexpr std::string_view null_bulk_string = "$-1\r\n";

static constexpr std::string_view out_of_range = "-ERR value is not an integer or out of range\r\n";

static constexpr std::string_view key_not_exist = ":-2\r\n";

static constexpr std::string_view key_no_expiry = ":-1\r\n";

static constexpr std::string_view empty_commands = "-ERR empty command\r\n";

constexpr auto wrong_number_of_arguments_error(std::string_view name) -> std::string
{
    return std::format("-ERR wrong number of arguments for '{}' command\r\n", name);
}

constexpr auto unknown_command(std::string_view name) -> std::string
{
    return std::format("-ERR unknown command '{}'\r\n", name);
}

class ArrayBuilder {
public:
    void add_simple_string(std::string_view message)
    {
        content_ += simple_string(message);
        ++count_;
    }

    void add_bulk_string(std::string_view message)
    {
        content_ += bulk_string(message);
        ++count_;
    }

    void add_wrong_message(std::string_view message)
    {
        content_ += wrong_string(message);
        ++count_;
    }

    void add_formatted_string(std::string_view message)
    {
        content_ += message;
        ++count_;
    }

    auto build() -> std::string { return std::format("*{}\r\n{}", count_, content_); }

private:
    int count_ = 0;
    std::string content_;
};

} // namespace sponge::redis

#endif // SPONGE_REDIS_ALIAS_H
