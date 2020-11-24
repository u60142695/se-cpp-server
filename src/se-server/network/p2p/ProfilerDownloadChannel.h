#ifndef PROFILER_DOWNLOAD_CHANNEL_H
#define PROFILER_DOWNLOAD_CHANNEL_H

#include "IP2PChannel.h"

class ProfilerDownloadChannel : public IP2PChannel
{
public:
    ProfilerDownloadChannel();
protected:
    void HandlePacket(P2PPacket* pkt);
};

#endif
