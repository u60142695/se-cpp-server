#include "SteamInterface.h"

#include "../util/LogUtility.h"
#include "../util/NetworkUtility.h"

#include "p2p/ControlChannel.h"
#include "p2p/GameEventChannel.h"
#include "p2p/ProfilerDownloadChannel.h"
#include "p2p/VoiceChatChannel.h"
#include "p2p/WorldDownloadChannel.h"

#include "../player/PlayerManager.h"

#if WIN32
#include <WinSock2.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

SteamInterface* SteamInterface::ms_instance = nullptr;

SteamInterface* SteamInterface::GetInstance()
{
    if (!ms_instance)
    {
        // Initialize Steam.
        if (!SteamGameServer_Init(0, 8766, 27016, MASTERSERVERUPDATERPORT_USEGAMESOCKETSHARE, eServerModeAuthenticationAndSecure, "1196019"))
        {
            sLog->Error("Failed to SteamGameServer_Init!");
            return nullptr;
        }

        ms_instance = new SteamInterface();
    }

    return ms_instance;
}

SteamInterface::SteamInterface()
{
    m_aP2PReceiveChannels[0] = new ControlChannel();
    m_aP2PReceiveChannels[1] = new WorldDownloadChannel();
    m_aP2PReceiveChannels[2] = new GameEventChannel();
    m_aP2PReceiveChannels[3] = new VoiceChatChannel();
    m_aP2PReceiveChannels[4] = new ProfilerDownloadChannel();
}

SteamInterface::~SteamInterface()
{
    delete ((ControlChannel*)m_aP2PReceiveChannels[0]);
    delete ((WorldDownloadChannel*)m_aP2PReceiveChannels[1]);
    delete ((GameEventChannel*)m_aP2PReceiveChannels[2]);
    delete ((VoiceChatChannel*)m_aP2PReceiveChannels[3]);
    delete ((ProfilerDownloadChannel*)m_aP2PReceiveChannels[4]);
}

bool SteamInterface::Initialize()
{
    sLog->Info("Initializing Steam Interface ...");

    // Create UDP Socket.
    m_hUDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (m_hUDPSocket == -1)
    {
        sLog->Error("Failed to initialize UDP Socket!");
        return false;
    }

    // Set UDP Socket Blocking
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

    SteamGameServer()->SetModDir("Space Engineers");
    SteamGameServer()->SetProduct("Space Engineers");
    SteamGameServer()->SetGameDescription("Space Engineers");
    SteamGameServer()->SetDedicatedServer(true);

    sLog->Info("Steam Interface initialized.");

    m_bInitialized = true;    

    return true;
}

void SteamInterface::Shutdown()
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

bool SteamInterface::Start()
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

    return true;
}

void SteamInterface::Stop()
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

void SteamInterface::SteamCallbackThread()
{
    m_bSteamCallbackThreadContinue = true;

    while (m_bSteamCallbackThreadContinue)
    {
        SteamGameServer_RunCallbacks();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void SteamInterface::UDPReceiveThread()
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

        if (recv > 0)
        {
            // if this packet starts with 0xFFFFFFFF, forward it to Steam interface.
            if (recv > 4 && *((uint32_t*)m_aUDPBuffer) == 0xFFFFFFFF)
            {
                sLog->Info("[STEAM] Forwarding Steam Packet from %s:%d size = %d", NetworkUtility::IPIntegerToString(ntohl(from.sin_addr.s_addr)).c_str(), ntohs(from.sin_port), recv);
                //NetworkUtility::HexDump(m_aUDPBuffer, recv);

                SteamGameServer()->HandleIncomingPacket(m_aUDPBuffer, recv, ntohl(from.sin_addr.s_addr), ntohs(from.sin_port));

                // Send Packets that Steam wants to send in this frame.
                char sendBuffer[16 * 1024] = { 0 };
                int sendSize = 0;
                uint32_t sendIP = 0;
                uint16_t sendPort = 0;
                while ((sendSize = SteamGameServer()->GetNextOutgoingPacket(sendBuffer, 16 * 1024, &sendIP, &sendPort)) != 0)
                {
                    sLog->Info("[STEAM] Found Steam Packet to forward to %s:%d size=%d", NetworkUtility::IPIntegerToString(sendIP).c_str(), sendPort, sendSize);
                    //NetworkUtility::HexDump(sendBuffer, sendSize);

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
                sLog->Info("[NET] Received %d unknown bytes from UDP Socket", recv);
                NetworkUtility::HexDump(m_aUDPBuffer, recv);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void SteamInterface::OnSteamServersConnected(SteamServersConnected_t* pCallback)
{
    sLog->Info("Connected to Steam Network.");
}

void SteamInterface::OnSteamServersDisconnected(SteamServersDisconnected_t* pCallback)
{
    sLog->Info("Disconnected from Steam Network.");
}

void SteamInterface::OnSteamServerConnectFailure(SteamServerConnectFailure_t* pCallback)
{
    sLog->Info("Failed to connect to Steam Network.");
}

void SteamInterface::OnP2PSessionRequest(P2PSessionRequest_t* pCallback)
{
    sLog->Info("P2P Session Request from %llu", pCallback->m_steamIDRemote.ConvertToUint64());

    SteamGameServerNetworking()->AcceptP2PSessionWithUser(pCallback->m_steamIDRemote);
}

void SteamInterface::OnP2PSessionConnectFail(P2PSessionConnectFail_t* pCallback)
{
    sLog->Info("P2P Connect Failed");
}

void SteamInterface::OnValidateAuthTicketResponse(ValidateAuthTicketResponse_t* pCallback)
{
    PlayerManager::GetInstance()->EndAuthenticate(pCallback->m_SteamID.ConvertToUint64(), pCallback->m_eAuthSessionResponse);
}
