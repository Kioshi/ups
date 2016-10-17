//
// Created by Stepan on 15.10.2016.
//

#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "Packet.h"

class Socket
{
    Socket(SOCKET socketId, eType type)
        : _socketId(socketId)
        , _running(true)
    {
        _thread = new std::thread([=,type]
        {
            if (type == UPSTREAM)
                recievePackets();
            else
                sendPackets();
        });
    }

    ~Socket()
    {
        _running = false;
        closesocket(socketId);
        _thread.join();
    }

    void addPacket(Packet* packet)
    {
        Guard(_lock);
        packets.push_back(packet);
    }

    void sendPackets()
    {
        while(_running)
        {
            std::vector<Packet*> packets;
            {
                Guard(_lock);
                packets(_packets);
                _packets.clear();
            }
            for (Packet *packet : packets)
            {
                sendPacket(packet);
            }

            sleep(1);
        }
    }

    void sendPacket(Packet* packet)
    {
    }

    void recievePackets()
    {
        while(_running)
        {
            Packet* packet = recvPacket();
            Guard(_lock);
            _packets.push_back(packet);
        }
    }

    std::vector<Packet*> getPackets()
    {
        Guard(_lock);
        std::vector<Packet*> packets;
        packets(_packets);
        _packets.clear();
        return packets;
    }

private:
    std::vector<Packet*> _packets;
    std::lock _lock;
    std::atomic<bool> _running;
    std::thread _thread;
    SOCKET _socket;
};


#endif //SERVER_SOCKET_H
