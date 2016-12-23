#pragma once
#include "Lobby.h"

Lobby::Lobby(Player* _owner, std::string _name)
    : owner(_owner)
    , name(_name)
{
    players.push_back(owner);
}

