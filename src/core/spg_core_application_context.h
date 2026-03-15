#ifndef SPONGE_CORE_APPLICATION_CONTEXT_H
#define SPONGE_CORE_APPLICATION_CONTEXT_H

#include <spg_base_io_context_pool.h>
#include <spg_core_database.h>

#include <cstddef>
#include <vector>

namespace spg::core {

struct ApplicationContext {
    using Size = std::size_t;

    std::vector<Database> shards;
    base::IOContextPool io_context_pool;

    explicit ApplicationContext(Size count)
        : shards(count)
        , io_context_pool(count)
    {
    }
};

} // namespace spg::core

#endif // SPONGE_CORE_APPLICATION_CONTEXT_H
