#pragma once

#include <thread>
#include <string>
#include <atomic>
#include <deque>
#include <map>

#include "LockFree.h"
#include "utils.h"
#include "Message.h"
#include "Game.h"

enum eStreams
{
    DOWNSTREAM,
    UPSTREAM,
    MAX_STREAMS
};

enum PlayerState
{
    READY,
    DISCONNECTED,
    RECONNECTING,
    WAITING_FOR_DELETE
};

class Message;
class TCP;
class Server;
class Game;

class PlayerMessage
{
public:
    PlayerMessage(class Player* _player, Message* _message);

    Player* player;
    Message* message;
};

class Player
{
public:
    Player(TCP* socket, std::string _name, std::string incompleteMessage, LFQueue<PlayerMessage>& messages, Server* server);

    ~Player();

    void sendLobbyList();

    void Reconnect(TCP* socket, std::string incompleteMessage);

    bool isActive();

    void joinLobby(class Lobby* lobby, bool create = false);
    void leaveLobby(class Lobby* lobby, bool removing = false);
    void sendMessage(std::string error);
    void disconnect();

    bool haveCard(enum Card c, uint8 count=1);
    void removeCard(enum Card c, uint8 count = 1);
    Card removeRandomCard();
    void addCard(enum Card c);
    void peak(std::deque<Card>& pack);
    void play(std::string card);
    uint8 countCards();
    void send(enum Opcodes opcode, std::string msg);
    void startGame(std::string startingPlayer, class Game* game);
    void ff();
    void endGame(Player * winner);

    Lobby* getLobby();
    Server* getServer();

private:
    void createNetworkThread();
public:
    std::string name;
    std::string session;
    Game* game;
    bool dead;
    class Lobby* _lobby;

private:
    Server* _server;
    std::string _incompleteMessage;
    PlayerState _state;
    TCP* _socket;
    std::thread* networkThread;
    LFQueue<PlayerMessage>& _messages;
    LFQueue<Message> _toSend;
    std::map<Card, uint8> cards;
};
