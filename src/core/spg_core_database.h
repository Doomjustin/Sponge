#ifndef SPONGE_CORE_DATABASE_SHARD_H
#define SPONGE_CORE_DATABASE_SHARD_H

#include <spg_core_string_view_hash.h>

#include <boost/unordered/unordered_flat_map.hpp>
#include <gsl/gsl>

#include <chrono>
#include <memory_resource>
#include <optional>

namespace spg::core {

struct persist_t {};

static constexpr auto persist = persist_t{};

constexpr auto is_persist(std::chrono::steady_clock::time_point tp) noexcept -> bool
{
    return tp == std::chrono::steady_clock::time_point::max();
}

class Database {
public:
    using Size = std::size_t;
    using String = std::pmr::string;
    using MemoryResource = std::pmr::memory_resource;
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using MilliSeconds = std::chrono::milliseconds;
    static constexpr auto TICK_INTERVAL = MilliSeconds{ 100 };

    explicit Database(gsl::not_null<MemoryResource*> resource);

    void set(std::string_view key, std::string_view value, MilliSeconds ttl) noexcept;

    void set(std::string_view key, std::string_view value, [[maybe_unused]] persist_t persist) noexcept
    {
        set(key, value, MilliSeconds::zero());
    }

    void ttl(std::string_view key, MilliSeconds ttl) noexcept;

    void set(std::string_view key, [[maybe_unused]] persist_t persist) noexcept { ttl(key, MilliSeconds::zero()); }

    [[nodiscard]]
    auto get(std::string_view key) noexcept -> std::optional<std::string_view>;

    auto expire_at(std::string_view key) noexcept -> std::optional<TimePoint>;

    auto erase(std::string_view key) noexcept -> bool
    {
        auto& segment = segments_[segment_index(key)];
        return segment.erase(key) > 0;
    }

    void tick() noexcept;

    void clear() noexcept;

    [[nodiscard]] constexpr auto size() const noexcept -> Size
    {
        Size total_size = 0;
        for (const auto& segment : segments_)
            total_size += segment.size();
        return total_size;
    }

    [[nodiscard]]
    constexpr auto empty() const noexcept -> bool
    {
        return size() == 0;
    }

private:
    struct Timer {
        String key;
        int remaining_laps;

        Timer(std::string_view k, int laps, MemoryResource* resource)
            : key{ k, resource }
            , remaining_laps{ laps }
        {
        }
    };

    struct Value {
        String value;
        TimePoint expire_at;

        Value(std::string_view val, TimePoint expire, MemoryResource* resource)
            : value{ val, resource }
            , expire_at{ expire }
        {
        }
    };
    using Pair = std::pair<const String, Value>;
    using Allocator = std::pmr::polymorphic_allocator<Pair>;
    using Container = boost::unordered_flat_map<String, Value, StringViewHash, std::equal_to<>, Allocator>;
    static constexpr auto WHEEL_SLOTS = 600; // 100ms * 600 = 60s
    static constexpr Size SEGMENTS = 1024;
    static constexpr Size SEGMENT_MASK = SEGMENTS - 1;

    MemoryResource* resource_;
    // DashTable<Value> repository_{ resource_ };
    Allocator allocator_;
    std::array<Container, SEGMENTS> segments_;
    std::pmr::vector<std::pmr::vector<Timer>> timer_wheel_{ resource_ };
    Size current_slot_ = 0;

    [[nodiscard]]
    static constexpr auto segment_index(std::string_view value) noexcept -> Size
    {
        return hash(value) & SEGMENT_MASK;
    }
};

} // namespace spg::core

#endif // SPONGE_CORE_DATABASE_SHARD_H
