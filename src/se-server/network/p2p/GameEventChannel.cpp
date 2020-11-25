#include "GameEventChannel.h"

#include "../../util/LogUtility.h"
#include "../../util/NetworkUtility.h"
#include "../../util/CPP20Util.h"

#include "../../player/PlayerManager.h"

GameEventChannel::GameEventChannel() : IP2PChannel(kP2PChannelId_GameEvent)
{

}

void GameEventChannel::HandlePacket(P2PPacket* pkt)
{
    uint8_t opcode = pkt->bitStream.ReadUInt8();
    uint8_t receiverIndex = pkt->bitStream.ReadUInt8();

    if (opcode == 14) // CLIENT_CONNECTED. This is the initial packet sent by the game client.
    {
        bool bExperimentalMode = pkt->bitStream.ReadBoolean();
        bool bIsAdmin = pkt->bitStream.ReadBoolean();
        bool bIsProfiling = pkt->bitStream.ReadBoolean();
        bool bJoin = pkt->bitStream.ReadBoolean();

        std::string userName = pkt->bitStream.ReadString();
        uint64_t steamId = pkt->bitStream.ReadUInt64();

        if (steamId != pkt->remoteSteamID)
        {
            sLog->Error("Received authentication for %llu from different steam remote %llu - hack?", steamId, pkt->remoteSteamID);
            // disconnect user?
            return;
        }

        // Read Token.
        pkt->bitStream.ReadBoolean(); // array is nullable
        uint32_t tokenSize = pkt->bitStream.ReadUInt32Variant();

        sLog->Info("Token Size: %u", tokenSize);

        std::shared_ptr<char> tokenData = CPP20Util::MakeArray<char>(tokenSize);
        for (int i = 0; i < tokenSize; i++)
        {
            tokenData.get()[i] = pkt->bitStream.ReadUInt8();
        }

        sLog->Info("Client Connected: %llu (%s)", steamId, userName.c_str());
        PlayerManager::GetInstance()->BeginAuthenticate(steamId, userName, bExperimentalMode, bIsProfiling, tokenData, tokenSize);
    }
    else
    {
        sLog->Info("Received Game Event Packet Opcode=%d RecvIndex=%d Size=%d", opcode, receiverIndex, pkt->bitStream.GetByteLength());
        NetworkUtility::HexDump(pkt->bitStream.GetBuffer(), pkt->bitStream.GetByteLength());
    }    
}
