#include "Server.h"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <string.h>

#include "Tcp.h"
#include "Message.h"
#include "Game.h"
#include "Lobby.h"
#include "Player.h"


Server::Server(uint16 port)
{
    TCP* server = new TCP();
    server->bind(port);
    _port = port;
    server->listen();
    _server = server;
    _state = RUNNING;
}

void Server::run()
{
    std::thread acceptThread([&]
    {
        while (_state == RUNNING)
        {
            _server->accept([&](int socket)
            {
                std::thread([&, socket]
                {
                    printf("%d - New connection\n", socket);
                    TCP* client = new TCP(socket);

                    std::string message;
                    char data[257];
                    memset(data, 0, sizeof(data));

                    bool found = false;
                    uint32 i = 1;
                    while (!found)
                    {
                        if (client->recv(data, sizeof(data)) <= 0)
                        {
                            delete client;
                            return;
                        }

                        message += data;

                        bool escaped = false;
                        for (i = 1; i < message.size(); i++)
                        {
                            if (message[i] == DELIMITER && !escaped)
                            {
                                found = true;
                                break;
                            }

                            if (message[i] == ESCAPE && !escaped)
                                escaped = true;
                            else
                                escaped = false;
                        }
                    }

                    std::string firstMsg = message.substr(0, i);
                    std::string secondMsg;
                    if (i + 1 < message.size())
                        secondMsg = message.substr(i + 1);

                    std::vector<std::string> tokens;                    
                    splitMessages(tokens, firstMsg, TOKEN);

                    if (tokens.size() < 2)
                    {
                        delete client;
                        return;
                    }

                    if (tokens[0] == "CMSG_LOGIN")
                    {
                        if (checkNickName(tokens[1]))
                        {
                            printf("%d - New player name: %s\n", socket, tokens[1].c_str());
                            Player* player = new Player(client, tokens[1], secondMsg, _messages, this);
                            addNewPlayer(player);
                            return;
                        }
                        printf("%d - Nick %s already taken disconnecting\n", socket, tokens[1].c_str());
                    }
                    if (tokens[0] == "CMSG_SESSION")
                    {
                        if (Player* player = getPlayerBySession(tokens[1]))
                        {
                            printf("%d - Reconnected from session: %s\n", socket, tokens[1].c_str());
                            player->Reconnect(client, secondMsg);
                            return;
                        }
                        printf("%d - Session %s not found disconnecting\n", socket, tokens[1].c_str());
                    }
                    delete client;
                }).detach();
            });
        }
    });

    std::thread inputThread([&]
    {
        std::string command;
        while (std::cin >> command)
        {
            if (command == "exit")
            {
                break;
            }
            if (command == "stats")
                TCP::printStats();
        }
        _state = EXITED;
        printf("State set to %d\n", (int)_state);
    });

    while (_state == RUNNING)
    {
        updateDisconnected(DIFF);
        updatePlayers();
        updateGames();
        updateLobbies();

        std::this_thread::sleep_for(std::chrono::milliseconds(DIFF));
    }
    printf("Exiting...\n");

    delete _server;

    if (acceptThread.joinable())
        acceptThread.join();

    if (inputThread.joinable())
        inputThread.join();
};

void Server::updateDisconnected(uint32 diff)
{
    Guard guard(_disconnectedLock);
    std::vector<Player*> toRemove;
    for (auto& pair : _disconnectedPlayers)
    {
        if (pair.second <= diff)
        {
            toRemove.push_back(pair.first);
        }
        else pair.second -= diff;

    }
    for (auto player : toRemove)
    {
        _disconnectedPlayers.erase(player);
        removePlayer(player);
    }
}

void Server::addToDisconected(Player* player)
{
    Guard guard(_disconnectedLock);
    _disconnectedPlayers[player] = DISCONNECT_TIMEOUT;
}

void Server::removeFromDisconected(Player* player)
{
    Guard guard(_disconnectedLock);
    _disconnectedPlayers.erase(player);
}

void Server::removePlayer(Player* player)
{
    {
        Guard guard(_playersLock);
        _loggedPlayers.erase(std::find(_loggedPlayers.begin(), _loggedPlayers.end(), player));
    }
    if (player->game)
        player->game->RemovePlayer(player);
    if (Lobby* lobby = player->getLobby())
    {
        Guard guard(lobbyLock);
        auto itr = std::find(lobbies.begin(), lobbies.end(), lobby);
        if (itr != lobbies.end() && lobby->owner == player)
        {
            lobbies.erase(itr);
            delete lobby;
        }
    }

    delete player;
}

// Check for ended games, move players to server and delete game
void Server::updateGames()
{
    std::vector<Game*> toDelete;
    for (auto* game : _games)
    {
        if (!game->isRunning())
        {
            toDelete.push_back(game);
        }
    }
    for (auto* game : toDelete)
    {
        _games.erase(std::find(_games.begin(), _games.end(), game));
        delete game;
    }
}

