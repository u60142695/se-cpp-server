#ifndef NETWORK_UTILITY_H
#define NETWORK_UTILITY_H

#include <string>
#include <stdint.h>

#if WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

class NetworkUtility
{
public:
    static bool IsPlatformBigEndian();
    
    static std::string IPIntegerToString(uint32_t lAddress, bool bSwapEndianness = false);
    static void HexDump(void* pData, uint32_t lLength);
    static void IPStringToSockAddrIn(const std::string& strIP, sockaddr_in* sockAddr);
};

#endif
