#pragma once
#include "player.hpp"
#include <vector>

class Player;
class PlayerPacket;

class Game
{
public:
    Game(std::vector<Player*> players)
        : _running(true)
        , _players(players)
    {
    }

    bool isRunning()
    {
        return _running;
    }

    void Disconnected(Player* player)
    {
        std::lock_guard<std::mutex> guard(_lock);
        auto itr = std::find(_players.begin(), _players.end(), player);
        if (itr != _players.end())
        {
            _players.erase(itr);
            _disconnected.push_back(player);
        }
    }

    void RemovePlayer(Player* player)
    {
        std::lock_guard<std::mutex> guard(_lock);

        auto itr = std::find(_players.begin(), _players.end(), player);
        if (itr != _players.end())
            _players.erase(itr);

        itr = std::find(_disconnected.begin(), _disconnected.end(), player);
        if (itr != _disconnected.end())
            _disconnected.erase(itr);
    }

    std::vector<PlayerPacket*>& GetPackets()
    {
        return _packets;
    }
private:
    std::vector<Player*> _players;
    std::vector<Player*> _disconnected;
    std::vector<PlayerPacket*> _packets;
    std::atomic<bool> _running;
    std::mutex _lock;
};