#include "spg_base_io_context_pool.h"

#include <boost/asio/executor_work_guard.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>

using namespace boost::asio;

namespace sponge::base {

IOContextPool::IOContextPool(Size size)
{
    if (size == 0)
        throw std::out_of_range("IOContextPool size is 0");

    for (Size i = 0; i < size; ++i) {
        auto context = std::make_shared<io_context>();
        io_contexts_.push_back(context);
        works_.push_back(boost::asio::make_work_guard(*context));
        threads_.emplace_back([context]() { context->run(); });
    }
}

IOContextPool::~IOContextPool()
{
    stop();
    std::ranges::for_each(threads_, [](auto& thread) { thread.join(); });
}

void IOContextPool::stop()
{
    std::ranges::for_each(io_contexts_, [](auto& ctx) { ctx->stop(); });
}

auto IOContextPool::get_io_context() -> Context&
{
    auto& context = *io_contexts_[next_io_context_];
    next_io_context_ = (next_io_context_ + 1) % io_contexts_.size();

    return context;
}

} // namespace sponge::base
