#include "Tcp.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string.h>

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
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#include <iostream>

std::atomic<uint32> TCP::_connections;
std::atomic<uint64> TCP::sendBytes;
std::atomic<uint64> TCP::recvBytes;
std::atomic<uint64> TCP::sendMessages;
std::atomic<uint64> TCP::recvMessages;
std::atomic<uint64> TCP::accepted;
std::atomic<uint64> TCP::closed;
std::atomic<uint64> TCP::interrupted;

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
    closed++;
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
    {
        accepted++;
        lambda(clientSocket);
    }
}

int TCP::recv(char * buffer, int lenght)
{
    int ret = ::recv(_socket, buffer, lenght, 0);
    if (ret <= -1)
        interrupted++;
    else
        recvBytes += ret;
    return ret;
}

void TCP::send(const char * buffer, int lenght)
{
    sendBytes += lenght;
    ::send(_socket, buffer, lenght, 0);
}

void TCP::send(Message* message)
{
    sendMessages++;
    std::string msg = message->toString();
    send(msg.c_str(), (int)msg.size());
}

void TCP::parseMessages(std::vector<Message*>& messages, std::vector<std::string>& msgs)
{
    for (auto& message : msgs)
    {
        std::vector<std::string> tokens;
        splitMessages(tokens, message, TOKEN);

        if (tokens.empty())
            continue;
#ifndef ef
#define ef else if

        Opcodes opcode;
        if (tokens[0] == "CMSG_LOGIN") opcode = CMSG_LOGIN;
        ef (tokens[0] == "CMSG_SESSION") opcode = CMSG_SESSION;
        ef (tokens[0] == "CMSG_LOBBY_LIST") opcode = CMSG_LOBBY_LIST;
        ef (tokens[0] == "CMSG_CREATE_LOBBY") opcode = CMSG_CREATE_LOBBY;
        ef (tokens[0] == "CMSG_JOIN_LOBBY") opcode = CMSG_JOIN_LOBBY;
        ef (tokens[0] == "CMSG_LEAVE_LOBBY") opcode = CMSG_LEAVE_LOBBY;
        ef (tokens[0] == "CMSG_START_GAME") opcode = CMSG_START_GAME;
        ef (tokens[0] == "CMSG_KICK_PLAYER") opcode = CMSG_KICK_PLAYER;
        ef (tokens[0] == "CMSG_FF") opcode = CMSG_FF;
        ef (tokens[0] == "CMSG_DRAW") opcode = CMSG_DRAW;
        ef (tokens[0] == "CMSG_CARDS") opcode = CMSG_CARDS;
        ef (tokens[0] == "CMSG_PLAY") opcode = CMSG_PLAY;
        ef (tokens[0] == "CMSG_DISCARD") opcode = CMSG_DISCARD;
        ef (tokens[0] == "CMSG_QUIT") opcode = CMSG_QUIT;
        else
            continue;
#undef ef
#endif
        tokens.erase(tokens.begin());
        if (validateMessage(opcode, tokens))
        {
            std::stringstream data;
            messages.push_back(new Message(opcode, tokens));
            recvMessages++;
        }
    }
}

bool TCP::validateMessage(enum Opcodes opcode, std::vector<std::string>& tokens)
{
    switch (opcode)
    {
        case CMSG_LOGIN:
        case CMSG_SESSION:
        case CMSG_CREATE_LOBBY:
        case CMSG_JOIN_LOBBY:
        case CMSG_KICK_PLAYER:
        case CMSG_PLAY:
            return !tokens.empty();
            break;
        case CMSG_LOBBY_LIST:
        case CMSG_LEAVE_LOBBY:
        case CMSG_START_GAME:
        case CMSG_FF:
        case CMSG_DRAW:
        case CMSG_CARDS:
        case CMSG_QUIT:
            tokens.clear();
            return true;
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
    } while (bytes_read == sizeof(buf));

    std::vector<std::string> msgs;
    splitMessages(msgs, message, DELIMITER);
    incompleteMessage += message;
    parseMessages(messages, msgs);

    return true;
}

//#ifdef WIN32
bool TCP::select(std::vector<Message*>& messages, std::string& incompleteMessage)
{
    fd_set readfds;
    //clear the socket fd set
    FD_ZERO(&readfds);

    FD_SET(_socket, &readfds);
#ifdef WIN32
    TIMEVAL tv = { 0 };
#else
    struct timeval tv = { 0 };
#endif
    tv.tv_sec = 0;
    tv.tv_usec = 10 * IN_MICROSECONDS;

    int ret = ::select(_socket + 1, &readfds, NULL, NULL, &tv);
    if (ret == -1)
    {
        interrupted++;
        return false;
    }
    else if (ret == 0)
        return true; // timeout

    if (FD_ISSET(_socket, &readfds))
    {
        if (!recieve(messages, incompleteMessage))
            return false;
    }

    return true;
}

void TCP::DieWithError(char *errorMessage)
{
#ifdef WIN32
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
#else
    perror(errorMessage);
#endif
    assert(false);
}

void TCP::printStats()
{
    printf("TCP stats:\n");
    printf("Connections - accepted: %lld closed: %lld interrupted: %lld\n", (uint64)accepted, (uint64)closed, (uint64)interrupted);
    printf("Bytes - recv: %lld send: %lld\n", (uint64)recvBytes, (uint64)sendBytes);
    printf("Messages - recv: %lld send: %lld\n", (uint64)recvMessages, (uint64)sendMessages);
}
