#include "Tcp.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include "Message.h"

#ifdef WIN32
#include <Winsock2.h>
#include <winioctl.h>
#define ioctl ioctlsocket
#pragma comment(lib, "Ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#endif

#include <iostream>

uint32 TCP::_connections;

TCP::TCP(int sock)
{
    _socket = sock;
#ifdef WIN32
    if (!_connections++)
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            DieWithError("WSAStartup() failed");
    }
#endif
    if (!_socket)
        socket();
}

TCP::~TCP()
{

#ifdef WIN32
    closesocket(_socket);
    if (--_connections == 0)
        WSACleanup();
#else
    close(_socket);
#endif
}

int TCP::socket()
{
    _socket = (int)::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_socket < 0)
        DieWithError("socket() failed");
    return _socket;
}

int TCP::bind(unsigned short port)
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

int TCP::listen()
{
    int ret = ::listen(_socket, 5);
    if (ret < 0)
        DieWithError("listen() failed");
    return ret;
}

void TCP::accept(std::function<void(int)> lambda)
{
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(sockaddr_in);
    int clientSocket = (int)::accept(_socket, (struct sockaddr *)&addr, &addrLen);
    if (clientSocket > 0)
        lambda(clientSocket);
    /*else
    {
        DieWithError("Accept error");
    }*/
}

int TCP::recv(char * buffer, int lenght)
{
    return ::recv(_socket, buffer, lenght, 0);
}

void TCP::send(const char * buffer, int lenght)
{
    ::send(_socket, buffer, lenght, 0);
}

void TCP::send(Message* message)
{
    send(message->data.c_str(), (int)message->data.size());
}

void TCP::parseMessages(std::vector<Message*>& messages, std::vector<std::string>& msgs)
{
    for (auto& message : msgs)
    {
        std::vector<std::string> tokens;
        splitMessages(tokens, message, TOKEN);

        if (tokens.empty())
            continue;

#define ef else if

        Opcodes opcode;
        if(tokens[0] == "LOGIN") opcode = LOGIN;
        ef(tokens[0] == "LOBBY_LIST") opcode = LOBBY_LIST;
        ef(tokens[0] == "CREATE_LOBBY") opcode = CREATE_LOBBY;
        ef(tokens[0] == "JOIN_LOBBY") opcode = JOIN_LOBBY;
        ef(tokens[0] == "LEAVE_LOBBY") opcode = LEAVE_LOBBY;
        ef(tokens[0] == "START_GAME") opcode = START_GAME;
        ef(tokens[0] == "KICK_PLAYER") opcode = KICK_PLAYER;
        ef(tokens[0] == "SEND_MESSAGE") opcode = SEND_MESSAGE;
        ef(tokens[0] == "QUIT") opcode = QUIT;
        else
            continue;
        
        tokens.erase(tokens.begin());
        if (validateMessage(opcode, tokens))
        {
            std::stringstream data;
            messages.push_back(new Message(opcode, tokens));
        }
    }
}

bool TCP::validateMessage(enum Opcodes opcode, std::vector<std::string>& tokens)
{
    switch (opcode)
    {
        case LOGIN:
        case SESSION:
            return !tokens.empty();
            break;
        case LOBBY_LIST:
            tokens.clear();
            return true;
        case CREATE_LOBBY:
        case JOIN_LOBBY:
        case LEAVE_LOBBY:
        case START_GAME:
        case KICK_PLAYER:
        case SEND_MESSAGE:
        case QUIT:
        case UNUSED:
        default:
            return false;
    }
}

bool TCP::recieve(std::vector<Message*>& messages, std::string& incompleteMessage)
{
    std::string message = incompleteMessage;
    incompleteMessage.clear();
    char buf[4096];
    int32 bytes_read;
    memset(buf, 0, sizeof(buf));
    do 
    {
        bytes_read = recv(buf, sizeof(buf));
        if (bytes_read < 0)
            return false;
        message += buf;
        memset(buf, 0, sizeof(buf));
    } while (bytes_read > 0);

    std::vector<std::string> msgs;
    splitMessages(msgs, message, DELIMITER);
    incompleteMessage += message;
    parseMessages(messages, msgs);

    return true;
}

#ifdef WIN32
bool TCP::select(std::vector<Message*>& messages, std::string& incompleteMessage)
{
    fd_set readfds;
    //clear the socket fd set
    FD_ZERO(&readfds);

    FD_SET(_socket, &readfds);

    TIMEVAL tv = { 0 };
    tv.tv_usec = 10 * IN_MICROSECONDS;
    int ret = ::select(0, &readfds, NULL, NULL, &tv);
    if (ret == -1)
        return false;
    else if (ret == 0)
        return true; // timeout

    if (FD_ISSET(_socket, &readfds))
    {
        if (!recieve(messages, incompleteMessage))
            return false;
    }

    return true;
}
#else
bool TCP::select(std::vector<Message*>& messages, std::string& incompleteMessage)
{
    return true;
}
#endif

void TCP::DieWithError(char *errorMessage)
{
#ifdef WIN32
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
#else
    perror(errorMessage);
#endif
    assert(false);
}
