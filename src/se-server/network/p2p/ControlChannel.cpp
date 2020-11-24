#include "ControlChannel.h"

ControlChannel::ControlChannel() : IP2PChannel(kP2PChannelId_Control)
{

}

void ControlChannel::HandlePacket(P2PPacket* pkt)
{
}
