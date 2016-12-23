#pragma once

#include <string>
#include <vector>

#include "Player.h"

class Lobby
{
public:

    Lobby(Player* _owner, std::string _name)
        : owner(_owner)
        , name(_name)
    {
        players.push_back(owner);
    }
    
    friend bool operator==(const Lobby& lobby, const std::string& string);
    friend bool operator==(const Lobby& lobby, const Player& player);


    std::string name;
    Player* owner;
    std::vector<Player*> players;
};


inline bool operator==(const Lobby* lobby, const std::string& string)
{
    return lobby->name == string;
}

inline bool operator==(const Lobby* lobby, const Player& player)
{
    for (auto* p : lobby->players)
    {
        if (player.name == p->name)
            return true;
    }
    return false;
}
