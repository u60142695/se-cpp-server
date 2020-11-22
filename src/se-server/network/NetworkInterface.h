#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include "P2PReceiveChannel.h"

#include <steam/steam_gameserver.h>
#include <string>
#include <thread>

#if WIN32
#else
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

class NetworkInterface
{
public:
    NetworkInterface();
    ~NetworkInterface();

    bool            Initialize();
    void            Shutdown();

    bool            Start();
    void            Stop();
private:
    void            SteamCallbackThread();
    void            UDPReceiveThread();
private:
    // Callbacks
    STEAM_CALLBACK(NetworkInterface, OnP2PSessionRequest, P2PSessionRequest_t);
    STEAM_CALLBACK(NetworkInterface, OnP2PSessionConnectFail, P2PSessionConnectFail_t);

    STEAM_CALLBACK(NetworkInterface, OnGSClientApprove, GSClientApprove_t);
    STEAM_CALLBACK(NetworkInterface, OnGSClientDeny, GSClientDeny_t);
    STEAM_CALLBACK(NetworkInterface, OnGSClientGroupStatus, GSClientGroupStatus_t);
    STEAM_CALLBACK(NetworkInterface, OnGSClientKick, GSClientKick_t);

    STEAM_CALLBACK(NetworkInterface, OnLobbyChatUpdate, LobbyChatUpdate_t);
    STEAM_CALLBACK(NetworkInterface, OnLobbyChatMsg, LobbyChatMsg_t);

    // Call Results
    void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
    CCallResult<NetworkInterface, LobbyCreated_t> m_LobbyCreatedCallResult;
private:
    bool                m_bInitialized = false;
    std::thread*        m_pSteamCallbackThread = nullptr;
    bool                m_bSteamCallbackThreadContinue = false;
    P2PReceiveChannel*  m_aP2PReceiveChannels[5] = { 0 };
    // Main UDP Socket
    int                 m_hUDPSocket = 0;
    char                m_aUDPBuffer[4096] = { 0 };
    std::thread*        m_pUDPReceiveThread = nullptr;
    bool                m_bUDPReceiveThreadContinue = false;
};

extern NetworkInterface* sNetworkInterface;

#endif
