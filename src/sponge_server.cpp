#include <spg_core_server.h>

#include <cstdlib>
#include <thread>

using namespace spg::core;

int main(int argc, char* argv[])
{
    auto size = std::thread::hardware_concurrency();

    Server server{ "0.0.0.0", 26379, size };
    server.start();

    return EXIT_SUCCESS;
}