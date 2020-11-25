#include "VRageBitStream.h"

#include <algorithm>

#include "../util/LogUtility.h"

uint64_t VRageBitStream::Int64Msb = ((uint64_t)1) << 63;
uint32_t VRageBitStream::Int32Msb = ((uint32_t)1) << 31;

VRageBitStream::VRageBitStream(int defaultByteSize)
{
    m_defaultByteSize = std::max(16, getDivisionCeil(defaultByteSize, 8) * 8);
}

VRageBitStream::~VRageBitStream()
{
    ReleaseInternalBuffer();
}

int VRageBitStream::GetBitPosition()
{
    return m_bitPosition;
}

int VRageBitStream::GetBitLength()
{
    return m_bitLength;
}

int VRageBitStream::GetBytePosition()
{
    return getDivisionCeil(m_bitPosition, 8);
}

int VRageBitStream::GetByteLength()
{
    return getDivisionCeil(m_bitLength, 8);
}

uint64_t* VRageBitStream::GetBuffer()
{
    return m_buffer;
}

void VRageBitStream::SetBytePosition(uint32_t position)
{
    m_bitPosition = position * 8;
}

bool VRageBitStream::OwnsBuffer()
{
    return m_ownedBuffer == m_buffer;
}

void VRageBitStream::FreeNotOwnedBuffer()
{
    if (!this->OwnsBuffer() && m_buffer != 0)
    {
        free(m_buffer);
        m_buffer = 0;
    }
}

void VRageBitStream::ReleaseInternalBuffer()
{
    if (m_ownedBuffer != 0)
    {
        if (m_buffer == m_ownedBuffer)
        {
            m_buffer = 0;
            m_bitLength = 0;
        }

        free(m_ownedBuffer);
        m_ownedBuffer = 0;
        m_ownedBufferBitLength = 0;
    }
}

void VRageBitStream::ResetRead()
{
    m_bitLength = m_bitPosition;
    m_buffer = m_ownedBuffer;
    m_writing = false;
    m_bitPosition = 0;
}

void VRageBitStream::ResetRead(uint8_t* data, uint32_t bitLength)
{
    ResetRead(data, 0, bitLength);
}

void VRageBitStream::ResetRead(uint8_t* data, uint32_t offset, uint32_t bitLength, bool copy)
{
    ResetRead(data + offset, bitLength, copy);
}

void VRageBitStream::ResetRead(uint8_t* data, uint32_t bitLength, bool copy)
{
    if (copy)
    {
        int byteLen = getDivisionCeil(bitLength, 8);
        int allocByteSize = std::max(byteLen, m_defaultByteSize);

        if (m_ownedBuffer == 0)
        {
            m_ownedBuffer = (uint64_t*)malloc(allocByteSize);
            memset(m_ownedBuffer, 0, allocByteSize);
            m_ownedBufferBitLength = allocByteSize * 8;
        }
        else if (m_ownedBufferBitLength < bitLength)
        {
            free(m_ownedBuffer);
            m_ownedBuffer = (uint64_t*)malloc(allocByteSize);
            memset(m_ownedBuffer, 0, allocByteSize);
            m_ownedBufferBitLength = allocByteSize * 8;
        }

        memcpy(m_ownedBuffer, data, byteLen);

        m_buffer = m_ownedBuffer;
        m_bitLength = bitLength;
        m_bitPosition = 0;
        m_writing = false;
    }
    else
    {
        m_buffer = (uint64_t*)data;
        m_bitLength = bitLength;
        m_bitPosition = 0;
        m_writing = 0;
    }
}

void VRageBitStream::ResetWrite()
{
    if (m_ownedBuffer == 0)
    {
        m_ownedBuffer = (uint64_t*)malloc(m_defaultByteSize);
        memset(m_ownedBuffer, 0, m_defaultByteSize);
        m_ownedBufferBitLength = m_defaultByteSize * 8;
    }
    else
    {
        memset(m_ownedBuffer, 0, getDivisionCeil(m_ownedBufferBitLength, 8));
    }

    m_buffer = m_ownedBuffer;
    m_bitLength = m_ownedBufferBitLength;
    m_bitPosition = 0;
    m_writing = true;
}

