#ifndef SPONGE_CORE_TRACKING_RESOURCE_H
#define SPONGE_CORE_TRACKING_RESOURCE_H

#include <gsl/gsl>

#include <atomic>
#include <memory_resource>

namespace spg::core {

// 一个基于std::pmr::memory_resource的内存跟踪器，
// 可以用来替代原来的allocator API，提供更现代的接口和更准确的内存使用统计
class TrackingMemoryResource : public std::pmr::memory_resource {
public:
    explicit TrackingMemoryResource(gsl::not_null<std::pmr::memory_resource*> upstream);

    [[nodiscard]]
    constexpr auto used_memory() const noexcept -> std::size_t
    {
        return total_allocated_.load(std::memory_order_relaxed);
    }

private:
    std::pmr::memory_resource* upstream_;
    std::atomic<std::size_t> total_allocated_{ 0 };

    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* override;

    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override;

    [[nodiscard]]
    constexpr auto do_is_equal(const std::pmr::memory_resource& other) const noexcept -> bool override
    {
        return this == &other;
    }
};

} // namespace spg::core

#endif // SPONGE_CORE_TRACKING_RESOURCE_H
