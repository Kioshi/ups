#pragma once
#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include "utils.h"

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
    SEND_MESSAGE,
    QUIT,
    UNUSED, //placehoder
};


class Message
{
public:
    Message(Opcodes _opcode, std::vector<std::string> tokens)
        : opcode(_opcode)
    {
        bool first = true;
        std::stringstream ss;
        for (auto& token : tokens)
        {
            if (!first)
                ss << " ";
            else
                first = false;
            ss << token;
        }

        data = ss.str();
    }

    Message(Opcodes _opcode, std::string _string)
        : opcode(_opcode)
        , data(_string)
    {
    }

    explicit operator Opcodes() const
    {
        return opcode;
    }
    
    Opcodes opcode;
    std::string data;
};
