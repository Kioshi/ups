#include "Player.h"

#include "Game.h"
#include "Lobby.h"
#include "tcp.h"
#include "Server.h"
#include "Message.h"

PlayerMessage::PlayerMessage(Player* _player, Message* _message)
    : player(_player)
    , message(_message)
{
}

Player::Player(TCP* socket, std::string _name, std::string incompleteMessage, std::vector<PlayerMessage*> argMessages, Server* server)
    : _socket(socket)
    , name(_name)
    , session(_name.rbegin(), _name.rend())
    , _state(READY)
    , game(nullptr)
    , _messages(argMessages)
    , dead(false)
    , _incompleteMessage(incompleteMessage)
    , _server(server)
{
    _socket->send(new Message(SESSION, session));

    std::vector<std::string> msgs;
    splitMessages(msgs, _incompleteMessage, DELIMITER);
    std::vector<Message*> messages;
    TCP::parseMessages(messages, msgs);
    std::vector<PlayerMessage*>& pmessages = game ? game->GetMessages() : _messages;
    for (auto message : messages)
        pmessages.push_back(new PlayerMessage(this, message));

    sendLobbyList();
    createNetworkThread();
}

Player::~Player()
{
    _state = WAITING_FOR_DELETE;
    if (networkThread)
        networkThread->join();

}

void Player::sendLobbyList()
{
    Guard guard(_server->lobbyLock);
    for (auto* lobby : _server->lobbies)
    {
        toSend.push_back("lobby-list " + lobby->name);
    }
}

void Player::Reconnect(TCP* socket, std::string incompleteMessage)
{
    _state = RECONNECTING;
    if (_socket)
        delete _socket;
    _socket = socket;
    _incompleteMessage = incompleteMessage;
    if (networkThread)
    {
        networkThread->join();
        delete networkThread;
    }
    createNetworkThread();
}

bool Player::isActive()
{
    return _state == READY && !dead;
}

void Player::createNetworkThread()
{
    networkThread = new std::thread([&]
    {
        while (_state == READY)
        {
            std::vector<Message*> messages;
            if (!_socket->select(messages, _incompleteMessage))
            {
                delete _socket;
                _state = DISCONNECTED;
                return;
            }
            else
            {
                std::vector<PlayerMessage*>& pmessages = game ? game->GetMessages() : _messages;
                for (auto message : messages)
                    pmessages.push_back(new PlayerMessage(this, message));
            }
        }
    });
}

void Player::sendJoinedLobby()
{

}

void Player::sendLeftLobby()
{

}

void Player::sendError(std::string error)
{

}
