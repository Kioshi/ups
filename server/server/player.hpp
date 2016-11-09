#pragma once

#include <thread>
#include <string>
#include <atomic>

enum eStreams
{
    DOWNSTREAM,
    UPSTREAM,
    MAX_STREAMS
};

enum eState
{
    READY,
    DISCONNECTED,
    RECONNECTING,
    WAITING_FOR_DELETE
};

class Player
{
public:
    Player(TCP* socket, std::string _name)
        : _socket(socket)
        , name(_name)
        , session(_name.rbegin(),_name.rend())
        , _state(READY)
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

    void Update()
    {
        if (_state != READY)
        {
            if (networkThread && _state == DISCONNECTED)
            {
                networkThread->join();
                delete networkThread;
                networkThread = nullptr;
            }
            return;
        }

        for (auto packet: _recieved)
        {
        }
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
private:
    void createNetworkThread()
    {
        networkThread = new std::thread([&]
        {
            while (_state == READY)
            {
                if (!_socket->winSelect(_recieved))
                {
                    delete _socket;
                    _state = DISCONNECTED;
                    return;
                }
            }
        });
        networkThread->detach();
    }

public:
    std::string name;
    std::string session;

private:
    eState _state;
    TCP* _socket;
    std::thread* networkThread;
    std::vector<Packet*> _recieved;
};
