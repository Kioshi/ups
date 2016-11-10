#pragma once
#include <assert.h>
#include <functional>
#include "utils.hpp"
#include "packet.hpp"
#include <vector>

#ifdef WIN32
#include <stdio.h>
#include <winsock.h>
#include <stdlib.h>
#include <winioctl.h>
#define ioctl ioctlsocket
#pragma comment(lib, "Ws2_32.lib")
typedef int socklen_t;
#else
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif


class TCP
{
public:
    TCP()
    {
#ifdef WIN32
        if (!_connections++)
        {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
                DieWithError("WSAStartup() failed");
        }
#endif
    }

    TCP(int socket) 
        : _socket(socket)
    {
        TCP();
    }

    int socket()
    {
        _socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (_socket < 0)
            DieWithError("socket() failed");
        return _socket;
    }
    /*
    int connect(unsigned short port, char *address)
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(address);
        addr.sin_port = htons(port);
        int ret =::connect(_socket, (struct sockaddr *) &addr, sizeof(addr));
        if (ret < 0)
            DieWithError("connect() failed");
        return ret;
    }*/
    int bind(unsigned short port)
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        int ret = ::bind(_socket, (struct sockaddr *) &addr, sizeof(addr));
        if (ret < 0)
            DieWithError("bind() failed");
        return ret;
    }

    int listen()
    {
        int ret = ::listen(_socket, 5);
        if (ret < 0)
            DieWithError("listen() failed");
        return ret;
    }

    void accept(std::function<void(int)> lambda )
    {
        struct sockaddr_in addr;
        socklen_t addrLen = sizeof(sockaddr_in);
        int clientSocket = ::accept(_socket, (struct sockaddr *)&addr, &addrLen);
        if (clientSocket > 0)
            lambda(clientSocket);
        else
            DieWithError("Accept error");
    }

    int recv(char * buffer, int lenght)
    {
        return ::recv(_socket, buffer, lenght, 0);
    }

    void send(const char * buffer, int lenght)
    {
        ::send(_socket, buffer, lenght, 0);
    }

    void send(Packet* packet)
    {
        char * buffer = new char[packet->getSize()];
        send(buffer, packet->getSize());
        delete buffer;
    }

    Packet* recieve()
    {
        char o[1];
        char s[1];
        char data[257];
        Opcodes opcode;
        uint8 size;
        memset(data, 0, sizeof(data));

        if (recv(o, sizeof(o)) > 0)
            opcode = (Opcodes)o[0];
        else
            return nullptr;

        //Packets without data
        switch (opcode)
        {
            case UNUSED:
                return new Packet(opcode);
        }

        if (recv(s, sizeof(s)) > 0)
            size = s[0];
        else
            return nullptr;

        if (recv(data, size + 1) > 0)
            return new Packet(opcode, size, data);
        else
            return nullptr;
    }

    bool winSelect(std::vector<Packet*>& packets)
    {
        fd_set readfds;
        //clear the socket fd set
        FD_ZERO(&readfds);

        FD_SET(_socket, &readfds);

        TIMEVAL tv = { 0 };
        tv.tv_usec = 10*IN_MICROSECONDS;
        int ret = ::select(0, &readfds, NULL, NULL, &tv);
        if (ret == -1)
            return false;
        else if (ret == 0)
            return true; // timeout
                   
        if (FD_ISSET(_socket, &readfds))
        {
            Packet* packet = recieve();
            if (packet)
                packets.push_back(packet);
            else
                return false;
        }

        return true;
    }

    ~TCP()
    {

#ifdef WIN32
        closesocket(_socket);
        if (--_connections == 0)
            WSACleanup();
#else
        close(_socket);
#endif
    }

private:
#ifdef WIN32
    void DieWithError(char *errorMessage)
    {
        fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
        assert(false);
    }
#else
    void DieWithError(char *errorMessage)
    {
        perror(errorMessage);
        assert(false);
}
#endif
    int _socket;
    static uint32 _connections;
};