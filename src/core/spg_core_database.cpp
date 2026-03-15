#include "spg_core_database.h"

namespace spg::core {

Database::Database(gsl::not_null<MemoryResource*> resource)
    : resource_{ resource }
    , allocator_{ resource_ }
{
    for (auto& segment : segments_) {
        segment = Container{ allocator_ };
        segment.max_load_factor(0.7F);
        segment.reserve(1024);
    }

    timer_wheel_.reserve(WHEEL_SLOTS);
    for (auto i = 0; i < WHEEL_SLOTS; ++i)
        timer_wheel_.push_back(std::pmr::vector<Timer>{ resource_ });
}

void Database::set(std::string_view key, std::string_view value, MilliSeconds ttl) noexcept
{
    auto expire_at = TimePoint::max();

    if (ttl.count() > 0) {
        expire_at = Clock::now() + ttl;
        auto ticks = ttl / TICK_INTERVAL;
        auto target_slot = (current_slot_ + ticks) % WHEEL_SLOTS;
        auto laps = static_cast<int>(ticks / WHEEL_SLOTS);
        timer_wheel_[target_slot].emplace_back(key, laps, resource_);
    }

    Value val{ value, expire_at, resource_ };
    auto& segment = segments_[segment_index(key)];
    String insert_key{ key, resource_ };
    segment.insert_or_assign(std::move(insert_key), std::move(val));
}

void Database::ttl(std::string_view key, MilliSeconds ttl) noexcept
{
    auto& segment = segments_[segment_index(key)];
    auto it = segment.find(key);
    if (it == segment.end())
        return; // key不存在，无法设置TTL

    set(key, it->second.value, ttl);
}

auto Database::get(std::string_view key) noexcept -> std::optional<std::string_view>
{
    auto& segment = segments_[segment_index(key)];
    auto it = segment.find(key);
    if (it != segment.end()) {
        // 如果过期了，删除并返回空
        if (it->second.expire_at <= Clock::now()) {
            segment.erase(key);
            return {};
        }

        return it->second.value;
    }

    return {};
}

auto Database::expire_at(std::string_view key) noexcept -> std::optional<TimePoint>
{
    auto& segment = segments_[segment_index(key)];
    auto it = segment.find(key);
    if (it != segment.end()) {
        if (it->second.expire_at <= Clock::now()) {
            segment.erase(key);
            return {};
        }

        return it->second.expire_at;
    }

    return {};
}

void Database::clear() noexcept
{
    for (auto& segment : segments_)
        segment.clear();

    for (auto& slot : timer_wheel_)
        slot.clear();

    current_slot_ = 0;
}

void Database::tick() noexcept
{
    auto now = Clock::now();
    auto& timers = timer_wheel_[current_slot_];
    int max_laps = 100;

    for (auto it = timers.begin(); it != timers.end();) {
        if (it->remaining_laps > 0) {
            it->remaining_laps--;
            ++it;
        }
        else {
            // 定时器到期，删除对应的键值对
            auto& segment = segments_[segment_index(it->key)];
            segment.erase(it->key);
            it = timers.erase(it);
            if (--max_laps <= 0)
                return;
        }
    }

    // 只有当前槽的定时器处理完了，才移动到下一个槽
    current_slot_ = (current_slot_ + 1) % WHEEL_SLOTS;
}

} // namespace spg::core
