#include "P2PReceiveChannel.h"

#include "../util/LogUtility.h"

#include <steam/steam_gameserver.h>

P2PReceiveChannel::P2PReceiveChannel(int32_t lChannelId)
: m_lChannelId(lChannelId)
{
    
}

P2PReceiveChannel::~P2PReceiveChannel()
{
    Stop();
}

void P2PReceiveChannel::Start()
{
    m_pReceiveThread = new std::thread([this]()
    {
        ReceiveThread();
    });
}

void P2PReceiveChannel::Stop()
{
    if (m_pReceiveThread)
    {
        m_bReceiveThreadContinue = false;

        m_pReceiveThread->join();

        delete m_pReceiveThread;
        m_pReceiveThread = nullptr;
    }
}

void P2PReceiveChannel::ReceiveThread()
{
    m_bReceiveThreadContinue = true;

    while (m_bReceiveThreadContinue)
    {
        uint32_t recvSize = 0;
        if (SteamGameServerNetworking()->IsP2PPacketAvailable(&recvSize, m_lChannelId))
        {
            sLog->Info("P2P Packet (Game Server) available");
        }

        // process p2p packet through steam
        /*int lReceivedMessages = SteamGameServerNetworkingMessages()->ReceiveMessagesOnChannel(m_lChannelId, m_pReceiveMessagesBuffer, MAX_MESSAGES_PER_TICK);

        if (lReceivedMessages > 0)
        {
            sLog->Info("Received %d P2P messages on channel %d", lReceivedMessages, m_lChannelId);
        }*/

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
