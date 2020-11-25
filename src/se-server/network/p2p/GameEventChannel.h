#ifndef GAME_EVENT_CHANNEL_H
#define GAME_EVENT_CHANNEL_H

#include "IP2PChannel.h"

enum class eJoinResult
{
    OK = 0,
    AlreadyJoined,
    TicketInvalid,
    SteamServersOffline,
    NotInGroup,
    GroupIdInvalid,
    ServerFull,
    BannedByAdmins,
    KickedRecently,
    TicketCanceled,
    TickedAlreadyUsed,
    LoggedInElsewhere,
    NoLicenseOrExpired,
    UserNotConnected,
    VACBanned,
    VACCheckTimedOut,
    PasswordRequired,
    WrongPassword,
    ExperimentalMode,
    ProfilingNotAllowed,
    FamilySharing,
    Enum_Max
};

class GameEventChannel : public IP2PChannel
{
public:
    GameEventChannel();

    void    SendJoinResult(uint64_t steamId, eJoinResult eResult);
protected:
    void    HandlePacket(P2PPacket* pkt);
};

#endif
