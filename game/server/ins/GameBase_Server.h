//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: BrainBread 2 Server Handler - Handles kicking, voting, baning, map change, etc...
//
//========================================================================================//

#ifndef GAME_BASE_SERVER_H
#define GAME_BASE_SERVER_H
#ifdef _WIN32
#pragma once
#endif

#include "hl2mp_player.h"

struct sharedDataItem_t
{
	char szInfo[128];
	int iType;
	int iParam;
};

enum sharedDataTypes
{
	DATA_SECTION_DEVELOPER = 0,
	DATA_SECTION_DONATOR,
	DATA_SECTION_TESTER,
	DATA_SECTION_SERVER_ADMIN,
};

enum AdminLevelMask
{
	ADMIN_LEVEL_NONE = 0x00, // Empty
	ADMIN_LEVEL_KICK = 0x01, // a
	ADMIN_LEVEL_BAN = 0x02, // b
	ADMIN_LEVEL_MISC = 0x04, // c
};

enum profileSystemType
{
	PROFILE_NONE = 0,
	PROFILE_GLOBAL,
	PROFILE_LOCAL,
};

class CGameBaseServer
{
public:

	void Init();
	void Release();
	void PostInit();
	void PostLoad(float flDelay) { m_flPostLoadTimer = engine->Time() + flDelay; }

	// Global
	void LoadSharedInfo(void);
	void AddItemToSharedList(const char* str, int type, int param = 0);
	const sharedDataItem_t* FindItemInSharedList(const char* str, int type) const;
	void SetCurrentMapAddon(const char* map);
	void GameAnnouncement(const char* format, const char* arg1, const char* arg2);
	void NewPlayerConnection(CHL2MP_Player* pClient);
	float GetDamageScaleForEntity(CBaseEntity* pAttacker, CBaseEntity* pVictim, int damageType, int customDamageType);

	bool IsTutorialModeEnabled(void);
	bool IsStoryMode(void);

	bool CanEditSteamStats();
	int CanStoreSkills();
	void OnUpdate(int iClientsInGame);
	void DoMapChange(const char* map);

private:

	bool m_bFoundCheats;
	float m_flPostLoadTimer;

	CUtlVector<sharedDataItem_t> m_pSharedDataList;

	char szNextMap[MAX_MAP_NAME];
	bool m_bShouldChangeMap;
	float m_flTimeToChangeLevel;
	float m_flLastProfileSystemStatusUpdateCheck;

	inline const char* GetPublicIP(uint32 unIP) const;
};

extern CGameBaseServer* GameBaseServer();

#endif // GAME_BASE_SERVER_H