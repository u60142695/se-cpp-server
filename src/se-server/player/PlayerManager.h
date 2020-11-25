#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include "Player.h"

#include <map>
#include <mutex>

class PlayerManager
{
public:
    static PlayerManager* GetInstance();
public:
    void BeginAuthenticate(uint64_t steamId, const std::string& userName, bool bExperimentalMode, bool bIsProfiling,
        std::shared_ptr<char> token, uint32_t tokenLength);
    void EndAuthenticate(uint64_t steamId, int steamResult);
private:
    std::map<uint64_t, Player*> m_players;
    std::map<uint64_t, Player*> m_playersAuthenticating;
    std::mutex m_mtxPlayers;
    std::mutex m_mtxPlayersAuthenticating;
private:
    static PlayerManager* ms_instance;
};

#endif
