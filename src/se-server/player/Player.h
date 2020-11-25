#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <string>

class Player
{
public:
    Player(uint64_t steamId);

    uint64_t    GetSteamId();
    std::string GetName();

    void        SetName(const std::string& strName);
    void        SetExperimentalMode(bool bExperimentalMode);
    void        SetIsProfiling(bool bIsProfiling);
private:
    uint64_t    m_llSteamId = 0;
    std::string m_strName;
    bool        m_bExperimentalMode = false;
    bool        m_bIsProfiling = false;
};

#endif
