// server.cpp : Defines the entry point for the console application.
//
#include "tcp.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <sstream>

int main(int argc, char** argv) 
{
    TCP server;
    server.socket();
    server.bind(10001);
    server.listen();
    server.accept([](int socket)
    {
        std::cout << "New client: " << socket << std::endl;
        std::thread thread([socket]
        {
            TCP client(socket);
            while (true)
            {
#define BUFFER_SIZE 500
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, sizeof(buffer));
                if (client.recv(buffer, sizeof(buffer)) > 0)
                {
                    int index = 0;
                    do 
                    {
                        std::string text(buffer + index);
                        index += text.size() + 1;
                        std::cout << "Recieved from: " << socket << " text: '" << text.c_str() << "'" << std::endl;
                        std::reverse(text.begin(), text.end());
                        std::cout << "Reversed: '" << text.c_str() << "'" << std::endl;
                        client.send(text.c_str(), text.length() + 1);

                    } while (buffer[index] != 0 && index < BUFFER_SIZE);
                }
                else
                    return;
            }
        });
        thread.detach();
    });

    return 0;
}