#include "spg_redis_reply.h"

#include <doctest/doctest.h>

#include <boost/asio/buffer.hpp>

namespace spg::redis {

namespace {

auto flatten_buffers(const std::vector<boost::asio::const_buffer>& buffers) -> std::string
{
    std::string out;
    for (const auto& buf : buffers) {
        const auto* ptr = static_cast<const char*>(buf.data());
        out.append(ptr, boost::asio::buffer_size(buf));
    }
    return out;
}

} // namespace

TEST_CASE("WriteContext keeps dynamic reply buffers valid")
{
    WriteContext ctx;

    std::string expected;
    for (int i = 0; i < 200; ++i) {
        std::string reply = "+MSG" + std::to_string(i) + "\r\n";
        expected += reply;
        ctx.append_reply(Reply{ reply });
    }

    CHECK(flatten_buffers(ctx.buffers()) == expected);
}

TEST_CASE("WriteContext serializes db string pointers as bulk strings")
{
    WriteContext ctx;

    auto v1 = std::make_shared<std::string>("hello");
    auto v2 = std::make_shared<std::string>("world");

    ctx.append_reply(Reply{ v1 });
    ctx.append_reply(Reply{ v2 });

    CHECK(flatten_buffers(ctx.buffers()) == "$5\r\nhello\r\n$5\r\nworld\r\n");
}

} // namespace spg::redis
