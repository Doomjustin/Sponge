#ifndef SPONGE_REDIS_DATABASE_H
#define SPONGE_REDIS_DATABASE_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
namespace sponge::redis {

class Database {
public:
    using Key = std::string;
    using String = std::string;
    using StringPointer = std::shared_ptr<String>;
    using Hash = std::unordered_map<Key, StringPointer>;
    using List = std::list<StringPointer>;

    using Value = std::variant<StringPointer, Hash, List>;
    using Size = std::size_t;
    using Seconds = std::uint64_t;

    template<typename T>
    static constexpr auto is_valid_value =
        std::is_same_v<T, StringPointer> || std::is_same_v<T, Hash> || std::is_same_v<T, List>;
    template<typename Value>
        requires is_valid_value<Value>
    void set(const Key& key, Value value)
    {
        erase_expire(key);

        data_.insert_or_assign(key, std::move(value));
    }

    template<typename Value>
        requires is_valid_value<Value>
    void set(const Key& key, Value value, Seconds expire_seconds)
    {
        erase_expire(key);
        auto [it, _] = data_.insert_or_assign(key, std::move(value));

        expires_.insert(std::make_pair(now() + expire_seconds * 1000, std::string_view{ it->first }));
    }

    void set(const Key& key, std::string_view message, Seconds expire_seconds);

    void set(const Key& key, std::string_view message);

    template<typename Value>
        requires is_valid_value<Value>
    auto get_if(const Key& key) -> std::optional<Value>
    {
        if (erase_if_expired(key))
            return {};

        auto it = data_.find(key);
        if (it != data_.end() && std::holds_alternative<Value>(it->second))
            return std::get<Value>(it->second);

        return {};
    }

    auto get(const Key& key) -> std::optional<Value>;

    template<typename Range>
        requires std::ranges::input_range<Range> && std::same_as<std::ranges::range_value_t<Range>, Key>
    auto keys(Range&& keys) -> std::vector<Key>
    {
        std::vector<Key> result{};
        result.reserve(keys.size());
        for (const auto& key : keys) {
            if (contains(key))
                result.push_back(key);
        }

        return result;
    }

    template<typename Range>
        requires std::ranges::input_range<Range> && std::same_as<std::ranges::range_value_t<Range>, Key>
    auto mget(Range&& keys) -> std::vector<StringPointer>
    {
        std::vector<StringPointer> result{};
        result.reserve(keys.size());
        for (const auto& key : keys) {
            if (auto res = get_if<StringPointer>(key))
                result.push_back(*res);
            else
                result.emplace_back(nullptr);
        }

        return result;
    }

    template<typename Range>
        requires std::ranges::input_range<Range> && std::same_as<std::ranges::range_value_t<Range>, Key>
    auto erase(Range&& keys) -> int
    {
        int count = 0;
        for (const auto& key : keys) {
            // 如果本来就过期了，那么就不算删除成功
            if (erase_if_expired(key))
                continue;

            auto it = data_.find(key);
            if (it != data_.end()) {
                erase_expire(key);
                data_.erase(it);
                ++count;
            }
        }
        return count;
    }

    auto expire_at(const Key& key, Seconds seconds) -> bool;
    // time to live
    auto ttl(const Key& key) -> std::optional<Seconds>;

    auto contains(const Key& key) -> bool;

    void flush();

    void flush_async();

    auto persist(const Key& key) -> bool;

    auto expired_keys() -> std::vector<std::string>;

    [[nodiscard]]
    constexpr auto size() const noexcept -> Size
    {
        return data_.size();
    }

private:
    using Clock = std::chrono::steady_clock;
    using TimeUnit = std::chrono::milliseconds;
    using Datas = std::unordered_map<Key, Value>;
    using Expires = std::set<std::pair<Seconds, std::string_view>>;

    Datas data_;
    Expires expires_;

    static auto now() -> Seconds;

    auto erase_if_expired(const Key& key) -> bool;

    auto find_expire_time(std::string_view key) -> Expires::iterator;

    void erase_expire(std::string_view key);
};

} // namespace sponge::redis

#endif // SPONGE_REDIS_DATABASE_H
