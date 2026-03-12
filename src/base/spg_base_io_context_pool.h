#ifndef SPONGE_BASE_IO_CONTEXT_POOL_H
#define SPONGE_BASE_IO_CONTEXT_POOL_H

#include <boost/asio.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <cstddef>
#include <list>
#include <memory>
#include <thread>
#include <vector>

namespace sponge::base {

class IOContextPool {
public:
    using Context = boost::asio::io_context;
    using ContextPtr = std::shared_ptr<Context>;
    using WorkGuard = boost::asio::executor_work_guard<Context::executor_type>;
    using Size = std::size_t;

    explicit IOContextPool(Size size);

    IOContextPool(const IOContextPool&) = delete;
    auto operator=(const IOContextPool&) = delete;

    ~IOContextPool();

    void stop();

    auto get_io_context() -> Context&;

private:
    std::vector<ContextPtr> io_contexts_;
    std::vector<std::jthread> threads_;
    std::list<WorkGuard> works_;
    Size next_io_context_ = 0;
};

} // namespace sponge::base

#endif // SPONGE_BASE_IO_CONTEXT_POOL_H