void VRageBitStream::ResetWrite(VRageBitStream& stream)
{
    int bitLength = stream.m_writing ? stream.m_bitPosition : stream.GetBitLength();
    if (m_ownedBuffer != 0 && m_ownedBufferBitLength < bitLength)
    {
        free(m_ownedBuffer);
        m_ownedBuffer = 0;
    }
    if (m_ownedBuffer == 0)
    {
        int byteSize = std::max(getDivisionCeil(bitLength, 64) * 8, m_defaultByteSize);
        m_ownedBuffer = (uint64_t*)malloc(byteSize);
        memset(m_ownedBuffer, 0, byteSize);
        m_ownedBufferBitLength = byteSize * 8;
    }
    m_buffer = m_ownedBuffer;
    m_bitLength = m_ownedBufferBitLength;
    memcpy(m_buffer, stream.m_buffer, getDivisionCeil(bitLength, 8));
    m_bitPosition = bitLength;
    m_writing = true;
}

bool VRageBitStream::ReadBoolean()
{
    return readInternal(1) > 0;
}

void VRageBitStream::WriteBoolean(bool value)
{
    writeInternal(value ? ULLONG_MAX : 0, 1);
}

void VRageBitStream::ReadMemory(void* ptr, int bitSize)
{
    int numLongs = bitSize / 8 / 8;
    uint64_t* p = (uint64_t*)ptr;
    for (int i = 0; i < numLongs; i++)
    {
        p[i] = ReadUInt64();
    }
    int remainingBits = bitSize - numLongs * 8 * 8;
    uint8_t* bptr = (uint8_t*)&p[numLongs];
    while (remainingBits > 0)
    {
        int readBits = std::min(remainingBits, 8);
        *bptr = ReadUInt8(readBits);
        remainingBits -= readBits;
        bptr++;
    }
}

void VRageBitStream::WriteMemory(void* ptr, int bitSize)
{
    int numLongs = bitSize / 8 / 8;
    for (int i = 0; i < numLongs; i++)
    {
        WriteUInt64((uint64_t) *((int64_t*)((uint8_t*)ptr + i * 8)), 64);
    }
    int remainingBits = bitSize - numLongs * 8 * 8;
    uint8_t* bptr = (uint8_t*)ptr + numLongs * 8;
    while (remainingBits > 0)
    {
        int writeBits = std::min(remainingBits, 8);
        WriteUInt8(*bptr, writeBits);
        remainingBits -= writeBits;
        bptr++;
    }
}

std::string VRageBitStream::ReadString()
{
    if (!ReadBoolean())
        return "";

    int byteCount = (int)ReadUInt32Variant();

    if (byteCount == 0)
        return "";

    uint8_t* ptr = new uint8_t[byteCount] { 0 };
    this->ReadMemory(ptr, byteCount * 8);

    std::string str((const char*)ptr, (size_t)byteCount);

    delete ptr;

    return str;
}

void VRageBitStream::WriteString(const std::string& str)
{
    WriteBoolean(true);

    WriteUInt32Variant(str.length());

    this->WriteMemory((void*)str.c_str(), str.length() * 8);
}

int8_t VRageBitStream::ReadInt8(int bitCount)
{
    return (int8_t)readInternal(bitCount);
}

uint8_t VRageBitStream::ReadUInt8(int bitCount)
{
    return (uint8_t)readInternal(bitCount);
}

int16_t VRageBitStream::ReadInt16(int bitCount)
{
    return (int16_t)readInternal(bitCount);
}

uint16_t VRageBitStream::ReadUInt16(int bitCount)
{
    return (uint16_t)readInternal(bitCount);
}

int32_t VRageBitStream::ReadInt32(int bitCount)
{
    return (int32_t)readInternal(bitCount);
}

uint32_t VRageBitStream::ReadUInt32(int bitCount)
{
    return (uint32_t)readInternal(bitCount);
}

