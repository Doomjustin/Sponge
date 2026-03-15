#include "spg_core_application_context.h"

namespace spg::core {

ApplicationContext::ApplicationContext(Size count)
    : io_context_pool(count)
{
    pools.reserve(count);
    resources.reserve(count);
    shards.reserve(count);

    for (Size i = 0; i < count; ++i) {
        pools.emplace_back(std::make_unique<Pool>());
        resources.emplace_back(std::make_unique<TrackingMemoryResource>(pools.back().get()));
        shards.emplace_back(resources.back().get());
    }
}

} // namespace spg::core
