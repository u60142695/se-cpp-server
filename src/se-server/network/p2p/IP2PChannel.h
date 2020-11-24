#ifndef P2P_RECEIVE_CHANNEL_H
#define P2P_RECEIVE_CHANNEL_H

#include <stdint.h>
#include <thread>

#include <steam/steam_gameserver.h>

#include "P2PPacket.h"

#define MAX_MESSAGES_PER_TICK           5
#define RECEIVE_BUFFER_SIZE             1024 * 16

#define kP2PChannelId_Control           0
#define kP2PChannelId_WorldDownload     1
#define kP2PChannelId_GameEvent         2
#define kP2PChannelId_VoiceChat         3
#define kP2PChannelId_ProfilerDownload  4

class IP2PChannel
{
public:
    IP2PChannel(int32_t lChannelId);
    ~IP2PChannel();

    void            Start();
    void            Stop();
protected:
    virtual void    HandlePacket(P2PPacket* pkt) = 0;
private:
    void            ReceiveThread();
private:
    int32_t         m_lChannelId = 0;
    std::thread*    m_pReceiveThread = nullptr;
    bool            m_bReceiveThreadContinue = false;
    uint8_t         m_aReceiveBuffer[RECEIVE_BUFFER_SIZE] = { 0 };
};

#endif
