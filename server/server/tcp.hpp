#pragma once
#include <assert.h>
#include <functional>

#ifdef WIN32
#include <stdio.h>
#include <winsock.h>
#include <stdlib.h>
#pragma comment(lib, "Ws2_32.lib")
typedef int socklen_t;
#else
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif


class TCP
{
public:
    TCP()
    {
#ifdef WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            DieWithError("WSAStartup() failed");
        _cleanup = true;
#endif
    }

    TCP(int socket) 
        : _socket(socket)
        , _cleanup(false)
    {

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
        while (true)
        {
            struct sockaddr_in addr;
            socklen_t addrLen = sizeof(sockaddr_in);
            int clientSocket = ::accept(_socket, (struct sockaddr *)&addr, &addrLen);
            if (clientSocket > 0)
                lambda(clientSocket);
            else
                DieWithError("Accept error");
        }
    }

    int recv(char * buffer, int lenght)
    {
        return ::recv(_socket, buffer, lenght, 0);
    }

    void send(const char * buffer, int lenght)
    {
        ::send(_socket, buffer, lenght, 0);
    }

    ~TCP()
    {

#ifdef WIN32
        closesocket(_socket);
        if (_cleanup)
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
    bool _cleanup;
};