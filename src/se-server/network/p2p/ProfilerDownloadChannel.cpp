#include "ProfilerDownloadChannel.h"

ProfilerDownloadChannel::ProfilerDownloadChannel() : IP2PChannel(kP2PChannelId_ProfilerDownload)
{

}

void ProfilerDownloadChannel::HandlePacket(P2PPacket* pkt)
{
}
