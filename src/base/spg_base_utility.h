#ifndef SPONGE_BASE_UTILITY_H
#define SPONGE_BASE_UTILITY_H

#include <charconv>
#include <optional>
#include <string_view>
#include <vector>

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

struct strings {
public:
    strings() = delete;

    static auto to_lowercase(std::string_view str) -> std::string;

    static auto to_uppercase(std::string_view str) -> std::string;

    static auto trim(std::string_view str) -> std::string;

    static auto trim_left(std::string_view str) -> std::string;

    static auto trim_right(std::string_view str) -> std::string;

    static auto split(std::string_view str, std::string_view delimeter) -> std::vector<std::string>;
};

} // namespace sponge::base

#endif // SPONGE_BASE_UTILITY_H
