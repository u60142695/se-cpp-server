#ifndef STEAM_INTERFACE_H
#define STEAM_INTERFACE_H

#include <steam/steam_gameserver.h>
#include "P2PReceiveChannel.h"

class SteamInterface
{
public:
    static SteamInterface* GetInstance();

    bool            Initialize();
    void            Shutdown();

    bool            Start();
    void            Stop();
private:
    void            SteamCallbackThread();
    void            UDPReceiveThread();
private:
    // Callbacks
    STEAM_CALLBACK(SteamInterface, OnSteamServersConnected, SteamServersConnected_t);
    STEAM_CALLBACK(SteamInterface, OnSteamServersDisconnected, SteamServersDisconnected_t);
    STEAM_CALLBACK(SteamInterface, OnSteamServerConnectFailure, SteamServerConnectFailure_t);
private:
    bool                m_bInitialized = false;
    std::thread*        m_pSteamCallbackThread = nullptr;
    bool                m_bSteamCallbackThreadContinue = false;
    // Main UDP Socket
    int                 m_hUDPSocket = 0;
    char                m_aUDPBuffer[4096] = { 0 };
    std::thread*        m_pUDPReceiveThread = nullptr;
    bool                m_bUDPReceiveThreadContinue = false;
private:
    static SteamInterface* ms_instance;
};

#endif
