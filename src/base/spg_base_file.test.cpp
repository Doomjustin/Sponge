#include "spg_base_file.h"

#include <doctest/doctest.h>

#include <filesystem>

using sponge::base::full_path;

TEST_SUITE("full_path")
{
    TEST_CASE("full_path joins cwd for relative path")
    {
        const auto rel = std::filesystem::path{ "base/spg_base_file.h" };
        const auto expected = std::filesystem::current_path() / rel;

        CHECK(full_path(rel.string()) == expected);
    }

    TEST_CASE("full_path keeps absolute path unchanged")
    {
        const auto abs = (std::filesystem::current_path() / "src/base/spg_base_file.h").lexically_normal();

        CHECK(full_path(abs.string()) == abs);
    }
}
