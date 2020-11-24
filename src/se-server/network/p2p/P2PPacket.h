#ifndef P2P_PACKET_H
#define P2P_PACKET_H

#include "../VRageBitStream.h"
#include "../Buffer.h"

class P2PPacket
{
public:
    P2PPacket();

    uint64_t remoteSteamID = 0;
    VRageBitStream bitStream;
    Buffer buffer;
    uint64_t receivedTime = 0;
    uint8_t messageId = 0;
};

#endif
