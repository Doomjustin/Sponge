#include "spg_core_database.h"

namespace spg::core {

Database::Database()
    : repository_{ allocator_ }
{
    // 10万key的空间大约需要 1.5MB，考虑到 Redis 官方默认 maxmemory 就是 512MB，这个预设值是完全合理的
    repository_.reserve(100000); // 预留10万个key的空间，避免过早扩容
}

void Database::set(std::string_view key, std::string_view value)
{
    repository_.insert_or_assign(std::string{ key }, value);
}

auto Database::get(std::string_view key) const -> std::optional<std::string_view>
{
    auto it = repository_.find(key);
    if (it != repository_.end())
        return it->second;

    return {};
}

} // namespace spg::core
