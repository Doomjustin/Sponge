#include "spg_core_resp_parser.h"

#include <doctest/doctest.h>

#include <string>

using namespace spg::core;

TEST_SUITE("parse_resp_batch")
{
    TEST_CASE("empty buffer")
    {
        auto r = parse_resp_batch("");
        CHECK(r.commands.empty());
        CHECK(r.consumed_bytes == 0);
    }

    TEST_CASE("single GET command")
    {
        // *2\r\n$3\r\nGET\r\n$5\r\nmykey\r\n
        std::string_view input = "*2\r\n$3\r\nGET\r\n$5\r\nmykey\r\n";
        auto r = parse_resp_batch(input);
        REQUIRE(r.commands.size() == 1);
        REQUIRE(r.commands[0].size() == 2);
        CHECK(r.commands[0][0] == "GET");
        CHECK(r.commands[0][1] == "mykey");
        CHECK(r.consumed_bytes == input.size());
    }

    TEST_CASE("single SET command")
    {
        std::string_view input = "*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n";
        auto r = parse_resp_batch(input);
        REQUIRE(r.commands.size() == 1);
        REQUIRE(r.commands[0].size() == 3);
        CHECK(r.commands[0][0] == "SET");
        CHECK(r.commands[0][1] == "mykey");
        CHECK(r.commands[0][2] == "myvalue");
        CHECK(r.consumed_bytes == input.size());
    }

    TEST_CASE("pipeline - two commands")
    {
        std::string_view input = "*2\r\n$3\r\nGET\r\n$3\r\nfoo\r\n"
                                 "*2\r\n$3\r\nGET\r\n$3\r\nbar\r\n";
        auto r = parse_resp_batch(input);
        REQUIRE(r.commands.size() == 2);
        CHECK(r.commands[0][0] == "GET");
        CHECK(r.commands[0][1] == "foo");
        CHECK(r.commands[1][0] == "GET");
        CHECK(r.commands[1][1] == "bar");
        CHECK(r.consumed_bytes == input.size());
    }

    TEST_CASE("half packet - value truncated")
    {
        // mykey is only partially received
        auto r = parse_resp_batch("*2\r\n$3\r\nGET\r\n$5\r\nmyk");
        CHECK(r.commands.empty());
        CHECK(r.consumed_bytes == 0);
    }

    TEST_CASE("half packet - header only")
    {
        auto r = parse_resp_batch("*2\r\n");
        CHECK(r.commands.empty());
        CHECK(r.consumed_bytes == 0);
    }

    TEST_CASE("pipeline partial - first complete second truncated")
    {
        std::string_view complete = "*2\r\n$3\r\nGET\r\n$3\r\nfoo\r\n";
        std::string partial = std::string(complete) + "*2\r\n$3\r\nGET\r\n$3\r\nba";
        auto r = parse_resp_batch(partial);
        REQUIRE(r.commands.size() == 1);
        CHECK(r.commands[0][1] == "foo");
        CHECK(r.consumed_bytes == complete.size());
    }

    TEST_CASE("string_views point into original buffer")
    {
        std::string input = "*2\r\n$3\r\nGET\r\n$5\r\nmykey\r\n";
        auto r = parse_resp_batch(input);
        REQUIRE(r.commands.size() == 1);
        // string_view should reference the original buffer, not a copy
        CHECK(r.commands[0][0].data() >= input.data());
        CHECK(r.commands[0][0].data() + r.commands[0][0].size() <= input.data() + input.size());
    }
}
