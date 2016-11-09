// server.cpp : Defines the entry point for the console application.
//
#include "tcp.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "utils.hpp"
#include <vector>
#include <mutex>

class Server
{
public:
    Server(uint16 port)
    {
        TCP* server = new TCP();
        server->bind(port);
        server->listen();
        _server = server;
    }

    void run()
    {
        // start thread to update players in lobby
        // start thread to accept players
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
                            addNewPlayer(new Player(client, name));
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
    
private:
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
    std::mutex _playersLock;
    TCP* _server;

};
