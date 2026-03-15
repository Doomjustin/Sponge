#ifndef SPONGE_CORE_APPLICATION_CONTEXT_H
#define SPONGE_CORE_APPLICATION_CONTEXT_H

#include <spg_base_io_context_pool.h>
#include <spg_core_database.h>
#include <spg_core_tracking_resource.h>

#include <cstddef>
#include <memory_resource>
#include <vector>

namespace spg::core {

struct ByHashT {};

static constexpr ByHashT by_hash{};

class ApplicationContext {
public:
    using Size = std::size_t;

    explicit ApplicationContext(Size count);

    ApplicationContext(const ApplicationContext&) = delete;
    auto operator=(const ApplicationContext&) -> ApplicationContext& = delete;

    ApplicationContext(ApplicationContext&&) = delete;
    auto operator=(ApplicationContext&&) -> ApplicationContext& = delete;

    ~ApplicationContext() = default;

    [[nodiscard]]
    constexpr auto data_size() const noexcept -> Size
    {
        auto total_size = Size{ 0 };
        for (const auto& shard : shards)
            total_size += shard.size();
        return total_size;
    }

    auto db(Size key_hash, ByHashT by_hash) noexcept -> Database& { return shards[key_hash % shards.size()]; }

    auto db(Size index) noexcept -> Database& { return shards[index]; }

    auto io_context(Size key_hash, ByHashT by_hash) noexcept -> boost::asio::io_context&
    {
        return io_context_pool[key_hash % io_context_pool.size()];
    }

    auto io_context(std::size_t id) noexcept -> boost::asio::io_context& { return io_context_pool[id]; }

    [[nodiscard]]
    constexpr auto size() const noexcept -> Size
    {
        return shards.size();
    }

    [[nodiscard]]
    constexpr auto used_memory(Size index) const noexcept -> std::size_t
    {
        return resources[index]->used_memory();
    }

    [[nodiscard]]
    constexpr auto total_used_memory() const noexcept -> std::size_t
    {
        Size total = 0;
        for (Size i = 0; i < size(); ++i)
            total += used_memory(i);

        return total;
    }

private:
    using Pool = std::pmr::unsynchronized_pool_resource;

    base::IOContextPool io_context_pool;
    std::vector<std::unique_ptr<Pool>> pools;
    std::vector<std::unique_ptr<TrackingMemoryResource>> resources;
    std::pmr::vector<Database> shards;
};

} // namespace spg::core

#endif // SPONGE_CORE_APPLICATION_CONTEXT_H
