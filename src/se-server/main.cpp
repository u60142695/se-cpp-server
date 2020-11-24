#include "util/LogUtility.h"
#include "network/NetworkInterface.h"

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
        return false;
    }
#endif

    // Initialize Steam API.
    /*if (!SteamAPI_Init())
    {
        sLog->Info("Failed to initialize Steam API!");
        return 1;
    }*/

    if (!sNetworkInterface->Initialize())
        return 1;

    if (!sNetworkInterface->Start())
        return 1;
    
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
    sNetworkInterface->Shutdown();

    //SteamAPI_Shutdown();

#if WIN32
    WSACleanup();
#endif

    return 0;
}
