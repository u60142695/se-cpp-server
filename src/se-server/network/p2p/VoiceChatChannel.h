#ifndef VOICE_CHAT_CHANNEL_H
#define VOICE_CHAT_CHANNEL_H

#include "IP2PChannel.h"

class VoiceChatChannel : public IP2PChannel
{
public:
    VoiceChatChannel();
protected:
    void HandlePacket(P2PPacket* pkt);
};

#endif
