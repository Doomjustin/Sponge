#ifndef SPONGE_CORE_STRING_VIEW_HASH_H
#define SPONGE_CORE_STRING_VIEW_HASH_H

#include <functional>
#include <string>
#include <string_view>

namespace spg::core {

struct StringViewHash {
    using is_transparent = void;

    auto operator()(std::string_view sv) const noexcept -> std::size_t { return std::hash<std::string_view>{}(sv); }

    auto operator()(const std::string& s) const noexcept -> std::size_t { return std::hash<std::string_view>{}(s); }
};

} // namespace spg::core

#endif // SPONGE_CORE_STRING_VIEW_HASH_H
