#include "Message.h"

#include <sstream>
#include <assert.h>
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
        case SMSG_SESSION: op = "SMSG_SESSION"; break;
        case SMSG_LOBBY_LIST: op = "SMSG_LOBBY_LIST"; break;
        case SMSG_CREATE_LOBBY: op = "SMSG_CREATE_LOBBY"; break;
        case SMSG_JOIN_LOBBY: op = "SMSG_JOIN_LOBBY"; break;
        case SMSG_LEAVE_LOBBY: op = "SMSG_LEAVE_LOBBY"; break;
        case SMSG_START_GAME: op = "SMSG_START_GAME"; break;
        case SMSG_KICK_PLAYER: op = "SMSG_KICK_PLAYER"; break;
        case SMSG_FF: op = "SMSG_FF"; break;
        case SMSG_DRAW: op = "SMSG_DRAW"; break;
        case SMSG_PLAY: op = "SMSG_PLAY"; break;
        case SMSG_CARDS: op = "SMSG_CARDS"; break;
        case SMSG_DISCARD: op = "SMSG_DISCARD"; break;
        case SMSG_MESSAGE: op = "SMSG_MESSAGE"; break;
        case SMSG_GAME_END: op = "SMSG_GAME_END"; break;
        default:
            assert(!(opcode < UNUSED));
            break;
    }
    return op + (data.empty() ? "" : (" " + data)) + (char)DELIMITER;
}
