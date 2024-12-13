//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: Shared Data Handler Class : Keeps information about the player and npc sound sets.
//
//========================================================================================//

#ifndef GAME_DEFINITIONS_SHARED_H
#define GAME_DEFINITIONS_SHARED_H

#ifdef _WIN32
#pragma once
#endif

#include "achievement_shareddefs.h"

#define PLAYER_GIB_GROUPS_MAX 5

enum ChatCommandType
{
	CHAT_CMD_VOICE = 1,
	CHAT_CMD_TEAM,
	CHAT_CMD_ALL,
};

struct DataPenetrationItem_t
{
	unsigned short material;
	float depth;
};

class CGameDefinitionsShared
{
public:
	CGameDefinitionsShared();
	~CGameDefinitionsShared();

	void Cleanup(void);
	bool LoadData(void);
	bool Precache(void);
#ifdef CLIENT_DLL
	void LoadClientModels(void);
#endif

	const char* GetBloodParticle(bool bExtremeGore = false);
	const char* GetHeadshotParticle(bool bExtremeGore = false);
	const char* GetBleedoutParticle(bool bExtremeGore = false);
	const char* GetBloodExplosionMist(bool bExtremeGore = false);
	const char* GetGibParticleForLimb(const char* limb, bool bExtremeGore = false);
};

namespace ACHIEVEMENTS
{
	const achievementStatItem_t* GetAchievementItem(int index);
	const achievementStatItem_t* GetAchievementItem(const char* str);
	int GetNumAchievements(void);
}

extern const DataPenetrationItem_t* GetPenetrationDataForMaterial(unsigned short material);
extern Vector TryPenetrateSurface(trace_t* tr, ITraceFilter* filter);

const char* COM_GetModDirectory();

#endif // GAME_DEFINITIONS_SHARED_H