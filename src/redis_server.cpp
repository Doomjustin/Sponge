#include <spg_redis_server.h>

#include <cstdlib>

int main(int argc, char* argv[])
{
    spg::redis::Server server{ "0.0.0.0" };
    server.run();

    return EXIT_SUCCESS;
}