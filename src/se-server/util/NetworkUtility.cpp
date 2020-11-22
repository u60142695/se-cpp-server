#include "NetworkUtility.h"

#if WIN32
#include <WS2tcpip.h>
#include <intrin.h>
#endif

std::string NetworkUtility::IPIntegerToString(uint32_t lAddress, bool bSwapEndianness)
{
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = lAddress;

    if (bSwapEndianness)
    {
#if WIN32
        sa.sin_addr.s_addr = _byteswap_ulong(lAddress);
#endif
    }

    char str[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);

    return str;
}

void NetworkUtility::HexDump(void* pData, uint32_t lLength)
{
    unsigned char* buf = (unsigned char*)pData;
    int i, j;
    for (i = 0; i < lLength; i += 16) {
        printf("%06x: ", i);
        for (j = 0; j < 16; j++)
            if (i + j < lLength)
                printf("%02x ", buf[i + j]);
            else
                printf("   ");
        printf(" ");
        for (j = 0; j < 16; j++)
            if (i + j < lLength)
                printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
        printf("\n");
    }
}

void NetworkUtility::IPStringToSockAddrIn(const std::string& strIP, sockaddr_in* sockAddr)
{
    inet_pton(AF_INET, strIP.c_str(), &sockAddr->sin_addr);
}
