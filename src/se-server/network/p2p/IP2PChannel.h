#ifndef P2P_RECEIVE_CHANNEL_H
#define P2P_RECEIVE_CHANNEL_H

#include <stdint.h>
#include <thread>

#include <steam/steam_gameserver.h>

#include "P2PPacket.h"

#define MAX_MESSAGES_PER_TICK           5
#define RECEIVE_BUFFER_SIZE             1024 * 16

enum class eP2PChannelId
{
    Control = 0,
    WorldDownload,
    GameEvent,
    VoiceChat,
    ProfilerDownload
};

class IP2PChannel
{
public:
    IP2PChannel(eP2PChannelId channelId);
    ~IP2PChannel();

    void            Start();
    void            Stop();

    void            SendPacket(uint64_t steamId, P2PPacket& pkt);
protected:
    virtual void    HandlePacket(P2PPacket* pkt) = 0;
private:
    void            ReceiveThread();
private:
    eP2PChannelId   m_eChannelId;
    std::thread*    m_pReceiveThread = nullptr;
    bool            m_bReceiveThreadContinue = false;
    uint8_t         m_aReceiveBuffer[RECEIVE_BUFFER_SIZE] = { 0 };
};

#endif
