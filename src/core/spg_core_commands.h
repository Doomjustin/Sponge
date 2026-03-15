#ifndef SPONGE_CORE_COMMANDS_H
#define SPONGE_CORE_COMMANDS_H

#include <spg_core_database.h>
#include <spg_core_string_view_hash.h>

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

namespace spg::core {

class RedisController {
public:
    using Command = std::vector<std::string_view>;
    using Handler = void (*)(Database& db, const Command& args, std::string& response);

    RedisController();

    void execute(Database& db, const Command& command, std::string& response);

    static auto instance() -> RedisController&;

private:
    enum class Type : std::uint8_t { Read, Write };

    struct Entry {
        Type type;
        Handler handler;
    };

    using Table = std::unordered_map<std::string, Entry, StringViewHash, std::equal_to<>>;

    Table handlers_;
};

} // namespace spg::core

#endif // SPONGE_CORE_COMMANDS_H
