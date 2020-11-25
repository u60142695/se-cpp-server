#include "PlayerManager.h"

#include "../network/SteamInterface.h"
#include "../util/LogUtility.h"

PlayerManager* PlayerManager::ms_instance = 0;

PlayerManager* PlayerManager::GetInstance()
{
    if (!ms_instance)
    {
        ms_instance = new PlayerManager();
    }

    return ms_instance;
}

void PlayerManager::BeginAuthenticate(uint64_t steamId, const std::string& userName, bool bExperimentalMode, bool bIsProfiling, std::shared_ptr<char> token, uint32_t tokenLength)
{
    if (m_players.count(steamId) > 0)
    {
        // Player is already online.
    }
    else if (m_playersAuthenticating.count(steamId) > 0)
    {
        // Player is already awaiting authentication response from Steam.
        // Return "already online".
    }
    else
    {
        // Send Ticket Validation Request to Steam.
        auto result = SteamGameServer()->BeginAuthSession(token.get(), tokenLength, CSteamID(steamId));
        if (result != k_EBeginAuthSessionResultOK)
        {
            sLog->Error("Failed to authenticate %llu with Steam - reason %d", steamId, result);
            return;
        }

        Player* pPlayer = new Player(steamId);
        pPlayer->SetName(userName);
        pPlayer->SetExperimentalMode(bExperimentalMode);
        pPlayer->SetIsProfiling(bIsProfiling);

        m_mtxPlayersAuthenticating.lock();
        {
            m_playersAuthenticating.insert(std::make_pair(steamId, pPlayer));
        }
        m_mtxPlayersAuthenticating.unlock();
    }
}

void PlayerManager::EndAuthenticate(uint64_t steamId, int steamResult)
{
    if (m_playersAuthenticating.count(steamId) == 0)
        return;

    Player* pPlayer = m_playersAuthenticating.at(steamId);

    m_mtxPlayersAuthenticating.lock();
    {
        m_playersAuthenticating.erase(steamId);
    }
    m_mtxPlayersAuthenticating.unlock();

    if (steamResult != k_EAuthSessionResponseOK)
    {
        sLog->Info("Steam Authentication failed for %llu", steamId);

        SteamInterface::GetInstance()->GetGameEventChannel()->SendJoinResult(steamId, eJoinResult::TicketInvalid);

        return;
    }

    // Check bans and other local database-related stuff to this account.

    // Authentication Success.
    sLog->Info("Player authenticated: %s - %llu", pPlayer->GetName().c_str(), pPlayer->GetSteamId());
    m_mtxPlayers.lock();
    {
        m_players.insert(std::make_pair(pPlayer->GetSteamId(), pPlayer));
    }
    m_mtxPlayers.unlock();

    SteamInterface::GetInstance()->GetGameEventChannel()->SendJoinResult(steamId, eJoinResult::ServerFull);
}
