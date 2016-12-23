// server.cpp : Defines the entry point for the console application.
//
#pragma once

#include <vector>
#include <mutex>
#include <map>
#include <atomic>

#include "utils.h"

class Player;
class Game;
class Lobby;
class TCP;
class PlayerMessage;

enum ServerState
{
    RUNNING,
    EXITED
};


class Server
{
public:
    Server(uint16 port);
    
    void run();
    
private:

    void updateDisconnected(uint32 diff);

    void addToDisconected(Player* player);

    void removePlayer(Player* player);
    
    // Check for ended games, move players to server and delete game
    void updateGames();

    void updatePlayers();

    bool checkNickName(std::string const& name);

    void addNewPlayer(Player* player);

    Player* getPlayerBySession(std::string session);

    void createLobby(PlayerMessage* pm);
    void joinLobby(PlayerMessage* pm);
    void leaveLobby(PlayerMessage* pm);

public:
    std::mutex lobbyLock;
    std::vector<Lobby*> lobbies;
private:
    std::vector<Player*> _loggedPlayers;
    std::map<Player*, uint32> _disconnectedPlayers;
    std::vector<PlayerMessage*> _messages;
    std::vector<Game*> _games;
    std::atomic<ServerState> _state;
    std::mutex _playersLock;
    std::mutex _disconnectedLock;
    TCP* _server;
    uint16 _port;
};
