#include "Game.h"

#include "Player.h"
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
    Guard guard(_lock);

    auto itr = std::find(_players.begin(), _players.end(), player);
    if (itr != _players.end())
        _players.erase(itr);
}

std::vector<PlayerMessage*>& Game::GetMessages()
{
    return _messages;
}

void Game::run()
{
    std::thread acceptThread([&]
    {
        while (_running)
        {
            processMessages();
            checkState();
        }
        //send announce winner
    });

};

void Game::processMessages()
{
    for (auto message : _messages)
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