#ifndef SPONGE_CORE_STRING_VIEW_HASH_H
#define SPONGE_CORE_STRING_VIEW_HASH_H

#include <functional>
#include <string>
#include <string_view>

namespace spg::core {

using Hasher = std::hash<std::string_view>;

[[nodiscard]]
constexpr auto hash(std::string_view key) -> std::size_t
{
    return Hasher{}(key);
}

struct StringViewHash {
    using is_transparent = void;

    auto operator()(std::string_view sv) const noexcept -> std::size_t { return hash(sv); }

    auto operator()(const std::string& s) const noexcept -> std::size_t { return hash(s); }
};

struct PmrStringViewHash {
    using is_transparent = void;

    auto operator()(std::string_view sv) const -> std::size_t { return hash(sv); }

    auto operator()(const std::pmr::string& s) const -> std::size_t
    {
        return hash(std::string_view(s.data(), s.size()));
    }
};

} // namespace spg::core

#endif // SPONGE_CORE_STRING_VIEW_HASH_H
