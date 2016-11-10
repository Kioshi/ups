#pragma once
#include <memory>

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
        return 1 + size ? 1 : 0 + size;
    }

    Opcodes opcode;
    uint8 size;
    char data[256];
};
