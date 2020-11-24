#include "NetworkInterface.h"

#include "../util/LogUtility.h"
#include "../util/NetworkUtility.h"

#if WIN32
#include <WinSock2.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

NetworkInterface* sNetworkInterface = new NetworkInterface();

NetworkInterface::NetworkInterface()
{
    for (char i = 0; i < 5; i++)
    {
        m_aP2PReceiveChannels[i] = new P2PReceiveChannel(i);
    }    
}

NetworkInterface::~NetworkInterface()
{
    Shutdown();
}

bool NetworkInterface::Initialize()
{
    sLog->Info("Initializing Network Layer ...");

    // Create UDP Socket.
    m_hUDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (m_hUDPSocket == -1)
    {
        sLog->Error("Failed to initialize UDP Socket!");
        return false;
    }

    // Set Non Blocking
#if WIN32
    u_long lMode = 0;
    if (ioctlsocket(m_hUDPSocket, FIONBIO, &lMode) == SOCKET_ERROR)
    {
        sLog->Error("Failed to set UDP socket as non-blocking!");
        closesocket(m_hUDPSocket);
        return false;
    }
#else
    fcntl(m_hUDPSocket, F_SETFL, O_NONBLOCK);
#endif

    sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(27016);
    addr.sin_family = AF_INET;

    if (bind(m_hUDPSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        sLog->Error("Failed to bind UDP Socket!");
#if WIN32
        closesocket(m_hUDPSocket);
#else
        close(m_hUDPSocket);
#endif
        return false;
    }

    // Init Steam.
    if (!SteamGameServer_Init(0, 8766, 27016, MASTERSERVERUPDATERPORT_USEGAMESOCKETSHARE, eServerModeAuthenticationAndSecure, "1196019"))
    {
        sLog->Error("Failed to SteamGameServer_Init!");
        return false;
    }

    m_bInitialized = true;

    SteamGameServer()->SetModDir("Space Engineers");
    SteamGameServer()->SetProduct("Space Engineers");
    SteamGameServer()->SetGameDescription("Space Engineers");
    SteamGameServer()->SetDedicatedServer(true);

    sLog->Info("Network Layer initialized.");

    return true;
}

void NetworkInterface::Shutdown()
{
    if (m_bInitialized)
    {
        Stop();

        SteamGameServer_Shutdown();

#if WIN32
        closesocket(m_hUDPSocket);
#else
        close(m_hUDPSocket);
#endif
    }
}

bool NetworkInterface::Start()
{
    if (!m_bInitialized)
    {
        sLog->Error("Failed to start network interface: not initialized!");
        return false;
    }

    // Start the UDP Receive Thread.
    m_pUDPReceiveThread = new std::thread([this]()
    {
        UDPReceiveThread();
    });

    sLog->Info("Connecting to Steam ...");

    // Start the callbacks thread.
    m_pSteamCallbackThread = new std::thread([this]()
    {
        SteamCallbackThread();
    });

    // Start the P2P Receive Channels.
    for (char i = 0; i < 5; i++)
    {
        m_aP2PReceiveChannels[i]->Start();
    }

    SteamGameServer()->SetServerName("Development Server");
    SteamGameServer()->SetMapName("DevMap0.1");
    SteamGameServer()->SetGameTags("groupId0 version1196019 datahash mods0 gamemodeS1-1-1-1 view3000");
    SteamGameServer()->SetGameData("1196019");
    SteamGameServer()->SetMaxPlayerCount(7);

    SteamGameServer()->LogOnAnonymous();
    SteamGameServer()->EnableHeartbeats(true);

    while (!SteamGameServer()->GetPublicIP().IsSet())
    {
        // Wait to receive Public IP to consider ourselves connected to the Master Server.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::string strPublicIP = NetworkUtility::IPIntegerToString(SteamGameServer()->GetPublicIP().m_unIPv4, true);
    sLog->Info("Public IP is %s - Steam ID: %llu", strPublicIP.c_str(), SteamGameServer()->GetSteamID().ConvertToUint64());
    sLog->Info("Connected to Steam!");

    // Create the Lobby.
    //SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, 7);
    //m_LobbyCreatedCallResult.Set(hSteamAPICall, this, &NetworkInterface::OnLobbyCreated);

    return true;
}

void NetworkInterface::Stop()
{
    for (char i = 0; i < 5; i++)
    {
        m_aP2PReceiveChannels[i]->Stop();
    }

    if (m_pSteamCallbackThread)
    {
        m_bSteamCallbackThreadContinue = false;

        m_pSteamCallbackThread->join();

        delete m_pSteamCallbackThread;
        m_pSteamCallbackThread = nullptr;
    }

    if (m_pUDPReceiveThread)
    {
        m_bUDPReceiveThreadContinue = false;

        m_pUDPReceiveThread->join();

        delete m_pUDPReceiveThread;
        m_pUDPReceiveThread = nullptr;
    }
}

void NetworkInterface::SteamCallbackThread()
{
    m_bSteamCallbackThreadContinue = true;

    while (m_bSteamCallbackThreadContinue)
    {
        //SteamAPI_RunCallbacks();

        SteamGameServer_RunCallbacks();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void NetworkInterface::UDPReceiveThread()
{
    m_bUDPReceiveThreadContinue = true;

    while (m_bUDPReceiveThreadContinue)
    {
        sockaddr_in from;
#if WIN32
        int fromlen = sizeof(from);
#else
        socklen_t fromlen = sizeof(from);
#endif

        int recv = recvfrom(m_hUDPSocket, m_aUDPBuffer, 4096, 0, reinterpret_cast<sockaddr*>(&from), &fromlen);

        if(recv > 0)
        {
            // if this packet starts with 0xFFFFFFFF, forward it to Steam interface.
            if (recv > 4 && *((uint32_t*)m_aUDPBuffer) == 0xFFFFFFFF)
            {
                sLog->Info("Forwarding Steam Packet from %s:%d size = %d", NetworkUtility::IPIntegerToString(ntohl(from.sin_addr.s_addr)).c_str(), ntohs(from.sin_port), recv);
                NetworkUtility::HexDump(m_aUDPBuffer, recv);

                SteamGameServer()->HandleIncomingPacket(m_aUDPBuffer, recv, ntohl(from.sin_addr.s_addr), ntohs(from.sin_port));

                // Send Packets that Steam wants to send in this frame.
                char sendBuffer[16 * 1024] = { 0 };
                int sendSize = 0;
                uint32_t sendIP = 0;
                uint16_t sendPort = 0;
                while ((sendSize = SteamGameServer()->GetNextOutgoingPacket(sendBuffer, 16 * 1024, &sendIP, &sendPort)) != 0)
                {
                    sLog->Info("Found Steam Packet to forward to %s:%d size=%d", NetworkUtility::IPIntegerToString(sendIP).c_str(), sendPort, sendSize);
                    NetworkUtility::HexDump(sendBuffer, sendSize);

                    sockaddr_in dst;
                    dst.sin_family = AF_INET;
                    dst.sin_addr.s_addr = htonl(sendIP);
                    dst.sin_port = htons(sendPort);

                    int ret = sendto(m_hUDPSocket, sendBuffer, sendSize, 0, reinterpret_cast<const sockaddr*>(&dst), sizeof(dst));

                    if (ret < 0)
                    {
                        sLog->Info("Error when sending packet from Steam API!");
                    }
                }
            }
            else
            {
                sLog->Info("Received %d unknown bytes from UDP Socket", recv);
                NetworkUtility::HexDump(m_aUDPBuffer, recv);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void NetworkInterface::OnP2PSessionRequest(P2PSessionRequest_t* pData)
{
    sLog->Info("P2P Session Request received from %llu", pData->m_steamIDRemote.ConvertToUint64());
}

void NetworkInterface::OnP2PSessionConnectFail(P2PSessionConnectFail_t* pData)
{
    sLog->Info("P2P Session connect failed steamid %llu error %d", pData->m_steamIDRemote.ConvertToUint64(), pData->m_eP2PSessionError);
}

void NetworkInterface::OnGSClientApprove(GSClientApprove_t* pData)
{
    sLog->Info("GS client approved");
}

void NetworkInterface::OnGSClientDeny(GSClientDeny_t* pData)
{
    sLog->Info("GS client deny");
} 

void NetworkInterface::OnGSClientGroupStatus(GSClientGroupStatus_t* pData)
{
    sLog->Info("GS client group status");
}

void NetworkInterface::OnGSClientKick(GSClientKick_t* pData)
{
    sLog->Info("GS client kick");
}

void NetworkInterface::OnLobbyChatUpdate(LobbyChatUpdate_t *pParam)
{
    sLog->Info("lobby chat update");
}

void NetworkInterface::OnLobbyChatMsg(LobbyChatMsg_t *pParam)
{
    sLog->Info("lobby chat message");
}

void NetworkInterface::OnLobbyCreated(LobbyCreated_t *pCallback, bool bIOFailure)
{
    sLog->Info("Lobby created %d", pCallback->m_eResult);

    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "world", "sylardev");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "gameMode", "1");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "worldSize", "13168184");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "appVersion", "1.196.019");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "dataHash", "shfAewLc3toqKgW0hnL+PVd+UoxH8=");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "inventoryMultiplier", "1");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "blocksInventoryMultiplier", "1");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "assemblerMultiplier", "1");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "refineryMultiplier", "1");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "welderMultiplier", "1");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "grinderMultiplier", "1");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "mods", "0");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "view", "3000");
    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "scenario", "False");

    SteamMatchmaking()->SetLobbyData(CSteamID(pCallback->m_ulSteamIDLobby), "host_steamId", std::to_string(SteamGameServer()->GetSteamID().ConvertToUint64()).c_str());

    //SteamMatchmaking()->SetLobbyGameServer(CSteamID(pCallback->m_ulSteamIDLobby), 3232300643, 27016, SteamGameServer()->GetSteamID());
}
