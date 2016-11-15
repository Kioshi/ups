#pragma once
#include <memory>
#include <string.h>
#include "utils.hpp"

enum Opcodes
{
    LOGIN = 0,
    SESSION,
    CREATE_LOBBY,
    START_GAME,
    JOIN_LOBBY,
    LEAVE_LOBBY,
    KICK_PLAYER,
    SEND_MESSAGE,
    QUIT,
    UNUSED, //placehoder
};


class Packet
{
public:
    Packet(Opcodes _opcode, uint8 _size, const char* _data)
        : opcode(_opcode)
        , size(_size)
    {
        memcpy(data, _data, 256);
    }

    Packet(Opcodes _opcode)
        : opcode(_opcode)
        , size(0)
    {
    }

    explicit operator Opcodes() const
    {
        return opcode;
    }

    int getSize()
    {
        return 1 + (size ? 1 : 0) + size;
    }
    char* toCharArray()
    {
        char * ch = new char[getSize()];
        ch[0] = opcode;
        if (size)
        {
            ch[1] = size;
            memcpy(ch+2, data, size);
        }
        return ch;
    }

    Opcodes opcode;
    uint8 size;
    char data[256];
};