void Server::createLobby(PlayerMessage* pm)
{
    if (pm->player->getLobby())
    {
        pm->player->sendMessage("You are already in lobby!");
        return;
    }
    if (pm->message->data.empty())
    {
        pm->player->sendMessage("Name of lobby cant be empty!");
        return;
    }
    Guard guard(lobbyLock);
    auto itr = std::find(lobbies.begin(), lobbies.end(), pm->message->data);
    
    if (itr == lobbies.end())
    {
        Lobby* lobby = new Lobby(pm->player, pm->message->data);
        lobbies.push_back(lobby);
        pm->player->joinLobby(lobby, true);
    }
    else
    {
        pm->player->sendMessage("Lobby with this name already exists!");
    }

}
void Server::joinLobby(PlayerMessage* pm)
{
    if (pm->player->getLobby())
    {
        pm->player->sendMessage("You are already in lobby!");
        return;
    }
    if (pm->message->data.empty())
    {
        pm->player->sendMessage("Name of lobby cant be empty!");
        return;
    }
    Guard guard(lobbyLock);

    auto itr = std::find(lobbies.begin(), lobbies.end(), pm->message->data);

    if (itr != lobbies.end())
    {
        if ((*itr)->players.size() >= 4)
            pm->player->sendMessage("Lobby is full!");
        else
            pm->player->joinLobby(*itr);
    }
    else
    {
        pm->player->sendMessage("Lobby with this name does not exists!");
    }

}
void Server::leaveLobby(PlayerMessage* pm)
{
    Guard guard(lobbyLock);
    Lobby* lobby = pm->player->getLobby();

    if (lobby)
    {
        pm->player->leaveLobby(lobby);
        auto itr = std::find(lobbies.begin(), lobbies.end(), lobby);
        if (itr != lobbies.end() && lobby->owner == pm->player)
        {
            lobbies.erase(itr);
            delete lobby;
        }
    }
    else
    {
        pm->player->sendMessage("You are not in a lobby!");
    }
}

void Server::kickPlayer(PlayerMessage * pm)
{
    Guard guard(lobbyLock);
    Lobby* lobby = pm->player->getLobby();

    if (lobby)
    {
        if (lobby->owner != pm->player)
        {
            pm->player->sendMessage("You are not lobby owner!");
            return;
        }

        Player* target = lobby->findPlayer(pm->message->data);
        if (!target)
        {
            pm->player->sendMessage("Player "+pm->message->data+" is not in your lobby!");
            return;
        }
        target->sendMessage("You have been kicked from lobby!");
        target->leaveLobby(lobby);
        auto itr = std::find(lobbies.begin(), lobbies.end(), lobby);
        if (itr != lobbies.end() && lobby->owner == target)
        {
            lobbies.erase(itr);
            delete lobby;
        }
    }
    else
    {
        pm->player->sendMessage("You are not in a lobby!");
    }
}

void Server::startGame(PlayerMessage* pm)
{
    if (Lobby* lobby = pm->player->getLobby())
    {
        if (lobby->owner != pm->player)
        {
            pm->player->sendMessage("You are not lobby owner!");
            return;
        }
        if (lobby->players.size() < 2)
        {
            pm->player->sendMessage("You need at least 2 players to start a game!");
            return;
        }

        _games.push_back(new Game(lobby->players));
        
        Guard guard(lobbyLock);
        lobby->players.clear();
        auto itr = std::find(lobbies.begin(), lobbies.end(), lobby);
        if (itr != lobbies.end())
            lobbies.erase(itr);
        delete lobby;
        
    }
    else
        pm->player->sendMessage("You are not in a lobby!");

}

void Server::updateLobbies()
{
    Guard guard(lobbyLock);
    std::vector<std::vector<Lobby*>::iterator> toRemove;
    for (auto itr = lobbies.begin(); itr != lobbies.end(); itr++)
        if (!(*itr)->owner)
            toRemove.push_back(itr);
    for (auto itr : toRemove)
    {
        Lobby* l = *itr;
        delete l;
        lobbies.erase(itr);
    }
}

void Server::updatePlayers()
{
    while(auto* message = _messages.pop())
    {
        switch ((Opcodes)*message->message)
        {
        case CMSG_LOBBY_LIST:
            message->player->sendLobbyList();
            break;
        case CMSG_CREATE_LOBBY:
            createLobby(message);
            break;
        case CMSG_JOIN_LOBBY:
            joinLobby(message);
            break;
        case CMSG_LEAVE_LOBBY:
            leaveLobby(message);
            break;
        case CMSG_START_GAME:
            startGame(message);
            break;
        case CMSG_KICK_PLAYER:
            kickPlayer(message);
            break;
        case CMSG_QUIT:
            removePlayer(message->player);
            break;
        default:
            //kick
            break;
        }
        delete message;
    }
}

bool Server::checkNickName(std::string const& name)
{
    if (name.empty())
        return false;

    Guard lock(_playersLock);
    for (auto player : _loggedPlayers)
        if (player->name == name)
            return false;

    return true;
}

void Server::addNewPlayer(Player* player)
{
    Guard lock(_playersLock);
    _loggedPlayers.push_back(player);
}

Player* Server::getPlayerBySession(std::string session)
{
    if (session.empty())
        return nullptr;

    Guard lock(_playersLock);
    for (auto player : _loggedPlayers)
        if (player->session == session)
            return player;

    return nullptr;
}
