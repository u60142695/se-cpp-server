#ifndef STEAM_INTERFACE_H
#define STEAM_INTERFACE_H

#include <steam/steam_gameserver.h>
#include "p2p/IP2PChannel.h"

class SteamInterface
{
public:
    static SteamInterface* GetInstance();

    SteamInterface();
    ~SteamInterface();

    bool            Initialize();
    void            Shutdown();

    bool            Start();
    void            Stop();
private:
    void            SteamCallbackThread();
    void            UDPReceiveThread();
private:
    // Callbacks
    STEAM_GAMESERVER_CALLBACK(SteamInterface, OnSteamServersConnected, SteamServersConnected_t);
    STEAM_GAMESERVER_CALLBACK(SteamInterface, OnSteamServersDisconnected, SteamServersDisconnected_t);
    STEAM_GAMESERVER_CALLBACK(SteamInterface, OnSteamServerConnectFailure, SteamServerConnectFailure_t);

    STEAM_GAMESERVER_CALLBACK(SteamInterface, OnP2PSessionRequest, P2PSessionRequest_t);
    STEAM_GAMESERVER_CALLBACK(SteamInterface, OnP2PSessionConnectFail, P2PSessionConnectFail_t);
private:
    bool                m_bInitialized = false;
    // Steam Callback Thread
    std::thread*        m_pSteamCallbackThread = nullptr;
    bool                m_bSteamCallbackThreadContinue = false;
    // Main UDP Socket
    int                 m_hUDPSocket = 0;
    char                m_aUDPBuffer[4096] = { 0 };
    std::thread*        m_pUDPReceiveThread = nullptr;
    bool                m_bUDPReceiveThreadContinue = false;
    // P2P Listen Channels
    IP2PChannel*        m_aP2PReceiveChannels[5] = { 0 };
private:
    static SteamInterface* ms_instance;
};

#endif
