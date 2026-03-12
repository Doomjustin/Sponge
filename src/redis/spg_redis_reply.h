#ifndef SPONGE_REDIS_REPLY_H
#define SPONGE_REDIS_REPLY_H

#include "spg_redis_alias.h"

#include <boost/asio/buffer.hpp>

#include <deque>
#include <format>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace sponge::redis {

struct Reply;

using ReplyData = std::variant<std::string_view,             // static string
                               std::shared_ptr<std::string>, // DB data
                               std::string,                  // dynamic string
                               std::int64_t,                 // number
                               std::vector<Reply>>;          // array

using BatchReply = std::vector<Reply>;

struct Reply {
    ReplyData data;

    template<typename T>
    Reply(T&& t)
        : data{ std::forward<T>(t) }
    {
    }
};

class WriteContext {
public:
    WriteContext()
    {
        // 终生只分配一次，64KB 足够应付最大的 Pipeline 批次
        // 只要不超出这个大小，底层的 data() 指针就绝对不会因为扩容而失效！
        out_buf_.reserve(64 * 1024);
    }

    void reset()
    {
        out_buf_.clear();
        iovecs_.clear();
        keep_alive_.clear();
        last_iovec_offset_ = 0;
    }

    void append_static(std::string_view sv) { out_buf_.append(sv); }

    void append_dynamic(std::string_view sv) { out_buf_.append(sv); }

    void append_db_value(const std::shared_ptr<std::string>& ptr)
    {
        if (!ptr) {
            out_buf_.append("$-1\r\n");
            return;
        }

        // 1. 把长度头追加到连续内存中 (0 malloc!)
        std::format_to(std::back_inserter(out_buf_), "${}\r\n", ptr->size());

        // 2. 将在这之前积累的所有字符串，折叠成 1 个 buffer 发送
        push_accumulated();

        // 3. 把 DB 的零拷贝内存放进发送数组
        iovecs_.push_back(boost::asio::buffer(*ptr));
        keep_alive_.push_back(ptr); // 保命

        // 4. 尾部的 CRLF 继续记入连续内存
        out_buf_.append("\r\n");
    }

    void append_reply(const Reply& reply)
    {
        std::visit(
            [this](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, std::string_view>)
                    append_static(arg);
                else if constexpr (std::is_same_v<T, StringPointer>)
                    append_db_value(arg);
                else if constexpr (std::is_same_v<T, std::string>)
                    append_dynamic(arg);
                else if constexpr (std::is_same_v<T, std::int64_t>)
                    std::format_to(std::back_inserter(out_buf_), ":{}\r\n", arg);
                else if constexpr (std::is_same_v<T, std::vector<Reply>>) {
                    std::format_to(std::back_inserter(out_buf_), "*{}\r\n", arg.size());
                    for (const auto& item : arg)
                        append_reply(item);
                }
            },
            reply.data);
    }

    // 处理完毕，索要最终的 buffers
    auto buffers() -> const std::vector<boost::asio::const_buffer>&
    {
        push_accumulated(); // 把循环结束时最后剩下的小字符串刷进去
        return iovecs_;
    }

private:
    std::string out_buf_;
    std::vector<boost::asio::const_buffer> iovecs_;
    std::vector<std::shared_ptr<std::string>> keep_alive_;
    std::size_t last_iovec_offset_ = 0;

    // 核心黑魔法：把连续字符串切片变成 buffer
    void push_accumulated()
    {
        if (out_buf_.size() > last_iovec_offset_) {
            iovecs_.push_back(
                boost::asio::buffer(out_buf_.data() + last_iovec_offset_, out_buf_.size() - last_iovec_offset_));
            last_iovec_offset_ = out_buf_.size();
        }
    }
};
} // namespace sponge::redis

#endif // SPONGE_REDIS_REPLY_H
