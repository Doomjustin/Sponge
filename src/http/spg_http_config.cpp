#include "spg_http_config.h"

#include <toml++/toml.h>

namespace spg::http {

auto parse_from_file(std::string_view path) -> Config
{
    Config config{};

    auto tml = toml::parse_file(path);

    using namespace std::literals;

    if (auto* common = tml["common"].as_table()) {
        config.common.worker_threads = (*common)["worker_threads"].value_or(1);
        config.common.worker_threads = (*common)["io_threads"].value_or(1);
    }

    if (auto* net = tml["network"].as_table()) {
        config.network.address = (*net)["address"].value_or("127.0.0.1"sv);
        config.network.port = (*net)["port"].value_or(12345);

        auto timeout = (*net)["timeout"].value_or(10);
        config.network.timeout = Config::Seconds{ timeout };
    }

    if (auto* ssl = tml["ssl"].as_table()) {
        config.ssl.cert = (*ssl)["cert_path"].value_or("server.crt");
        config.ssl.key = (*ssl)["key_path"].value_or("server.key");
        config.ssl.enable_tls_v13 = (*ssl)["key_path"].value_or(false);
    }

    return config;
}

} // namespace spg::http
