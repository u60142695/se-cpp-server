#ifndef WORLD_DOWNLOAD_CHANNEL_H
#define WORLD_DOWNLOAD_CHANNEL_H

#include "IP2PChannel.h"

class WorldDownloadChannel : public IP2PChannel
{
public:
    WorldDownloadChannel();
protected:
    void HandlePacket(P2PPacket* pkt);
};

#endif
