#pragma once
#include <string>
#include <vector>

enum Opcodes
{
    LOGIN = 0,
    SESSION,
    LOBBY_LIST,
    CREATE_LOBBY,
    JOIN_LOBBY,
    LEAVE_LOBBY,
    START_GAME,
    KICK_PLAYER,
    FF,
    CARD,
    PLAY,
    QUIT,
    MESSAGE,
    UNUSED, //placehoder
};

class Message
{
public:
    Message(Opcodes _opcode, std::vector<std::string> tokens);

    Message(Opcodes _opcode, std::string _string);

    Message(Opcodes _opcode);

    explicit operator Opcodes() const;
    
    Opcodes opcode;
    std::string data;

    std::string toString();
};
