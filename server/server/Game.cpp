#include "Game.hpp"
#include "player.hpp"
#include <algorithm>

Game::Game(std::vector<Player*> players)
    : _running(true)
    , _players(players)
{
}

bool Game::isRunning()
{
    return _running;
}

void Game::RemovePlayer(Player* player)
{
    std::lock_guard<std::mutex> guard(_lock);

    auto itr = std::find(_players.begin(), _players.end(), player);
    if (itr != _players.end())
        _players.erase(itr);
}

std::vector<PlayerPacket*>& Game::GetPackets()
{
    return _packets;
}

void Game::run()
{
    std::thread acceptThread([&]
    {
        while (_running)
        {
            processPackets();
            checkState();
        }
        //send announce winner
    });
    acceptThread.detach();

};

void Game::processPackets()
{
    for (auto packet : _packets)
    {
        //do shits
    }
}

void Game::checkState()
{
    uint8 activePlayers = 0;
    for (auto pl : _players)
        activePlayers += pl->isActive();
    _running = activePlayers > 1;
}