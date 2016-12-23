#pragma once
#include "Message.h"

#include <sstream>

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

Message::operator Opcodes() const
{
    return opcode;
}
    
