#include "P2PPacket.h"

P2PPacket::P2PPacket()
{

}

P2PPacket::P2PPacket(uint8_t messageId, bool bReliable)
{
    reliable = bReliable;

    bitStream.ResetWrite();
    bitStream.WriteUInt8(206); // first byte is always 206.
    bitStream.WriteUInt8(bReliable ? 0 : 1); // whether it is a unreliable therefore there is a checksum.
    bitStream.WriteUInt32(0); // placeholder for checksum
    bitStream.WriteUInt8(0); // unk
    bitStream.WriteUInt8(1); // packet count

    // header
    bitStream.WriteUInt8(messageId);
    bitStream.WriteUInt8(0); // receiver index
}
