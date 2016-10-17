//
// Created by Stepan on 16.10.2016.
//

#ifndef SERVER_PACKET_H
#define SERVER_PACKET_H


class Packet
{
    Packet(uint8 opcode, std::vector<char> data)
        : _opcode(opcode)
        , _data(data)
    { }

private:
    uint8 opcode;
    std::vector<char> data;
};


#endif //SERVER_PACKET_H
