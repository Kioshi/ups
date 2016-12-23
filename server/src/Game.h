#pragma once
#include <vector>
#include <mutex>
#include <atomic>

#include "utils.h"
#include "LockFree.h"

class Player;
class PlayerMessage;

class Game
{
public:
    Game(std::vector<Player*> players);

    bool isRunning();
    
    void RemovePlayer(Player* player);

    LFQueue<PlayerMessage>& GetMessages();
private:

    void run();

    void processMessages();
    void checkState();

private:
    std::vector<Player*> _players;
    LFQueue<PlayerMessage> _messages;
    std::atomic<bool> _running;
    std::mutex _lock;
};