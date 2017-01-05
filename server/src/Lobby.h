#pragma once

#include <string>
#include <vector>
#include "Player.h"


class Lobby
{
public:

    Lobby(Player* _owner, std::string _name);
    ~Lobby();

    friend bool operator==(const Lobby& lobby, const std::string& string);
    friend bool operator==(const Lobby& lobby, const class Player& player);
    void onPlayerJoined(Player* player);
    void onPlayerLeft(Player* player);
    Player* findPlayer(std::string name);
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