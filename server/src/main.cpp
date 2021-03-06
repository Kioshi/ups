#include <iostream>
#include "Tcp.h"
#include <thread>
#include "utils.h"
#include "Server.h"
#include <ctime>

//#define DEBUGING
int main(int argc, char** argv)
{
    uint32 port;
#ifndef DEBUGING
    do 
    {
        std::cout << "Zadejte port: ";
        std::cin >> port;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } while (port <= 0 || port >= 65536);
    std::srand((unsigned int)std::time(0));
#else
    port = 10001;
#endif // !DEBUGING

    Server server(port);
    server.run();

    return 0;
}