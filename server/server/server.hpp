// server.cpp : Defines the entry point for the console application.
//
#include "tcp.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "utils.hpp"
#include "Game.hpp"
#include "Lobby.hpp"
#include <vector>
#include <mutex>
#include <map>
#include <iostream>

enum ServerState
{
    RUNNING,
    EXITED
};


class Server
{
public:
    Server(uint16 port)
    {
        TCP* server = new TCP();
        server->bind(port);
        server->listen();
        _server = server;
        _state = RUNNING;
    }

    void run()
    {
        std::thread acceptThread([&]
        {
            while (_state == RUNNING)
            {
                _server->accept([&](int socket)
                {
                    std::thread([&]
                    {
                        TCP* client = new TCP(socket);
                        Packet* recv = client->recieve();
                        if (!recv)
                        {
                            delete client;
                            return;
                        }

                        switch ((Opcodes)*recv)
                        {
                        case LOGIN:
                        {
                            std::string name(recv->data);
                            if (checkNickName(name))
                            {
                                std::string session(name.rbegin(), name.rend());
                                addNewPlayer(new Player(client, name, _packets));
                                delete recv;
                                return;
                            }
                            break;
                        }
                        case SESSION:
                            if (Player* player = getPlayerBySession(recv->data))
                            {
                                player->Reconnect(client);
                                delete recv;
                                return;
                            }
                            break;
                        default: break;
                        }
                        delete recv;
                        delete client;
                    }).detach();
                });
            }
        });
        acceptThread.detach();

        std::thread inputTHread([&]
        {
            std::string command;
            while (std::cin >> command)
            {
                if (command == "exit")
                {
                    _state = EXITED;
                    break;
                }
            }
            _state = EXITED;
        });

        while (_state == RUNNING)
        {
            updateDisconnected(DIFF);
            updatePlayers();
            updateGames();

            std::this_thread::sleep_for(std::chrono::milliseconds(DIFF));
        }

        inputTHread.join();
        acceptThread.join();
    };
    
private:

    void updateDisconnected(uint32 diff)
    {
        std::lock_guard<std::mutex> guard(_disconnectedLock);
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

    void addToDisconected(Player* player)
    {
        std::lock_guard<std::mutex> guard(_disconnectedLock);
        _disconnectedPlayers[player] = DISCONNECT_TIMEOUT;
    }

    void removePlayer(Player* player)
    {
        {
            std::lock_guard<std::mutex> guard(_playersLock);
            _loggedPlayers.erase(std::find(_loggedPlayers.begin(), _loggedPlayers.end(), player));
        }
        if (player->game)
            player->game->RemovePlayer(player);
        delete player;
    }
    
    // Check for ended games, move players to server and delete game
    void updateGames()
    {
        std::vector<Game*> toDelete;
        for (auto* game: _games)
        {
            if (!game->isRunning())
            {
                toDelete.push_back(game);
            }
        }
        for (auto* game: toDelete)
        {
            _games.erase(std::find(_games.begin(), _games.end(), game));
            delete game;
        }
    }

    void updatePlayers()
    {
        for (auto packet: _packets)
        {
            switch ((Opcodes)*packet->packet)
            {
                case CREATE_LOBBY:
                case START_GAME:
                case JOIN_LOBBY:
                case LEAVE_LOBBY:
                case KICK_PLAYER:
                case SEND_MESSAGE:
                case QUIT:
                        break;
                    default:
                        //kick
                        break;
            }
        }
    }

    bool checkNickName(std::string const& name)
    {
        if (name.empty())
            return false;

        std::lock_guard<std::mutex> lock(_playersLock);
        for (auto player : _loggedPlayers)
            if (player->name == name)
                return false;

        return true;
    }

    void addNewPlayer(Player* player)
    {
        std::lock_guard<std::mutex> lock(_playersLock);
        _loggedPlayers.push_back(player);
    }

    Player* getPlayerBySession(std::string session)
    {
        if (session.empty())
            return nullptr;

        std::lock_guard<std::mutex> lock(_playersLock);
        for (auto player : _loggedPlayers)
            if (player->session == session)
                return player;

        return nullptr;
    }


private:
    std::vector<Player*> _loggedPlayers;
    std::map<Player*, uint32> _disconnectedPlayers;
    std::vector<Lobby*> _lobbys;
    std::vector<PlayerPacket*> _packets;
    std::vector<Game*> _games;
    std::atomic<ServerState> _state;
    std::mutex _playersLock;
    std::mutex _disconnectedLock;
    TCP* _server;
};
