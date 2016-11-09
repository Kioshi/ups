#include <iostream>
#include "tcp.hpp"
#include <thread>
#include "utils.hpp"
#include "server.hpp"

int main(int argc, char** argv)
{
    uint16 port;
    std::cout << "Zadejte port: ";
    std::cin >> port;
    Server server(port);
    server.run();

    return 0;
}