int64_t VRageBitStream::ReadInt64(int bitCount)
{
    return (long)readInternal(bitCount);
}

uint64_t VRageBitStream::ReadUInt64(int bitCount)
{
    return readInternal(bitCount);
}

void VRageBitStream::WriteInt8(int8_t value, int bitCount)
{
    writeInternal((uint64_t)((int64_t)value), bitCount);
}

void VRageBitStream::WriteUInt8(uint8_t value, int bitCount)
{
    writeInternal((uint64_t)value, bitCount);
}

void VRageBitStream::WriteInt16(int16_t value, int bitCount)
{
    writeInternal((uint64_t)((int64_t)value), bitCount);
}

void VRageBitStream::WriteUInt16(uint16_t value, int bitCount)
{
    writeInternal((uint64_t)value, bitCount);
}

void VRageBitStream::WriteInt32(int32_t value, int bitCount)
{
    writeInternal((uint64_t)((int64_t)value), bitCount);
}

void VRageBitStream::WriteUInt32(uint32_t value, int bitCount)
{
    writeInternal((uint64_t)value, bitCount);
}

void VRageBitStream::WriteInt64(int64_t value, int bitCount)
{
    writeInternal((uint64_t)((int64_t)value), bitCount);
}

void VRageBitStream::WriteUInt64(uint64_t value, int bitCount)
{
    writeInternal((uint64_t)value, bitCount);
}

int32_t VRageBitStream::ReadInt32Variant()
{
    return int32_t();
}

int64_t VRageBitStream::ReadInt64Variant()
{
    return uint32_t();
}

uint32_t VRageBitStream::ReadUInt32Variant()
{
    uint32_t value = ReadUInt8();
    if ((value & 0x80) == 0) return value;
    value &= 0x7F;

    uint32_t chunk = ReadUInt8();
    value |= (chunk & 0x7F) << 7;
    if ((chunk & 0x80) == 0) return value;

    chunk = ReadUInt8();
    value |= (chunk & 0x7F) << 14;
    if ((chunk & 0x80) == 0) return value;

    chunk = ReadUInt8();
    value |= (chunk & 0x7F) << 21;
    if ((chunk & 0x80) == 0) return value;

    chunk = ReadUInt8();
    value |= chunk << 28; // can only use 4 bits from this chunk
    if ((chunk & 0xF0) == 0) return value;

    sLog->Error("BitStream: error when deserializing variant uint32");
    return 0;
}

uint64_t VRageBitStream::ReadUInt64Variant()
{
    uint64_t value = ReadUInt8();
    if ((value & 0x80) == 0) return value;
    value &= 0x7F;

    uint64_t chunk = ReadUInt8();
    value |= (chunk & 0x7F) << 7;
    if ((chunk & 0x80) == 0) return value;

    chunk = ReadUInt8();
    value |= (chunk & 0x7F) << 14;
    if ((chunk & 0x80) == 0) return value;

    chunk = ReadUInt8();
    value |= (chunk & 0x7F) << 21;
    if ((chunk & 0x80) == 0) return value;

    chunk = ReadUInt8();
    value |= (chunk & 0x7F) << 28;
    if ((chunk & 0x80) == 0) return value;

    chunk = ReadUInt8();
    value |= (chunk & 0x7F) << 35;
    if ((chunk & 0x80) == 0) return value;

    chunk = ReadUInt8();
    value |= (chunk & 0x7F) << 42;
    if ((chunk & 0x80) == 0) return value;


    chunk = ReadUInt8();
    value |= (chunk & 0x7F) << 49;
    if ((chunk & 0x80) == 0) return value;

    chunk = ReadUInt8();
    value |= (chunk & 0x7F) << 56;
    if ((chunk & 0x80) == 0) return value;

    chunk = ReadUInt8();
    value |= chunk << 63; // can only use 1 bit from this chunk

    if ((chunk & ~(uint64_t)0x01) != 0)
    {
        sLog->Error("BitStream: error when deserializing variant uint64");
        return 0;
    }

    return value;
}

