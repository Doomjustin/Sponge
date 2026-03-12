#ifndef SPONGE_REDIS_ALIAS_H
#define SPONGE_REDIS_ALIAS_H

#include <boost/asio.hpp>

namespace sponge::redis {

static constexpr auto use_nothrow_awaitable = boost::asio::as_tuple(boost::asio::use_awaitable);

} // namespace sponge::redis

#endif // SPONGE_REDIS_ALIAS_H
