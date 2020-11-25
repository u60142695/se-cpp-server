#include "ControlChannel.h"

ControlChannel::ControlChannel() : IP2PChannel(eP2PChannelId::Control)
{

}

void ControlChannel::HandlePacket(P2PPacket* pkt)
{
}
