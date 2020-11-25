#ifndef VRAGE_BIT_STREAM_H
#define VRAGE_BIT_STREAM_H

#include <vector>
#include <string>

class VRageBitStream
{
public:
    VRageBitStream(int defaultByteSize = 1536);
    ~VRageBitStream();

    int         GetBitPosition();
    int         GetBitLength();
    int         GetBytePosition();
    int         GetByteLength();
    uint64_t*   GetBuffer();

    void        SetBytePosition(uint32_t position);

    bool        OwnsBuffer();

    void        FreeNotOwnedBuffer();
    void        ReleaseInternalBuffer();

    void        ResetRead();
    void        ResetRead(uint8_t* data, uint32_t bitLength);
    void        ResetRead(uint8_t* data, uint32_t offset, uint32_t bitLength, bool copy = true);
    void        ResetRead(uint8_t* data, uint32_t bitLength, bool copy);

    void        ResetWrite();
    void        ResetWrite(VRageBitStream& stream);

    bool        ReadBoolean();
    void        WriteBoolean(bool value);

    void        ReadMemory(void* ptr, int bitSize);
    void        WriteMemory(void* ptr, int bitSize);

    std::string ReadString();
    void        WriteString(const std::string& str);

    int8_t      ReadInt8(int bitCount = 8);
    uint8_t     ReadUInt8(int bitCount = 8);
    int16_t     ReadInt16(int bitCount = 16);
    uint16_t    ReadUInt16(int bitCount = 16);
    int32_t     ReadInt32(int bitCount = 32);
    uint32_t    ReadUInt32(int bitCount = 32);
    int64_t     ReadInt64(int bitCount = 64);
    uint64_t    ReadUInt64(int bitCount = 64);

    void        WriteInt8(int8_t value, int bitCount = 8);
    void        WriteUInt8(uint8_t value, int bitCount = 8);
    void        WriteInt16(int16_t value, int bitCount = 16);
    void        WriteUInt16(uint16_t value, int bitCount = 16);
    void        WriteInt32(int32_t value, int bitCount = 32);
    void        WriteUInt32(uint32_t value, int bitCount = 32);
    void        WriteInt64(int64_t value, int bitCount = 64);
    void        WriteUInt64(uint64_t value, int bitCount = 64);

    int32_t     ReadInt32Variant();
    int64_t     ReadInt64Variant();
    uint32_t    ReadUInt32Variant();
    uint64_t    ReadUInt64Variant();

    void        WriteInt32Variant(int32_t value);
    void        WriteInt64Variant(int64_t value);
    void        WriteUInt32Variant(uint32_t value);
    void        WriteUInt64Variant(uint64_t value);
private:
    int         getDivisionCeil(int num, int div);
    void        ensureSize(int bitCount);
    void        resize(int bitSize);
    uint64_t    readInternal(int bitSize);
    void        writeInternal(uint64_t value, int bitSize);
private:
    static int  zag(uint32_t ziggedValue);
    static int  zag(uint64_t ziggedValue);
private:
    static uint64_t Int64Msb;
    static uint32_t Int32Msb;
private:
    int         m_bitPosition = 0;

    uint64_t*   m_buffer = nullptr;
    int         m_bitLength = 0;

    uint64_t*   m_ownedBuffer = nullptr;
    int         m_ownedBufferBitLength = 0;

    bool        m_writing = false;

    int         m_defaultByteSize = 0;
    
};

#endif
