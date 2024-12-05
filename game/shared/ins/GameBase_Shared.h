//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: BrainBread 2 Shared Game Handler: Handles .bbd files for data parsing, reading, storing, etc...
//
//========================================================================================//

#ifndef GAME_BASE_SHARED_H
#define GAME_BASE_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "KeyValues.h"
#include "filesystem.h"
#include "GameDefinitions_Shared.h"
#include "skills_shareddefs.h"
#include "GameDefinitions_MapData.h"

#ifdef CLIENT_DLL
#include "c_hl2mp_player.h"
#include "music_system.h"
#else
#include "hl2mp_player.h"
#include "achievement_manager.h"
#include "GameDefinitions_Workshop.h"
#include "player_loadout_handler.h"
#endif

#define DELAYED_USE_TIME 1.5f
#define MAX_MELEE_LAGCOMP_DIST 300.0f

// Time Definitions in hours.
#define TIME_STRING_YEAR 8760
#define TIME_STRING_MONTH 730
#define TIME_STRING_WEEK 168
#define TIME_STRING_DAY 24

#ifdef CLIENT_DLL
#define CGameBaseShared C_GameBaseShared
#endif

enum
{
	GROUPID_IS_DEVELOPER = 0x001,

	GROUPID_IS_TESTER = 0x002,
	GROUPID_IS_DONATOR = 0x004,

	MAX_GROUPID_BITS = 3
};

enum
{
	MAT_OVERLAY_BLOOD = 0x001,
	MAT_OVERLAY_SPAWNPROTECTION = 0x002,
	MAT_OVERLAY_CRIPPLED = 0x004,
	MAT_OVERLAY_BURNING = 0x008,
	MAT_OVERLAY_COLDSNAP = 0x010,
	MAT_OVERLAY_BLEEDING = 0x020,
	MAX_MAT_OVERLAYS_BITS = 6
};

enum MeleeAttackTypes_t
{
	MELEE_TYPE_BLUNT = 1, // Stab, Club, Smack, etc...
	MELEE_TYPE_SLASH, // Slash, Strike, etc...
	MELEE_TYPE_BASH_BLUNT, // Bash Strike... Push back strike!
	MELEE_TYPE_BASH_SLASH, // Bash Slash + push back.
};

enum ClientAttachmentTypes_t
{
	CLIENT_ATTACHMENT_WEAPON = 0,
};

class CGameBaseShared
{
public:

	void Init();
	void LoadBase();
	void Release();

	const unsigned char* GetEncryptionKey(void) { return (unsigned char*)"F3QxBzK6"; }

	// Game Base
	KeyValues* ReadEncryptedKeyValueFile(IFileSystem* filesystem, const char* filePath, bool bEncryption = false);

	// Base Stat / Skill / Player Values: Base Models / Survivors / Character Profiles:
	CGameDefinitionsShared* GetSharedGameDetails(void) { return m_pSharedGameDefinitions; }
	CGameDefinitionsMapData* GetSharedMapData(void) { return m_pSharedGameMapData; }

	// Misc
	const char* GetTimeString(int iHoursPlayed);
	void GetFileContent(const char* path, char* buf, int size);
	float GetDropOffDamage(const Vector& vecStart, const Vector& vecEnd, float damage, float minDist);
	float GetSequenceDuration(CStudioHdr* ptr, int sequence);
	float GetPlaybackSpeedThirdperson(CHL2MP_Player* pClient, int viewmodelActivity, int thirdpersonActivity);

	// Bleeding Dispatches
	void DispatchBleedout(const Vector& vPos);

#ifdef CLIENT_DLL
#else
	// Achievement Checks
	void EntityKilledByPlayer(CBaseEntity* pKiller, CBaseEntity* pVictim, CBaseEntity* pInflictor, int forcedWeaponID = WEAPON_ID_NONE);
	void OnGameOver(float timeLeft, int iWinner);

	// Workshop Handler:
	CGameDefinitionsWorkshop* GetServerWorkshopData(void) { return m_pServerWorkshopData; }

	// Loadout Handler:
	CPlayerLoadoutHandler* GetPlayerLoadoutHandler(void) { return m_pPlayerLoadoutHandler; }

	// Server Commands and Client Commands checks.
	bool ClientCommand(const CCommand& args);

	// Events
	void NewPlayerConnection(bool bState, int index);

	// Misc
	void ComputePlayerWeight(CHL2MP_Player* pPlayer);
#endif

private:

#ifdef CLIENT_DLL
	CMusicSystem* m_pMusicSystem;
#else
	// Server Workshop Handler:
	CGameDefinitionsWorkshop* m_pServerWorkshopData;
	// Server Loadout Handler:
	CPlayerLoadoutHandler* m_pPlayerLoadoutHandler;
#endif

	// Shared Game Data Info
	CGameDefinitionsShared* m_pSharedGameDefinitions;
	CGameDefinitionsMapData* m_pSharedGameMapData;
};

void ClearCharVectorList(CUtlVector<char*>& list);
extern CGameBaseShared* GameBaseShared();

#endif // GAME_BASE_SHARED_H