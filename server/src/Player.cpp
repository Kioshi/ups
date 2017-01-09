#include "Player.h"

#include "Message.h"
#include "Game.h"
#include "Lobby.h"
#include "Tcp.h"
#include "Server.h"
#include <random>

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
    _socket->send(new Message(SMSG_SESSION, session));

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
    _toSend.push(new Message(SMSG_LOBBY_LIST, msg));
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
    _socket->send(new Message(SMSG_SESSION, session));
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

        if (_socket)
            delete _socket;
        _socket = nullptr;
        if (_state == DISCONNECTED)
            _server->addToDisconected(this);
        if (_lobby)
            leaveLobby(_lobby);
    });
}

void Player::endGame(Player * winner)
{
    game = nullptr;
    _toSend.push(winner ? new Message(SMSG_GAME_END,winner->name) : new Message(SMSG_GAME_END));
}

void Player::ff()
{
    game = nullptr;
    _toSend.push(new Message(SMSG_FF));
}

void Player::startGame(std::string startingPlayer, Game* _game)
{
    _lobby = nullptr;
    dead = false;
    game = _game;
    cards.clear();
    _toSend.push(new Message(SMSG_START_GAME, startingPlayer));
}

void Player::send(Opcodes opcode, std::string msg)
{
    _toSend.push(new Message(opcode, msg));
}

Card Player::removeRandomCard()
{
    auto item = cards.begin();

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, (int)cards.size());
    std::advance(item, distr(eng));
    Card c = item->first;
    removeCard(c);
    return c;
}

uint8 Player::countCards()
{
    uint8 count = 0;
    for (auto p : cards)
        count += p.second;
    return count;
}

void Player::play(std::string card)
{
    _toSend.push(new Message(SMSG_PLAY, card));
}

void Player::peak(std::deque<Card>& pack)
{
    std::string msg = "PEAK";
    for (uint8 i = 0; i < 3 && i < pack.size(); i++)
        msg += " " + Game::encodeCard(pack[i]);

    _toSend.push(new Message(SMSG_PLAY, msg));
}

bool Player::haveCard(Card c, uint8 count)
{
    auto itr = cards.find(c);
    return itr != cards.end() && itr->second >= count;
}

void Player::removeCard(Card c, uint8 count)
{
    auto itr = cards.find(c);
    if (itr->second > count)
        itr->second -= count;
    else
        cards.erase(itr);
    for (uint8 i = 0; i < count; i++)
        _toSend.push(new Message(SMSG_DISCARD, Game::encodeCard(c)));
}

void Player::addCard(Card c)
{
    cards[c]++;
    _toSend.push(new Message(SMSG_DRAW, Game::encodeCard(c)));
}

void Player::joinLobby(Lobby* lobby, bool create)
{
    _lobby = lobby;
    _toSend.push(new Message(create ? SMSG_CREATE_LOBBY : SMSG_JOIN_LOBBY, lobby->name));
    if (!create)
        lobby->onPlayerJoined(this);
}

void Player::leaveLobby(Lobby* lobby, bool removing)
{
    _lobby = nullptr;
    _toSend.push(new Message(SMSG_LEAVE_LOBBY));
    if (!removing)
        lobby->onPlayerLeft(this);
}

void Player::sendMessage(std::string msg)
{
    _toSend.push(new Message(SMSG_MESSAGE,msg));
}

Lobby* Player::getLobby()
{
    return _lobby;
}

Server* Player::getServer()
{
    return _server;
}

void Player::disconnect()
{
    _state = DISCONNECTED;
}

void Player::turn()
{
    _toSend.push(new Message(SMSG_TURN));
}
