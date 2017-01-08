#pragma once

#include "utils.h"
#include "Message.h"
#include <vector>
#include <functional>

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
    
private:
    void DieWithError(char *errorMessage);
    int _socket;
    static uint32 _connections;
};