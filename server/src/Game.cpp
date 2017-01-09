#include "Game.h"

#include "Player.h"
#include <algorithm>
#include "Message.h"
#include "Server.h"
#include <sstream>

Game::Game(std::vector<Player*> players)
    : _running(true)
    , _players(players)
    , thisPlayerDraws(1)
    , nextPlayerDraws(1)
{
    std::random_shuffle(_players.begin(), _players.end());
    playing = 0;
    generatePack();
    for (auto p : _players)
    {
        p->startGame(_players[playing]->name, this);
        for (uint8 i = 0; i < 3; i++)
        {
            p->addCard(pack.front());
            pack.pop_front();
        }
    }
    for (uint8 i = 0; i < _players.size() - 1; i++)
        pack.push_back(BOMB);
    std::random_shuffle(pack.begin(), pack.end());

    run();
}

Game::~Game()
{
    Player* winner = _players.size() == 1 ? _players[0] : nullptr;
    for (auto p : _players)
        if (!p->dead)
        {
            winner = p;
            break;
        }

    for (auto p : _players)
        p->endGame(winner);
    _running = false;
    _acceptThread->join();
}

bool Game::isRunning()
{
    return _running;
}

void Game::RemovePlayer(Player* player)
{
    Guard guard(_lock);

    bool newPlayer = _players[playing] == player;
    auto itr = std::find(_players.begin(), _players.end(), player);
    if (itr != _players.end())
        _players.erase(itr);

    if (playing == _players.size())
    {
        playing = 0;
        newPlayer = true;
    }

    if (newPlayer)
    {
        checkState();
        if (_players.size() > 1)
        {
            announce(nullptr, "Next player is \"" + _players[playing]->name + "\".");
            _players[playing]->turn();
        }
    }
}

LFQueue<PlayerMessage>& Game::GetMessages()
{
    return _messages;
}

void Game::run()
{
    _acceptThread = new std::thread([&]
    {
        while (_running)
        {
            processMessages();
            checkState();
        }
        //send announce winner
    });

};

void Game::processMessages()
{
    while (auto* msg = _messages.pop())
    {
        if (msg->player != _players[playing])
        {
            switch (msg->message->opcode)
            {
                case CMSG_PLAY:
                case CMSG_DRAW:
                    msg->player->sendMessage("Not your turn!");
                    delete msg;
                    continue;
                default:
                    break;
            }
        }

        switch (msg->message->opcode)
        {
            case CMSG_PLAY:
                playCard(msg);
                break;
            case CMSG_DRAW:
            {
                drawCard(msg);
                checkState();
                if (thisPlayerDraws == 0)
                    endTurn(msg->player);
                else if (_running)
                {
                    std::stringstream ss;
                    ss << "You have to draw " << (int)thisPlayerDraws << " more cards.";
                    msg->player->sendMessage(ss.str());
                    announce(msg->player, "draw cards and continue playing.");
                }
                break;
            }
            case CMSG_CARDS:
                countCards(msg->player);
                break;
            case CMSG_FF:
            {
                if (!msg->player->dead)
                    announce(msg->player, "forfeit match, so he went kaboom!");
                RemovePlayer(msg->player);
                msg->player->ff();
                break;
            }
            case CMSG_QUIT:
                if (!msg->player->dead)
                    announce(msg->player, "forfeit match, so he went kaboom!");
                msg->player->getServer()->removePlayer(msg->player);
                break;
        }
        delete msg;
    }
}

void Game::checkState()
{
    uint8 activePlayers = 0;
    for (auto pl : _players)
        activePlayers += pl->isActive();
    _running = activePlayers > 1;
}

void Game::playCard(PlayerMessage * msg)
{
    std::vector<std::string> parts;
    std::string s(msg->message->data);
    splitMessages(parts, s, TOKEN);
    Card c = parseCard(parts[0] == "TRIPLE" ? parts[1] : parts[0]);
    if (!msg->player->haveCard(c, parts[0] == "TRIPLE" ? 3 : 1))
    {
        msg->player->sendMessage("You dont have that card! Cheater!");
        return;
    }
    switch (c)
    {
        case SHUFFLE:
            msg->player->removeCard(c);
            std::random_shuffle(pack.begin(), pack.end());
            announce(msg->player, "shuffled deck");
            break;
        case PEAK:
            msg->player->removeCard(c);
            msg->player->peak(pack);
            announce(msg->player, "peaked top three cards in deck");
            break;
        case KICK:
            msg->player->removeCard(c);
            kick(msg);
            break;
        case RUN:
            msg->player->removeCard(c);
            kick(msg, true);
            break;
        case STEAL:
            if (parts.size() <= 1)
                msg->player->sendMessage("Target not found!");
            else if (steal(msg, parts[1]))
                msg->player->removeCard(c);
            break;
        default:
            if (parts[0] == "TRIPLE")
            {
                if (parts.size() != 3)
                    msg->player->sendMessage("Invalid play tripple!");
                else
                {
                    if (steal(msg, parts[2], true))
                        msg->player->removeCard(c,3);
                }
                break;
            }

            msg->player->sendMessage("Unknown card! Cheater!");
            break;
    }
}

