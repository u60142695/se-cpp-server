#ifndef P2P_PACKET_H
#define P2P_PACKET_H

#include "../VRageBitStream.h"
#include "../Buffer.h"

#define SE_MESSAGE_ID_FLUSH                     2
#define SE_MESSAGE_ID_RPC                       3
#define SE_MESSAGE_ID_REPLICATION_CREATE        4
#define SE_MESSAGE_ID_REPLICATION_DESTROY       5
#define SE_MESSAGE_ID_SERVER_DATA               6
#define SE_MESSAGE_ID_SERVER_STATE_SYNC         7
#define SE_MESSAGE_ID_CLIENT_READY              8
#define SE_MESSAGE_ID_CLIENT_UPDATE             9
#define SE_MESSAGE_ID_REPLICATION_READY         10
#define SE_MESSAGE_ID_REPLICATION_STREAM_BEGIN  11
#define SE_MESSAGE_ID_JOIN_RESULT               12
#define SE_MESSAGE_ID_WORLD_DATA                13
#define SE_MESSAGE_ID_CLIENT_CONNECTED          14
#define SE_MESSAGE_ID_CLIENT_ACKS               17
#define SE_MESSAGE_ID_REPLICATION_ISLAND_DONE   18
#define SE_MESSAGE_ID_REPLICATION_REQUEST       19
#define SE_MESSAGE_ID_WORLD                     20
#define SE_MESSAGE_ID_PLAYER_DATA               21

class P2PPacket
{
public:
    P2PPacket();
    P2PPacket(uint8_t messageId, bool bReliable = true);

    uint64_t remoteSteamID = 0;
    VRageBitStream bitStream;
    uint64_t receivedTime = 0;
    uint8_t messageId = 0;
    uint8_t receiverIndex = 0;
    bool reliable = true;
};

#endif
