#include "ProfilerDownloadChannel.h"

ProfilerDownloadChannel::ProfilerDownloadChannel() : IP2PChannel(eP2PChannelId::ProfilerDownload)
{

}

void ProfilerDownloadChannel::HandlePacket(P2PPacket* pkt)
{
}
