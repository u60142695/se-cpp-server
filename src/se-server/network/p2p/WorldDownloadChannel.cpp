#include "WorldDownloadChannel.h"

WorldDownloadChannel::WorldDownloadChannel() : IP2PChannel(eP2PChannelId::WorldDownload)
{

}

void WorldDownloadChannel::HandlePacket(P2PPacket* pkt)
{
}
