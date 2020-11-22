#ifndef P2P_RECEIVE_CHANNEL_H
#define P2P_RECEIVE_CHANNEL_H

#include <stdint.h>
#include <thread>

#include <steam/steam_gameserver.h>

#define MAX_MESSAGES_PER_TICK       5

class P2PReceiveChannel
{
public:
    P2PReceiveChannel(int32_t lChannelId);
    ~P2PReceiveChannel();

    void            Start();
    void            Stop();
private:
    void            ReceiveThread();
private:
    int32_t         m_lChannelId = 0;
    std::thread*    m_pReceiveThread = nullptr;
    bool            m_bReceiveThreadContinue = false;
    SteamNetworkingMessage_t* m_pReceiveMessagesBuffer[MAX_MESSAGES_PER_TICK] = { 0 };
};

#endif
