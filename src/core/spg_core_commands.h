#ifndef SPONGE_CORE_COMMANDS_H
#define SPONGE_CORE_COMMANDS_H

#include "spg_core_application_context.h"
#include <spg_core_database.h>
#include <spg_core_string_view_hash.h>

#include <string>
#include <string_view>
#include <unordered_map>

namespace spg::core {

class RedisController {
public:
    using Command = std::vector<std::string_view>;
    using Handler = boost::asio::awaitable<void> (*)(ApplicationContext& context, const Command& command,
                                                     std::string& response);

    RedisController(ApplicationContext& context, std::size_t id);

    auto execute(const std::vector<Command>& batch_commands, std::string& response) -> boost::asio::awaitable<void>;

private:
    using Table = std::unordered_map<std::string, Handler, StringViewHash, std::equal_to<>>;
    static Table handlers_;

    std::size_t id_;
    ApplicationContext& context_;
};

} // namespace spg::core

#endif // SPONGE_CORE_COMMANDS_H
