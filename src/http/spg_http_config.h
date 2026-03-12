#ifndef SPONGE_HTTP_CONFIG_H
#define SPONGE_HTTP_CONFIG_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

namespace sponge::http {

struct Config {
    using Port = std::uint16_t;
    using Seconds = std::chrono::seconds;
    using Path = std::filesystem::path;
    using Size = std::size_t;

    struct Common {
        Size worker_threads;
        Size io_threads;
    };

    struct Network {
        Port port;
        std::string address;
        Seconds timeout;
    };

    struct SSL {
        Path cert;
        Path key;
        bool enable_tls_v13;
    };

    Common common;
    Network network;
    SSL ssl;
};

auto parse_from_file(std::string_view path) -> Config;

} // namespace sponge::http

#endif // SPONGE_HTTP_CONFIG_H
