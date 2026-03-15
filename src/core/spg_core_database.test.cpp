#include "spg_core_database.h"

#include <doctest/doctest.h>

#include <memory_resource>
#include <random>
#include <string>
#include <unordered_map>

using namespace spg::core;

namespace {

auto make_db(std::pmr::unsynchronized_pool_resource& pool) -> Database { return Database{ &pool }; }

} // namespace

TEST_CASE("database: get missing key returns nullopt")
{
    std::pmr::unsynchronized_pool_resource pool;
    auto db = make_db(pool);

    auto value = db.get("missing");
    CHECK(!value.has_value());
}

TEST_CASE("database: set and get basic value")
{
    std::pmr::unsynchronized_pool_resource pool;
    auto db = make_db(pool);

    db.set("k", "v", persist);
    auto value = db.get("k");

    REQUIRE(value.has_value());
    CHECK(*value == "v");
}

TEST_CASE("database: overwrite existing key")
{
    std::pmr::unsynchronized_pool_resource pool;
    auto db = make_db(pool);

    db.set("k", "v1", persist);
    db.set("k", "v2", persist);

    auto value = db.get("k");
    REQUIRE(value.has_value());
    CHECK(*value == "v2");
}

TEST_CASE("database: erase existing and missing key")
{
    std::pmr::unsynchronized_pool_resource pool;
    auto db = make_db(pool);

    db.set("k", "v", persist);

    CHECK(db.erase("k"));
    CHECK(!db.erase("k"));
    CHECK(!db.get("k").has_value());
}

TEST_CASE("database: clear removes all keys")
{
    std::pmr::unsynchronized_pool_resource pool;
    auto db = make_db(pool);

    db.set("k1", "v1", persist);
    db.set("k2", "v2", persist);
    REQUIRE(db.size() == 2);

    db.clear();

    CHECK(db.empty());
    CHECK(!db.get("k1").has_value());
    CHECK(!db.get("k2").has_value());
}

TEST_CASE("database: ttl key expires after expected ticks")
{
    std::pmr::unsynchronized_pool_resource pool;
    auto db = make_db(pool);

    db.set("k", "v", Database::MilliSeconds{ 100 });

    // ttl=100ms lands in the next slot, so it takes two ticks to evict:
    // tick #1 advances to target slot, tick #2 processes and erases.
    db.tick();
    CHECK(db.get("k").has_value());

    db.tick();
    CHECK(!db.get("k").has_value());
}

TEST_CASE("database: persist removes ttl from existing key")
{
    std::pmr::unsynchronized_pool_resource pool;
    auto db = make_db(pool);

    db.set("k", "v", Database::MilliSeconds{ 100 });
    db.set("k", persist);

    db.tick();
    db.tick();

    auto value = db.get("k");
    REQUIRE(value.has_value());
    CHECK(*value == "v");
}

TEST_CASE("database: long ttl with laps works")
{
    std::pmr::unsynchronized_pool_resource pool;
    auto db = make_db(pool);

    constexpr auto long_ttl = Database::MilliSeconds{ 60100 }; // 601 ticks
    db.set("k", "v", long_ttl);

    for (int i = 0; i < 602; ++i)
        db.tick();

    CHECK(db.get("k").has_value());

    for (int i = 0; i < 600; ++i)
        db.tick();

    CHECK(!db.get("k").has_value());
}

TEST_CASE("database: random key set/get remains consistent")
{
    std::pmr::unsynchronized_pool_resource pool;
    auto db = make_db(pool);

    std::mt19937 rng{ 20260316 };
    std::unordered_map<std::string, std::string> expected;

    constexpr int operations = 20000;
    for (int i = 0; i < operations; ++i) {
        auto key_id = static_cast<int>(rng() % 4000);
        auto val_id = static_cast<int>(rng() % 1000000);

        auto key = std::string{ "k" } + std::to_string(key_id);
        auto value = std::string{ "v" } + std::to_string(val_id);

        db.set(key, value, persist);
        expected[key] = value;

        if ((i % 257) == 0) {
            auto got = db.get(key);
            REQUIRE(got.has_value());
            CHECK(*got == expected[key]);
        }
    }

    for (const auto& [key, value] : expected) {
        auto got = db.get(key);
        REQUIRE(got.has_value());
        CHECK(*got == value);
    }
}

TEST_CASE("database: random key set and erase consistency")
{
    std::pmr::unsynchronized_pool_resource pool;
    auto db = make_db(pool);

    std::mt19937 rng{ 31415926 };
    std::unordered_map<std::string, std::string> expected;

    constexpr int operations = 15000;
    for (int i = 0; i < operations; ++i) {
        auto key_id = static_cast<int>(rng() % 3000);
        auto key = std::string{ "rk" } + std::to_string(key_id);

        if ((rng() % 5) == 0) {
            auto erased = db.erase(key);
            auto existed = expected.erase(key) > 0;
            CHECK(erased == existed);
        }
        else {
            auto value = std::string{ "rv" } + std::to_string(static_cast<int>(rng() % 1000000));
            db.set(key, value, persist);
            expected[key] = value;
        }
    }

    for (const auto& [key, value] : expected) {
        auto got = db.get(key);
        REQUIRE(got.has_value());
        CHECK(*got == value);
    }
}
