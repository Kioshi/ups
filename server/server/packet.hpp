#pragma once
#include <memory>

enum Opcodes
{
    LOGIN = 0,
    SESSION,
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

    explicit operator Opcodes() const
    {
        return opcode;
    }

    int getSize()
    {
        return 2 + size;
    }

    Opcodes opcode;
    uint8 size;
    char data[256];
};