void Game::drawCard(PlayerMessage * msg)
{
    Card c = pack.front();
    pack.pop_front();
    thisPlayerDraws--;
    if (c == BOMB)
    {
        msg->player->dead = true;
        announce(msg->player, "has died!");
        msg->player->sendMessage("You have been blown to pieces!");
    }
    else
        msg->player->addCard(c);
}

Card Game::parseCard(std::string data)
{
    #ifndef ef
#define ef else if
        if(data == "SHUFFLE") return SHUFFLE;
        ef(data == "PEAK") return PEAK;
        ef(data == "KICK") return KICK;
        ef(data == "RUN") return RUN;
        ef(data == "STEAL") return STEAL;
        ef(data == "TRIPLE") return TRIPLE;
        ef(data == "A") return A;
        ef(data == "B") return B;
        ef(data == "C") return C;
        ef(data == "D") return D;
        ef(data == "E") return E;
        ef(data == "F") return F;
#undef ef
#endif
        return UNKNOWN;
}

std::string Game::encodeCard(Card c)
{
    switch (c)
    {
        case SHUFFLE: return "SHUFFLE";
        case PEAK: return "PEAK";
        case KICK: return "KICK";
        case RUN: return "RUN";
        case STEAL: return "STEAL";
        case TRIPLE: return "TRIPLE";
        case A: return "A";
        case B: return "B";
        case C: return "C";
        case D: return "D";
        case E: return "E";
        case F: return "F";
    }
    return "unknown";
}

void Game::announce(Player* player, std::string text, bool excludePlayer)
{
    for (auto p : _players)
        if (p != player || !excludePlayer)
            p->sendMessage((player ? "Player \"" + player->name + "\" " : "") + text);
}

void Game::generatePack()
{
    for (uint8 i = 0; i < BOMB; i++)
    {
        for (uint8 j = 0; j < (i < PEAK ? 4 : 6); j++)
            pack.push_back((Card)i);
    }
    std::random_shuffle(pack.begin(), pack.end());
}

void Game::kick(PlayerMessage * msg, bool run)
{
    msg->player->play(run ? "RUN" : "KICK");
    announce(msg->player,run ? "ran away from bom" : "kicked bomb to next player");
    if (!run)
        nextPlayerDraws++;
    thisPlayerDraws--;
    if (thisPlayerDraws == 0)
        endTurn(msg->player);
    else
    {
        std::stringstream ss;
        ss << "You have to draw " << (int)thisPlayerDraws << " more cards.";
        msg->player->sendMessage(ss.str());
        announce(msg->player, "escaped one bomb and continue playing.");
    }
}

bool Game::steal(PlayerMessage * msg, std::string targetName, bool discard /*= false*/)
{
    Player* target = nullptr;
    for (auto p : _players)
        if (p->name == targetName)
        {
            target = p;
            break;
        }

    if (!target)
        msg->player->sendMessage("Target not found!");
    else if (target == msg->player)
        msg->player->sendMessage("Cant play to your self!");
    else if (target->countCards() == 0)
        msg->player->sendMessage("Target doesnt have any cards!");
    else
    {
        Card c = target->removeRandomCard();
        if (!discard)
            msg->player->addCard(c);
        announce(msg->player, (discard ? "discarded card " + encodeCard(c) : "stole card") +" of player " + target->name );
        return true;
    }
    return false;
}

void Game::countCards(Player* player)
{
    std::stringstream ss;
    ss << (int)pack.size();
    for (auto p : _players)
    {
        if (p == player) continue;
        ss << " " << p->name << " ";
        ss << (int)(p->dead ? -1 : p->countCards());
    }
    
    player->send(SMSG_CARDS, ss.str());
}

void Game::endTurn(Player * player)
{
    bool found = false;
    thisPlayerDraws = nextPlayerDraws;
    nextPlayerDraws = 1;
    for (uint8 i = playing + 1; i < _players.size(); i++)
        if (!_players[i]->dead)
        {
            found = true;
            playing = i;
            break;
        }
    if (found)
    {
        announce(player, "finnished his turn, next player is \"" + _players[playing]->name + "\".", false);
        _players[playing]->turn();
        return;
    }
    for (uint8 i = 0; i < playing; i++)
        if (!_players[i]->dead)
        {
            playing = i;
            break;
        }
    announce(player, "finnished his turn, next player is \"" + _players[playing]->name + "\".", false);
    _players[playing]->turn();
}

