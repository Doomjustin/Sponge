#ifndef SPONGE_CORE_RESP_PARSER_H
#define SPONGE_CORE_RESP_PARSER_H

#include <string_view>
#include <vector>

namespace spg::core {

using Command = std::vector<std::string_view>;
using Commands = std::vector<Command>;

struct ParseResult {
    Commands commands;
    std::size_t consumed_bytes = 0;
};

auto parse_resp_batch(std::string_view buffer) -> ParseResult;

} // namespace spg::core

#endif // SPONGE_CORE_RESP_PARSER_H
