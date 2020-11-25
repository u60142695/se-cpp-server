#include "GameEventChannel.h"

#include "../../util/LogUtility.h"
#include "../../util/NetworkUtility.h"
#include "../../util/CPP20Util.h"

#include "../../player/PlayerManager.h"

GameEventChannel::GameEventChannel() : IP2PChannel(eP2PChannelId::GameEvent)
{

}

void GameEventChannel::SendJoinResult(uint64_t steamId, eJoinResult eResult)
{
    P2PPacket pkt(SE_MESSAGE_ID_JOIN_RESULT);

    pkt.bitStream.WriteUInt64(0); // adminId
    // write enum
    pkt.bitStream.WriteUInt64((uint64_t)eResult, (int)((std::log((int)eJoinResult::Enum_Max - 1) / std::log(2))) + 1);
    pkt.bitStream.WriteBoolean(false); // experimental mode

    SendPacket(steamId, pkt);
    // ulong:admin
    // joinresult:joinresult
    // bool:server_experimental
}

void GameEventChannel::HandlePacket(P2PPacket* pkt)
{
    if (pkt->messageId == SE_MESSAGE_ID_CLIENT_CONNECTED) // CLIENT_CONNECTED. This is the initial packet sent by the game client.
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
        sLog->Info("Received unknown Game Event Packet Opcode=%d RecvIndex=%d Size=%d", pkt->messageId, pkt->receiverIndex, pkt->bitStream.GetByteLength());
        NetworkUtility::HexDump(pkt->bitStream.GetBuffer(), pkt->bitStream.GetByteLength());
    }    
}
