#pragma once
#include "Lobby.h"
#include <algorithm>

Lobby::Lobby(Player* _owner, std::string _name)
    : owner(_owner)
    , name(_name)
{
    players.push_back(owner);
}

Lobby::~Lobby()
{
    if (players.empty())
        return;

    for (auto player : players)
        if (player != owner)
            player->leaveLobby(this, true);
}

void Lobby::onPlayerJoined(Player* player)
{
    for (auto p : players)
        p->sendMessage("Player " + player->name + " joined lobby");
    players.push_back(player);
}

void Lobby::onPlayerLeft(Player* player)
{
    if (players.empty())
        return;

    auto itr = std::find(players.begin(), players.end(), player);
    if (itr == players.end())
        return;

    players.erase(itr);
    for (auto p : players)
        p->sendMessage("Player " + player->name + " left lobby");
}


Player* Lobby::findPlayer(std::string name)
{
    for (auto player : players)
        if (player->name == name)
            return player;
    return nullptr;
}
