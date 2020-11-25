#include "VoiceChatChannel.h"

VoiceChatChannel::VoiceChatChannel() : IP2PChannel(eP2PChannelId::VoiceChat)
{

}

void VoiceChatChannel::HandlePacket(P2PPacket* pkt)
{
}
