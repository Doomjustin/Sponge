#ifndef SPONGE_CORE_DATABASE_SHARD_H
#define SPONGE_CORE_DATABASE_SHARD_H

#include <spg_core_string_view_hash.h>

#include <boost/unordered/unordered_flat_map.hpp>

#include <memory_resource>
#include <optional>
#include <string>

namespace spg::core {

class Database {
public:
    using Size = std::size_t;
    using String = std::pmr::string;

    Database();

    void set(std::string_view key, std::string_view value);

    auto get(std::string_view key) const -> std::optional<std::string_view>;

    constexpr auto size() const noexcept -> Size { return repository_.size(); }

    constexpr auto empty() const noexcept -> bool { return repository_.empty(); }

private:
    struct PmrStringViewHash {
        using is_transparent = void;

        auto operator()(std::string_view sv) const -> std::size_t { return std::hash<std::string_view>{}(sv); }

        auto operator()(const String& s) const -> std::size_t
        {
            return std::hash<std::string_view>{}(std::string_view(s.data(), s.size()));
        }
    };

    std::pmr::unsynchronized_pool_resource pool_;
    std::pmr::polymorphic_allocator<char> allocator_{ &pool_ };

    using Allocator = std::pmr::polymorphic_allocator<std::pair<const std::string, std::string>>;
    using Container = boost::unordered_flat_map<std::string, std::string, StringViewHash, std::equal_to<>, Allocator>;
    // using Container = std::pmr::unordered_map<std::string, std::string, StringViewHash, std::equal_to<>>;
    Container repository_;
};

} // namespace spg::core

#endif // SPONGE_CORE_DATABASE_SHARD_H
