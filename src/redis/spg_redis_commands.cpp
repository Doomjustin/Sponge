#include "spg_redis_commands.h"

namespace sponge::redis {

auto commands::dispatch(Database& db, const Arguments& arguments) -> Response { return "$4\r\nPONG\r\n"; }

} // namespace sponge::redis
