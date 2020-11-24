#include "VoiceChatChannel.h"

VoiceChatChannel::VoiceChatChannel() : IP2PChannel(kP2PChannelId_VoiceChat)
{

}

void VoiceChatChannel::HandlePacket(P2PPacket* pkt)
{
}
