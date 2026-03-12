#ifndef SPONGE_BASE_UTILITY_H
#define SPONGE_BASE_UTILITY_H

#include <charconv>
#include <optional>
#include <string_view>

namespace sponge::base {

template<typename T>
auto numeric_cast(std::string_view str) -> std::optional<T>
{
    T seconds;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), seconds);
    if (ec != std::errc())
        return {};

    return seconds;
}

} // namespace sponge::base

#endif // SPONGE_BASE_UTILITY_H
