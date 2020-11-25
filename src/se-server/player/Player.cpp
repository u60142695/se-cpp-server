#include "Player.h"

Player::Player(uint64_t steamId)
: m_llSteamId(steamId)
{

}

uint64_t Player::GetSteamId()
{
    return m_llSteamId;
}

std::string Player::GetName()
{
    return m_strName;
}

void Player::SetName(const std::string& strName)
{
    m_strName = strName;
}

void Player::SetExperimentalMode(bool bExperimentalMode)
{
    m_bExperimentalMode = bExperimentalMode;
}

void Player::SetIsProfiling(bool bIsProfiling)
{
    m_bIsProfiling = bIsProfiling;
}
