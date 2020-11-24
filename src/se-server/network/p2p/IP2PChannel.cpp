#include "IP2PChannel.h"

#include "../../util/LogUtility.h"
#include "P2PPacket.h"
#include "../../util/Crc32.h"

IP2PChannel::IP2PChannel(int32_t lChannelId)
: m_lChannelId(lChannelId)
{
    memset(m_aReceiveBuffer, 0, RECEIVE_BUFFER_SIZE);
}

IP2PChannel::~IP2PChannel()
{
    Stop();
}

void IP2PChannel::Start()
{
    m_pReceiveThread = new std::thread([this]()
    {
        ReceiveThread();
    });
}

void IP2PChannel::Stop()
{
    if (m_pReceiveThread)
    {
        m_bReceiveThreadContinue = false;

        m_pReceiveThread->join();

        delete m_pReceiveThread;
        m_pReceiveThread = nullptr;
    }
}

void IP2PChannel::ReceiveThread()
{
    m_bReceiveThreadContinue = true;

    while (m_bReceiveThreadContinue)
    {
        uint32_t pktLength = 0;
        uint32_t pktLength2 = 0;
        if (SteamGameServerNetworking()->IsP2PPacketAvailable(&pktLength, m_lChannelId))
        {
            CSteamID remoteSteamId;
            if (SteamGameServerNetworking()->ReadP2PPacket(m_aReceiveBuffer, pktLength, &pktLength2, &remoteSteamId, m_lChannelId))
            {
                if (pktLength2 >= 8)
                {
                    sLog->Info("[P2P] Packet available on channel %d from %llu size = %d", m_lChannelId, remoteSteamId.ConvertToUint64(), pktLength2);

                    if (m_aReceiveBuffer[7] > 1)
                    {
                        sLog->Error("[P2P] Received so-called large packet");
                    }
                    else
                    {
                        P2PPacket pkt;

                        pkt.messageId = m_aReceiveBuffer[8];
                        pkt.remoteSteamID = remoteSteamId.ConvertToUint64();
                        pkt.bitStream.ResetRead(m_aReceiveBuffer + 8, (pktLength2 - 8) * 8);
                        pkt.buffer.Append(m_aReceiveBuffer + 8, pktLength2 - 8);

                        this->HandlePacket(&pkt);
                    }
                }
                else
                {
                    sLog->Error("[P2P] Found a too short packet (size %d) from %llu", pktLength2, remoteSteamId);
                }
            }
        }

        // process p2p packet through steam
        /*int lReceivedMessages = SteamGameServerNetworkingMessages()->ReceiveMessagesOnChannel(m_lChannelId, m_pReceiveMessagesBuffer, MAX_MESSAGES_PER_TICK);

        if (lReceivedMessages > 0)
        {
            sLog->Info("Received %d P2P messages on channel %d", lReceivedMessages, m_lChannelId);
        }*/

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
