#pragma once

#include <thread>
#include <string>
#include <atomic>
#include "Game.hpp"
#include "Lobby.hpp"
#include "tcp.hpp"

enum eStreams
{
    DOWNSTREAM,
    UPSTREAM,
    MAX_STREAMS
};

enum PlayerState
{
    READY,
    DISCONNECTED,
    RECONNECTING,
    WAITING_FOR_DELETE
};

class PlayerPacket
{
public:
    PlayerPacket(class Player* _player, Packet* _packet)
        : player(_player)
        , packet(_packet)
    {}

    Player* player;
    Packet* packet;
};

class Player
{
public:
    Player(TCP* socket, std::string _name, std::vector<PlayerPacket*> packets)
        : _socket(socket)
        , name(_name)
        , session(_name.rbegin(), _name.rend())
        , _state(READY)
        , game(nullptr)
        , _packets(packets)
        , dead(false)
    {
        _socket->send(new Packet(SESSION, (uint8)session.size()+1, session.c_str()));
        createNetworkThread();
    }

    ~Player()
    {
        _state = WAITING_FOR_DELETE;
        if (networkThread)
            networkThread->join();

    }


    void Reconnect(TCP* socket)
    {
        _state = RECONNECTING;
        if (_socket)
            delete _socket;
        _socket = socket;
        if (networkThread)
        {
            networkThread->join();
            delete networkThread;
        }
        createNetworkThread();
    }

    bool isActive()
    {
        return _state == READY && !dead;
    }

private:
    void createNetworkThread()
    {
        networkThread = new std::thread([&]
        {
            while (_state == READY)
            {
                std::vector<Packet*> packets;
                if (!_socket->select(packets))
                {
                    delete _socket;
                    _state = DISCONNECTED;
                    return;
                }
                else
                {
                    std::vector<PlayerPacket*>& ppackets = game ? game->GetPackets() : _packets;
                    for (auto packet : packets)
                        ppackets.push_back(new PlayerPacket(this, packet));
                }
            }
        });
        networkThread->detach();
    }

public:
    std::string name;
    std::string session;
    Game* game;
    bool dead;

private:
    PlayerState _state;
    TCP* _socket;
    std::thread* networkThread;
    std::vector<PlayerPacket*> _packets;
};
