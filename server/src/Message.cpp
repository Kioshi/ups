#pragma once
#include "Message.h"

#include <sstream>
#include "utils.h"

Message::Message(Opcodes _opcode, std::vector<std::string> tokens)
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

Message::Message(Opcodes _opcode, std::string _string)
    : opcode(_opcode)
    , data(_string)
{
}

Message::Message(Opcodes _opcode)
    : opcode(_opcode)
    , data("")
{
}

Message::operator Opcodes() const
{
    return opcode;
}
    
std::string Message::toString()
{
    std::string op = "";
    switch (opcode)
    {
        case LOBBY_LIST: op = "LOBBY_LIST"; break;
        case CREATE_LOBBY: op = "CREATE_LOBBY"; break;
        case JOIN_LOBBY: op = "JOIN_LOBBY"; break;
        case LEAVE_LOBBY: op = "LEAVE_LOBBY"; break;
        case START_GAME: op = "START_GAME"; break;
        case KICK_PLAYER: op = "KICK_PLAYER"; break;
        case FF: op = "FF"; break;
        case CARD: op = "CARD"; break;
        case PLAY: op = "PLAY"; break;
        case MESSAGE: op = "MESSAGE"; break;
    }
    return op + (data.empty() ? "" : (" " + data)) + (char)DELIMITER;
}
