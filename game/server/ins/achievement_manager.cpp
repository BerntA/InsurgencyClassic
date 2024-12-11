//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: BrainBread 2 Achievement & Stat Handler
//
//========================================================================================//

#include "cbase.h"
#include "achievement_manager.h"
#include "GameBase_Shared.h"
#include "GameBase_Server.h"

const char* pszGameSkills[] = // TODO
{
	"BBX_ST_AGI_SPEED",
	"BBX_ST_AGI_ACROBATICS",
};

void AchievementManager::AnnounceAchievement(int plIndex, const char* pcAchievement, int iAchievementType)
{
	IGameEvent* event = gameeventmanager->CreateEvent("game_achievement");
	if (event)
	{
		event->SetString("ach_str", pcAchievement);
		event->SetInt("index", plIndex);
		event->SetInt("type", iAchievementType);
		gameeventmanager->FireEvent(event);
	}
}

// Set an achievement here.
bool AchievementManager::WriteToAchievement(CBasePlayer* pPlayer, const char* szAchievement, int iAchievementType)
{
	if (!CanWrite(pPlayer, szAchievement, iAchievementType))
	{
		DevMsg("Failed to write to the achievement %s!\n", szAchievement);
		return false;
	}

	CSteamID pSteamClient;
	if (!pPlayer->GetSteamID(&pSteamClient))
	{
		Warning("Unable to get SteamID for user %i\n", pPlayer->GetUserID());
		return false;
	}

	bool bAchieved = false;
	steamgameserverapicontext->SteamGameServerStats()->GetUserAchievement(pSteamClient, szAchievement, &bAchieved);
	if (bAchieved)
		return false;

	// Do the change.
	if (steamgameserverapicontext->SteamGameServerStats()->SetUserAchievement(pSteamClient, szAchievement))
	{
		// Store the change.
		steamgameserverapicontext->SteamGameServerStats()->StoreUserStats(pSteamClient);

		// Send Event:
		AnnounceAchievement(pPlayer->entindex(), szAchievement, iAchievementType);

		return true;
	}

	return false;
}

// Write to a stat
bool AchievementManager::WriteToStat(CBasePlayer* pPlayer, const char* szStat, int iForceValue, bool bAddTo)
{
	if (!CanWrite(pPlayer, szStat))
	{
		DevMsg("Failed to write to the stat %s!\n", szStat);
		return false;
	}

	CSteamID pSteamClient;
	if (!pPlayer->GetSteamID(&pSteamClient))
	{
		Warning("Unable to get SteamID for user %i\n", pPlayer->GetUserID());
		return false;
	}

	int iCurrentValue = 0, iMaxValue = GetMaxValueForStat(szStat);
	steamgameserverapicontext->SteamGameServerStats()->GetUserStat(pSteamClient, szStat, &iCurrentValue);

	// Make sure we're not above the max value!
	if (iMaxValue <= iCurrentValue)
		return false;

	if (iForceValue > 0)
	{
		if (bAddTo)
			iCurrentValue += iForceValue;
		else
			iCurrentValue = iForceValue;
	}
	else
		iCurrentValue++;

	// Give us achievements if the stats related to certain achievs have been surpassed.
	for (int i = 0; i < ACHIEVEMENTS::GetNumAchievements(); i++)
	{
		const achievementStatItem_t* pAchiev = ACHIEVEMENTS::GetAchievementItem(i);
		if (pAchiev && (pAchiev->value <= iCurrentValue) && pAchiev->szAchievement && pAchiev->szAchievement[0] && !strcmp(szStat, pAchiev->szStat))
			WriteToAchievement(pPlayer, pAchiev->szAchievement);
	}

	steamgameserverapicontext->SteamGameServerStats()->SetUserStat(pSteamClient, szStat, iCurrentValue);
	steamgameserverapicontext->SteamGameServerStats()->StoreUserStats(pSteamClient);
	return true;
}

bool AchievementManager::WriteToStatPvP(CBasePlayer* pPlayer, const char* szStat)
{
	// Make sure we have loaded stats, have PvP mode on, and have a server context!
	if (
		!pPlayer || !pPlayer->HasLoadedStats() || pPlayer->IsBot() ||
		!steamgameserverapicontext || !steamgameserverapicontext->SteamGameServerStats()
		)
		return false;

	CSteamID pSteamClient;
	if (!pPlayer->GetSteamID(&pSteamClient))
	{
		Warning("Unable to get SteamID for user %i\n", pPlayer->GetUserID());
		return false;
	}

	int iCurrentValue;
	steamgameserverapicontext->SteamGameServerStats()->GetUserStat(pSteamClient, szStat, &iCurrentValue);
	iCurrentValue++;
	iCurrentValue = clamp(iCurrentValue, 0, 9999999);

	steamgameserverapicontext->SteamGameServerStats()->SetUserStat(pSteamClient, szStat, iCurrentValue);
	steamgameserverapicontext->SteamGameServerStats()->StoreUserStats(pSteamClient);
	return true;
}

bool AchievementManager::IsGlobalStatsAllowed(void)
{
	return (steamgameserverapicontext && steamgameserverapicontext->SteamGameServerStats());
}

bool AchievementManager::CanLoadSteamStats(CBasePlayer* pPlayer)
{
	if (!engine->IsDedicatedServer() || !pPlayer || pPlayer->HasLoadedStats() || pPlayer->IsBot() || !steamgameserverapicontext || !steamgameserverapicontext->SteamGameServerStats())
		return false;

	CSteamID pSteamClient;
	if (!pPlayer->GetSteamID(&pSteamClient))
	{
		Warning("Unable to get SteamID for user %s\n", pPlayer->GetPlayerName());
		return false;
	}

	SteamAPICall_t apiCall = steamgameserverapicontext->SteamGameServerStats()->RequestUserStats(pSteamClient);
	pPlayer->m_SteamCallResultRequestPlayerStats.Set(apiCall, pPlayer, &CBasePlayer::OnReceivedSteamStats);
	return true;
}

// Can we write to this achiev/stat?
bool AchievementManager::CanWrite(CBasePlayer* pClient, const char* szParam, int iAchievementType)
{
	if (!pClient || !pClient->HasLoadedStats() || pClient->IsBot() || !IsGlobalStatsAllowed() || !szParam || !szParam[0])
		return false;

	if (iAchievementType == ACHIEVEMENT_TYPE_MAP)
	{
		for (int i = 0; i < ACHIEVEMENTS::GetNumAchievements(); i++)
		{
			const achievementStatItem_t* pAchiev = ACHIEVEMENTS::GetAchievementItem(i);
			if (pAchiev && (iAchievementType == pAchiev->type) && !strcmp(pAchiev->szAchievement, szParam))
				return true;
		}

		return false;
	}

	return true;
}

// Get the highest value for this stat. (if it is defined for multiple achievements it will have different max values)
int AchievementManager::GetMaxValueForStat(const char* szStat)
{
	int iValue = 0;
	for (int i = 0; i < ACHIEVEMENTS::GetNumAchievements(); i++)
	{
		const achievementStatItem_t* pAchiev = ACHIEVEMENTS::GetAchievementItem(i);
		if (pAchiev && (iValue < pAchiev->value) && !strcmp(szStat, pAchiev->szStat))
			iValue = pAchiev->value;
	}
	return iValue;
}