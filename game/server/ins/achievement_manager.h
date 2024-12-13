//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: BrainBread 2 Achievement & Stat Handler
//
//========================================================================================//

#ifndef SHARED_ACHIEVEMENT_HANDLER_H
#define SHARED_ACHIEVEMENT_HANDLER_H

#ifdef _WIN32
#pragma once
#endif

#include <steam/steam_api.h>
#include "gameinterface.h"
#include "achievement_shareddefs.h"
#include "player.h"

namespace AchievementManager
{
	void AnnounceAchievement(int plIndex, const char* pcAchievement, int iAchievementType = 0);
	bool WriteToAchievement(CBasePlayer* pPlayer, const char* szAchievement, int iAchievementType = 0);
	bool WriteToStat(CBasePlayer* pPlayer, const char* szStat, int iForceValue = 0, bool bAddTo = false);
	bool WriteToStatPvP(CBasePlayer* pPlayer, const char* szStat);
	bool IsGlobalStatsAllowed(void);
	bool CanLoadSteamStats(CBasePlayer* pPlayer);
	bool CanWrite(CBasePlayer* pClient, const char* szParam, int iAchievementType = 0);
	int GetMaxValueForStat(const char* szStat);
}

#endif // SHARED_ACHIEVEMENT_HANDLER_H