#ifndef GAME_EVENT_CHANNEL_H
#define GAME_EVENT_CHANNEL_H

#include "IP2PChannel.h"

class GameEventChannel : public IP2PChannel
{
public:
    GameEventChannel();
protected:
    void HandlePacket(P2PPacket* pkt);
};

#endif
