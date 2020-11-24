#include "util/LogUtility.h"
#include "network/SteamInterface.h"

#include <string>

#if WIN32
#include <winsock2.h>
#endif

int main(int argc, char* argv[])
{
    sLog->Info("Space Engineers Dedicated Server v0.1\n");

#if WIN32
    WSADATA wsaData;
    DWORD wsaVersion = MAKEWORD(2, 2);
    int wsaError = WSAStartup(wsaVersion, &wsaData);
    if (wsaError != 0)
    {
        sLog->Error("WSAStartup failed with error %d", wsaError);
        return 1;
    }
#endif

    if (!SteamInterface::GetInstance()->Initialize())
    {
        sLog->Error("Steam Interface init failed, exiting");
        return 1;
    }

    if (!SteamInterface::GetInstance()->Start())
    {
        sLog->Error("Steam Interface start failed, exiting");
        return 1;
    }
        
    bool bContinue = true;
    std::string strLine;
    while (bContinue)
    {
        std::getline(std::cin, strLine);

        if (strLine == "exit")
        {
            sLog->Info("Exiting. Have a nice day!");
            bContinue = false;
        }
    }

    // Shutdown work.
    SteamInterface::GetInstance()->Shutdown();

#if WIN32
    WSACleanup();
#endif

    return 0;
}
