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

Player::Player(TCP* socket, std::string _name, std::string incompleteMessage, LFQueue<PlayerMessage>& argMessages, Server* server)
    : _socket(socket)
    , name(_name)
    , session(_name.rbegin(), _name.rend())
    , _state(READY)
    , game(nullptr)
    , _messages(argMessages)
    , dead(false)
    , _incompleteMessage(incompleteMessage)
    , _server(server)
    , _lobby(nullptr)
{
    _socket->send(new Message(SESSION, session));

    std::vector<std::string> msgs;
    splitMessages(msgs, _incompleteMessage, DELIMITER);
    std::vector<Message*> messages;
    TCP::parseMessages(messages, msgs);
    LFQueue<PlayerMessage>& pmessages = game ? game->GetMessages() : _messages;
    for (auto message : messages)
        pmessages.push(new PlayerMessage(this, message));

    sendLobbyList();
    createNetworkThread();
}

Player::~Player()
{
    _state = WAITING_FOR_DELETE;
    if (networkThread)
        networkThread->join();
    if (_lobby)
        _lobby->onPlayerLeft(this);

}

void Player::sendLobbyList()
{
    std::string msg;
    bool first = true;
    if (_lobby)
    {
        for (auto p : _lobby->players)
        {
            if (first)
                first = false;
            else
                msg += " ";
            msg += p->name;
        }

    }
    else
    {
        Guard guard(_server->lobbyLock);
        for (auto* lobby : _server->lobbies)
        {
            if (first)
                first = false;
            else
                msg += " ";
            //msg += escapeString(lobby->name);
            msg += lobby->name;
        }
    }
    _toSend.push(new Message(LOBBY_LIST, msg));
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
    _server->removeFromDisconected(this);
    _state = READY;
    _socket->send(new Message(SESSION, session));
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
                _state = DISCONNECTED;
                break;
            }
            else
            {
                LFQueue<PlayerMessage>& pmessages = game ? game->GetMessages() : _messages;
                for (auto message : messages)
                    pmessages.push(new PlayerMessage(this, message));
            }
            while (auto* msg = _toSend.pop())
            {
                _socket->send(msg);
                delete msg;
            }
        }

        delete _socket;
        _socket = nullptr;
        if (_state == DISCONNECTED)
            _server->addToDisconected(this);
        if (_lobby)
            leaveLobby(_lobby);
    });
}

void Player::joinLobby(Lobby* lobby, bool create)
{
    _lobby = lobby;
    _toSend.push(new Message(create ? CREATE_LOBBY : JOIN_LOBBY, lobby->name));
    if (!create)
        lobby->onPlayerJoined(this);
}

void Player::leaveLobby(Lobby* lobby, bool removing)
{
    _lobby = nullptr;
    _toSend.push(new Message(LEAVE_LOBBY));
    if (!removing)
        lobby->onPlayerLeft(this);
}

void Player::sendMessage(std::string msg)
{
    _toSend.push(new Message(MESSAGE,msg));
}

Lobby* Player::getLobby()
{
    return _lobby;
}

void Player::disconnect()
{
    _state = DISCONNECTED;
}