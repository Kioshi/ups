#pragma once

#include "utils.h"
#include "Message.h"
#include <vector>
#include <functional>
#include <atomic>

class Message;

class TCP
{
public:
    TCP(int sock = 0);

    ~TCP();

    int socket();

    int bind(unsigned short port);

    int listen();

    void accept(std::function<void(int)> lambda);

    int recv(char * buffer, int lenght);

    void send(const char * buffer, int lenght);

    void send(Message* message);

    bool recieve(std::vector<Message*>& messages, std::string& incompleteMessage);

    bool select(std::vector<Message*>& messages, std::string& incompleteMessage);

    static void parseMessages(std::vector<Message*>& messages, std::vector<std::string>& msgs);
    static bool validateMessage(enum Opcodes opcode, std::vector<std::string>& tokens);

    static void printStats();
    
private:
    void DieWithError(char *errorMessage);
    int _socket;
    static std::atomic<uint32> _connections;
    // stats
    static std::atomic<uint64> sendBytes;
    static std::atomic<uint64> recvBytes;
    static std::atomic<uint64> sendMessages;
    static std::atomic<uint64> recvMessages;
    static std::atomic<uint64> accepted;
    static std::atomic<uint64> closed;
    static std::atomic<uint64> interrupted;
};