void VRageBitStream::WriteUInt32Variant(uint32_t value)
{
    uint64_t value2;
    uint8_t* ptr = (uint8_t*)&value2;
    int num = 0;
    int num2 = 0;
    do
    {
        ptr[num2++] = (uint8_t)(value | 128);
        num++;
    } while ((value >>= 7) != 0);
    uint8_t* ptr2 = ptr + (num2 - 1);
    *ptr2 &= 127;
    writeInternal(value2, num * 8);
}

void VRageBitStream::WriteUInt64Variant(uint64_t value)
{
    uint8_t ptr[16];
    int num = 0;
    int num2 = 0;
    do
    {
        ptr[num2++] = (uint8_t)((value & 127) | 128);
        num++;
    } while ((value >> 7) != 0);
    uint8_t* ptr2 = ptr + (num2 - 1);
    *ptr2 &= 127;
    if (num > 8)
    {
        writeInternal((uint64_t)(*(int64_t*)ptr), 64);
        writeInternal((uint64_t)(*(int64_t*)(ptr + 8)), (num - 8) * 8);
        return;
    }
    writeInternal((uint64_t)(*(int64_t*)ptr), num * 8);
}

int VRageBitStream::getDivisionCeil(int num, int div)
{
    return (num - 1) / div + 1;
}

void VRageBitStream::ensureSize(int bitCount)
{
    if (m_bitLength < bitCount)
        this->resize(bitCount);
}

void VRageBitStream::resize(int bitSize)
{
    if (!OwnsBuffer())
        return;

    int newBitSize = std::max(m_bitLength * 2, bitSize);
    int newByteLen = getDivisionCeil(newBitSize, 64) * 8;

    uint64_t* newBuffer = (uint64_t*)malloc(newByteLen);
    memset(newBuffer, 0, newByteLen);
    memcpy(newBuffer, m_buffer, GetBytePosition());
    free(m_buffer);

    m_buffer = newBuffer;
    m_bitLength = newBitSize;

    m_ownedBuffer = m_buffer;
    m_ownedBufferBitLength = m_bitLength;
}

uint64_t VRageBitStream::readInternal(int bitSize)
{
    int longOffsetStart = m_bitPosition >> 6;
    int longOffsetEnd = m_bitPosition + bitSize - 1 >> 6;
    uint64_t basemask = ULLONG_MAX >> 64 - bitSize;
    int placeOffset = m_bitPosition & 63;
    uint64_t value = m_buffer[longOffsetStart] >> placeOffset;
    if (longOffsetEnd != longOffsetStart)
    {
        value |= m_buffer[longOffsetEnd] << 64 - placeOffset;
    }
    m_bitPosition += bitSize;
    return value & basemask;
}

void VRageBitStream::writeInternal(uint64_t value, int bitSize)
{
    if (bitSize == 0)
        return;

    ensureSize(m_bitPosition + bitSize);

    int longOffsetStart = m_bitPosition >> 6;
    int longOffsetEnd = m_bitPosition + bitSize - 1 >> 6;
    uint64_t basemask = ULLONG_MAX >> 64 - bitSize;
    int placeOffset = m_bitPosition & 63;
    value &= basemask;
    m_buffer[longOffsetStart] &= ~(basemask << placeOffset);
    m_buffer[longOffsetStart] |= value << placeOffset;
    if (longOffsetEnd != longOffsetStart)
    {
        m_buffer[longOffsetEnd] &= ~(basemask >> 64 - placeOffset);
        m_buffer[longOffsetEnd] |= value >> 64 - placeOffset;
    }
    m_bitPosition += bitSize;
}

int VRageBitStream::zag(uint32_t ziggedValue)
{
    int value = (int)ziggedValue;
    return (-(value & 0x01)) ^ ((value >> 1) & ~Int32Msb);
}

int VRageBitStream::zag(uint64_t ziggedValue)
{
    long value = (long)ziggedValue;
    return (-(value & 0x01L)) ^ ((value >> 1) & ~Int64Msb);
}
