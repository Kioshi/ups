#include <iostream>
#include "Tcp.h"
#include <thread>
#include "utils.h"
#include "Server.h"
#include <ctime>

//#define DEBUGING
int main(int argc, char** argv)
{
    uint16 port;
#ifndef DEBUGING
    std::cout << "Zadejte port: ";
    std::cin >> port;
    std::srand(std::time(0));
#else
    port = 10001;
#endif // !DEBUGING

    Server server(port);
    server.run();

    return 0;
}