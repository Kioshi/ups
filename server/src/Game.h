#pragma once
#include <deque>
#include <mutex>
#include <atomic>
#include <thread>

#include "utils.h"
#include "LockFree.h"

class Player;
class PlayerMessage;

enum Card
{
    SHUFFLE = 0,
    KICK,
    RUN,
    STEAL,
    PEAK,
    A,
    B,
    C,
    D,
    E,
    F,
    BOMB,
    TRIPLE,
    UNKNOWN
};

class Game
{
public:
    Game(std::vector<Player*> players);
    ~Game();

    bool isRunning();
    
    void RemovePlayer(Player* player);

    LFQueue<PlayerMessage>& GetMessages();
    static std::string encodeCard(Card c);
private:

    void run();

    void processMessages();
    void checkState();
    void playCard(PlayerMessage * msg);
    void drawCard(PlayerMessage * msg);
    Card parseCard(std::string data);
    void announce(Player* player, std::string text, bool excludePlayer = true);
    void generatePack();
    void kick(PlayerMessage * msg, bool run = false);
    bool steal(PlayerMessage * msg, std::string targetName, bool discard = false);
    void countCards(Player* player);
    void endTurn(Player* player);
private:
    uint8 nextPlayerDraws;
    uint8 thisPlayerDraws;
    std::vector<Player*> _players;
    LFQueue<PlayerMessage> _messages;
    std::atomic<bool> _running;
    std::mutex _lock;
    uint8 playing;
    std::deque<Card> pack;
    std::thread* _acceptThread;
};