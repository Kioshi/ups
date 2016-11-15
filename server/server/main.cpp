#include <iostream>
#include "tcp.hpp"
#include <thread>
#include "utils.hpp"
#include "server.hpp"

int main(int argc, char** argv)
{
    uint16 port;
#ifndef DEBUGING
    std::cout << "Zadejte port: ";
    std::cin >> port;
#else
    port = 10001;
#endif // !DEBUGING

    Server server(port);
    server.run();

    return 0;
}