#include "spg_redis_commands_string.h"

#include <spg_base_utility.h>
#include <spg_redis_alias.h>

using namespace spg::base;

namespace spg::redis {

auto set_with_expiry(Database& db, Commands args) -> Reply
{
    auto expiry = numeric_cast<std::uint64_t>(args[4]);
    if (!expiry)
        return invalid_integral_error;

    db.set(args[1], args[2], *expiry);
    return ok;
}

auto set_persist(Database& db, Commands args) -> Reply
{
    db.set(args[1], args[2]);
    return ok;
}

auto string_commands::set(Database& db, Commands commands) -> Reply
{
    if (commands.size() == 3)
        return set_persist(db, commands);

    if (commands.size() == 5 && strings::to_lowercase(commands[3]) == "ex")
        return set_with_expiry(db, commands);

    return wrong_number_of_arguments_error("set");
}

auto string_commands::get(Database& db, Commands commands) -> Reply
{
    if (commands.size() != 2)
        return wrong_number_of_arguments_error("get");

    auto res = db.get(commands[1]);
    if (!res)
        return nullptr;

    if (auto* value = std::get_if<StringPointer>(&*res))
        return *value;

    return wrong_type_error;
}

} // namespace spg::redis