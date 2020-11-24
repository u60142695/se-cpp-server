#ifndef CONTROL_CHANNEL_H
#define CONTROL_CHANNEL_H

#include "IP2PChannel.h"

class ControlChannel : public IP2PChannel
{
public:
    ControlChannel();
protected:
    void HandlePacket(P2PPacket* pkt);
};

#endif
