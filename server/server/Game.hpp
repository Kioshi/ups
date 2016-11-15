#pragma once
#include <vector>
#include <mutex>
#include <atomic>

class Player;
class PlayerPacket;

class Game
{
public:
    Game(std::vector<Player*> players);

    bool isRunning();
    
    void RemovePlayer(Player* player);

    std::vector<PlayerPacket*>& GetPackets();
private:

    void run();

    void processPackets();
    void checkState();

private:
    std::vector<Player*> _players;
    std::vector<PlayerPacket*> _packets;
    std::atomic<bool> _running;
    std::mutex _lock;
};