#include <iostream>
#include "tcp.h"
#include <thread>
#include "utils.h"
#include "server.h"

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