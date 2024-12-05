//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef C_STATSMAN_H
#define C_STATSMAN_H
#ifdef _WIN32
#pragma once
#endif

/*#include "c_statsman.h"
#include "statsman_api.h"
#include "playerstats.h"
#include "stats_protocal.h"

//=========================================================
//=========================================================
#define MAX_PLAYER_FINDS 8
#define INVALID_PLAYERINFO_HANDLE -1

typedef int HPlayerInfo;

struct PlayerInfoData_t
{
	HPlayerInfo m_hID;

	bool m_bReady;
	PlayerStats_t m_PlayerStats;
	PlayerAvatar_t m_Avatar;
};

//=========================================================
//=========================================================
class C_StatsMan
{
public:
	C_StatsMan();
	
	virtual HPlayerInfo FindPlayerInfo(bool bLong, int iMemberID, const char *pszDBName);
	virtual bool GetPlayerInfo(HPlayerInfo hID, PlayerStats_t &PlayerStats);

private:
	PlayerInfoData_t m_PlayerInfoHandles[MAX_PLAYER_FINDS];
};

//=========================================================
//=========================================================
extern "C" __declspec(dllexport) C_StatsMan *GetCStatsMan(void);

//deathz0rz [
#ifdef WIN32
extern "C" __declspec(dllexport) void InitDLL(char* pszSearchPath);
#endif
//deathz0rz ]*/

#endif // C_STATSMAN_H