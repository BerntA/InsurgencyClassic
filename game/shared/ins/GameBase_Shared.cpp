//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: BrainBread 2 Shared Game Handler: Handles .bbd files for data parsing, reading, storing, etc...
//
//========================================================================================//

#include "cbase.h"
#include "GameBase_Shared.h"
#include "hl2mp_gamerules.h"
#include "particle_parse.h"
#include "checksum_sha1.h"

#ifndef CLIENT_DLL
#include "items.h"
#include "GameBase_Server.h"
#else
#include "fmod_manager.h"
#include "hud_macros.h"
#endif

#include <ctime>

void ClearCharVectorList(CUtlVector<char*> &list)
{
	for (int i = 0; i < list.Count(); i++)
		delete[] list[i];
	list.RemoveAll();
}

static CGameBaseShared gGameBaseShared;
CGameBaseShared* GameBaseShared()
{
	return &gGameBaseShared;
}

void CGameBaseShared::Init()
{
#ifdef CLIENT_DLL
	m_pMusicSystem = new CMusicSystem();
#endif

	m_pSharedGameDefinitions = NULL;
	m_pSharedGameMapData = NULL;
	LoadBase();

#ifdef CLIENT_DLL
#else
	m_pServerWorkshopData = new CGameDefinitionsWorkshop();
	m_pPlayerLoadoutHandler = new CPlayerLoadoutHandler();
#endif
}

void CGameBaseShared::LoadBase()
{
	// Not NULL? Release.
	if (m_pSharedGameDefinitions)
		delete m_pSharedGameDefinitions;

	if (m_pSharedGameMapData)
		delete m_pSharedGameMapData;

#ifdef CLIENT_DLL
	if (m_pMusicSystem)
		m_pMusicSystem->ParseMusicData();
#endif

	// We load our base values and such:
	m_pSharedGameDefinitions = new CGameDefinitionsShared();
	m_pSharedGameMapData = new CGameDefinitionsMapData();

#ifdef CLIENT_DLL
	Msg("Client loaded the game base successfully!\n");
#else
	Msg("Server loaded the game base successfully!\n");
#endif
}

void CGameBaseShared::Release()
{
	if (m_pSharedGameDefinitions)
		delete m_pSharedGameDefinitions;

	m_pSharedGameDefinitions = NULL;

	if (m_pSharedGameMapData)
		delete m_pSharedGameMapData;

	m_pSharedGameMapData = NULL;

#ifdef CLIENT_DLL
	if (m_pMusicSystem)
		delete m_pMusicSystem;
#else
	if (m_pServerWorkshopData)
		delete m_pServerWorkshopData;

	if (m_pPlayerLoadoutHandler)
		delete m_pPlayerLoadoutHandler;
#endif
}

////////////////////////////////////////////////
// Purpose:
// Decrypt and return a readable format @ a bbd file.
///////////////////////////////////////////////
KeyValues *CGameBaseShared::ReadEncryptedKeyValueFile(IFileSystem *filesystem, const char *filePath, bool bEncryption)
{
	char szFile[MAX_WEAPON_STRING];
	Q_strncpy(szFile, filePath, MAX_WEAPON_STRING);

	char szFileWithExtension[MAX_WEAPON_STRING];
	Q_snprintf(szFileWithExtension, MAX_WEAPON_STRING, "%s.txt", szFile);

	if (!filesystem->FileExists(szFileWithExtension, "MOD"))
	{
		bEncryption = true;

		Q_snprintf(szFileWithExtension, MAX_WEAPON_STRING, "%s.bbd", szFile);

		if (!filesystem->FileExists(szFileWithExtension, "MOD"))
		{
			Warning("Couldn't find the %s for .bbd or .txt!\n", szFile);
			return NULL;
		}
	}

	KeyValues *pKV = new KeyValues("BB2Data");

	// If we're not going to read an encrypted file, read a regular one.
	if (!bEncryption)
	{
		if (!pKV->LoadFromFile(filesystem, szFileWithExtension, "MOD"))
		{
			pKV->deleteThis();
			return NULL;
		}

		return pKV;
	}

	FileHandle_t f = filesystem->Open(szFileWithExtension, "rb", "MOD");
	if (!f)
	{
		pKV->deleteThis();
		return NULL;
	}

	// load file into a null-terminated buffer
	int fileSize = filesystem->Size(f);
	char *buffer = (char*)MemAllocScratch(fileSize + 1);

	Assert(buffer);

	filesystem->Read(buffer, fileSize, f); // read into local buffer
	buffer[fileSize] = 0; // null terminate file as EOF
	filesystem->Close(f);	// close file after reading

	UTIL_DecodeICE((unsigned char*)buffer, fileSize, GetEncryptionKey());

	bool retOK = pKV->LoadFromBuffer(szFileWithExtension, buffer, filesystem);

	MemFreeScratch();

	if (!retOK)
	{
		pKV->deleteThis();
		return NULL;
	}

	return pKV;
}

////////////////////////////////////////////////
// Purpose:
// Get the time input returned into a formatted string. Example: 4h, 34min, 20sec....
///////////////////////////////////////////////
const char *CGameBaseShared::GetTimeString(int iHoursPlayed)
{
	if (iHoursPlayed <= 0)
		return "N/A";

	int iCurrentValue = iHoursPlayed;

	int iYears = 0;
	int iMonths = 0;
	int iWeeks = 0;
	int iDays = 0;
	int iHours = 0;

	while (iCurrentValue >= TIME_STRING_YEAR)
	{
		iCurrentValue -= TIME_STRING_YEAR;
		iYears++;
	}

	while (iCurrentValue >= TIME_STRING_MONTH)
	{
		iCurrentValue -= TIME_STRING_MONTH;
		iMonths++;
	}

	while (iCurrentValue >= TIME_STRING_WEEK)
	{
		iCurrentValue -= TIME_STRING_WEEK;
		iWeeks++;
	}

	while (iCurrentValue >= TIME_STRING_DAY)
	{
		iCurrentValue -= TIME_STRING_DAY;
		iDays++;
	}

	iHours = iCurrentValue;

#ifdef CLIENT_DLL
	return VarArgs("%iY, %iM, %iW, %iD, %iH", iYears, iMonths, iWeeks, iDays, iHours);
#else
	return UTIL_VarArgs("%iY, %iM, %iW, %iD, %iH", iYears, iMonths, iWeeks, iDays, iHours);
#endif
}

////////////////////////////////////////////////
// Purpose:
// Read a file and return its contents.
///////////////////////////////////////////////
void CGameBaseShared::GetFileContent(const char *path, char *buf, int size)
{
	Q_strncpy(buf, "", size);

	FileHandle_t f = filesystem->Open(path, "rb", "MOD");
	if (f)
	{
		int fileSize = filesystem->Size(f);
		unsigned bufSize = ((IFileSystem *)filesystem)->GetOptimalReadSize(f, fileSize + 2);

		char *buffer = (char*)((IFileSystem *)filesystem)->AllocOptimalReadBuffer(f, bufSize);
		Assert(buffer);

		// read into local buffer
		bool bRetOK = (((IFileSystem *)filesystem)->ReadEx(buffer, bufSize, fileSize, f) != 0);

		filesystem->Close(f);	// close file after reading

		if (bRetOK)
		{
			buffer[fileSize] = 0; // null terminate file as EOF
			buffer[fileSize + 1] = 0; // double NULL terminating in case this is a unicode file

			Q_strncpy(buf, buffer, size);
		}

		((IFileSystem *)filesystem)->FreeOptimalReadBuffer(buffer);
	}
	else
		Warning("Unable to read file: %s\n", path);
}

////////////////////////////////////////////////
// Purpose:
// Used by firebullets in baseentity_shared. Returns the new damage to take depending on how far away the victim is from the start pos.
///////////////////////////////////////////////
float CGameBaseShared::GetDropOffDamage(const Vector &vecStart, const Vector &vecEnd, float damage, float minDist)
{
	// If min dist is zero we don't want drop off!
	if (minDist <= 0.0f)
		return damage;

	// If the dist traveled is not longer than minDist we don't care...
	float distanceTraveled = fabs((vecEnd - vecStart).Length());
	if (distanceTraveled <= minDist)
		return damage;

	return ((minDist / distanceTraveled) * damage);
}

////////////////////////////////////////////////
// Purpose:
// Get the raw sequence duration for any activity, the default SequenceDuration function only returns the duration of an active sequence, this func returns the duration regardless of that.
///////////////////////////////////////////////
float CGameBaseShared::GetSequenceDuration(CStudioHdr *ptr, int sequence)
{
	if (ptr)
	{
		int sequences = ptr->GetNumSeq();
		for (int i = 0; i < sequences; i++)
		{
			mstudioseqdesc_t &seqdesc = ptr->pSeqdesc(i);
			mstudioanimdesc_t &animdesc = ptr->pAnimdesc(ptr->iRelativeAnim(i, seqdesc.anim(0, 0)));
			if (seqdesc.activity == sequence)
			{
				float numFrames = ((float)animdesc.numframes);
				return (numFrames / animdesc.fps);
			}
		}
	}

	return 0.0f;
}

float CGameBaseShared::GetPlaybackSpeedThirdperson(CHL2MP_Player *pClient, int viewmodelActivity, int thirdpersonActivity)
{
	if (pClient == NULL || pClient->GetViewModel() == NULL || pClient->GetActiveWeapon() == NULL)
		return 1.0f;

	float durationViewmodel = GetSequenceDuration(pClient->GetViewModel()->GetModelPtr(), viewmodelActivity);
	float durationThirdperson = GetSequenceDuration(pClient->GetModelPtr(), (int)pClient->GetActiveWeapon()->ActivityOverride((Activity)thirdpersonActivity));
	if ((durationViewmodel > 0.0f) && (durationThirdperson > 0.0f))
		return (1.0f / (durationViewmodel / durationThirdperson));

	return 1.0f;
}

////////////////////////////////////////////////
// Purpose:
// Spawn the bleedout effect.
///////////////////////////////////////////////
void CGameBaseShared::DispatchBleedout(const Vector& vPos)
{
	Vector vecStart = vPos + Vector(0.0f, 0.0f, 5.0f);
	Vector vecEnd = vecStart + Vector(0.0f, 0.0f, -1.0f) * MAX_TRACE_LENGTH;

	trace_t tr;
	CTraceFilterNoNPCsOrPlayer filter(NULL, COLLISION_GROUP_DEBRIS);
	UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID_BRUSHONLY, &filter, &tr);

	if (tr.DidHitWorld() && !tr.allsolid && !tr.IsDispSurface() && (tr.fraction != 1.0f) && (tr.plane.normal.z == 1.0f))
		DispatchParticleEffect(GameBaseShared()->GetSharedGameDetails()->GetBleedoutParticle(), tr.endpos, vec3_angle);
}

#ifdef CLIENT_DLL
#else
////////////////////////////////////////////////
// Purpose:
// Called when the player kills any entity.
// Achievement progressing.
///////////////////////////////////////////////
void CGameBaseShared::EntityKilledByPlayer(CBaseEntity *pKiller, CBaseEntity *pVictim, CBaseEntity *pInflictor, int forcedWeaponID)
{
	if (!pKiller || !pKiller->IsPlayer() || !pVictim || !pInflictor)
		return;

	CHL2MP_Player *pClient = ToHL2MPPlayer(pKiller);
	if (!pClient || pClient->IsBot())
		return;

	if (pClient->GetTeamNumber() == TEAM_HUMANS)
	{
		if (!pClient->GetPerkFlags())
			pClient->m_iNumPerkKills++;
	}

	if (!HL2MPRules()->CanUseSkills() || (GameBaseServer()->CanStoreSkills() != PROFILE_GLOBAL))
		return;

	int uniqueWepID = forcedWeaponID;

	if (pClient->GetTeamNumber() == TEAM_HUMANS)
	{
		if (pVictim->Classify() == CLASS_ZOMBIE)
		{
			AchievementManager::WriteToAchievement(pClient, "ACH_ZOMBIE_FIRST_BLOOD");
			AchievementManager::WriteToStat(pClient, "BBX_KI_ZOMBIES");
		}

		if (uniqueWepID == WEAPON_ID_NONE)
		{
			CBaseCombatWeapon *pWeapon = pClient->GetActiveWeapon();
			if (pWeapon && FClassnameIs(pInflictor, "player"))
			{
				uniqueWepID = pWeapon->GetUniqueWeaponID();
				if (pWeapon->IsAkimboWeapon())
					AchievementManager::WriteToStat(pClient, "BBX_KI_AKIMBO");
			}
			else if (FClassnameIs(pInflictor, "npc_grenade_frag"))
				AchievementManager::WriteToStat(pClient, "BBX_KI_EXPLOSIVES");
			else if (FClassnameIs(pInflictor, "prop_propane_explosive"))
			{
				AchievementManager::WriteToAchievement(pClient, "ACH_GM_SURVIVAL_PROPANE");
				AchievementManager::WriteToStat(pClient, "BBX_KI_EXPLOSIVES");
			}
			else if (FClassnameIs(pInflictor, "prop_thrown_brick"))
			{
				AchievementManager::WriteToAchievement(pClient, "ACH_WEP_BRICK");
				AchievementManager::WriteToStat(pClient, "BBX_KI_BRICK");
			}
		}
	}
	else if (pClient->GetTeamNumber() == TEAM_DECEASED)
	{
		if (pVictim->IsHuman())
		{
			AchievementManager::WriteToAchievement(pClient, "ACH_ZOMBIE_KILL_HUMAN");
			AchievementManager::WriteToStat(pClient, "BBX_KI_HUMANS");
		}
	}

	if (uniqueWepID > WEAPON_ID_NONE)
	{
		switch (uniqueWepID)
		{
		case WEAPON_ID_BERETTA:
		case WEAPON_ID_BERETTA_AKIMBO:
			AchievementManager::WriteToStat(pClient, "BBX_KI_BERETTA");
			break;
		case WEAPON_ID_GLOCK17:
		case WEAPON_ID_GLOCK17_AKIMBO:
			AchievementManager::WriteToStat(pClient, "BBX_KI_GLOCK17");
			break;
		case WEAPON_ID_DEAGLE:
			AchievementManager::WriteToStat(pClient, "BBX_KI_DEAGLE");
			break;

		case WEAPON_ID_REXMP412:
		case WEAPON_ID_REXMP412_AKIMBO:
			break;

		case WEAPON_ID_AK74:
			AchievementManager::WriteToStat(pClient, "BBX_KI_AK74");
			break;
		case WEAPON_ID_FAMAS:
			AchievementManager::WriteToStat(pClient, "BBX_KI_FAMAS");
			break;
		case WEAPON_ID_G36C:
			AchievementManager::WriteToStat(pClient, "BBX_KI_G36C");
			break;
		case WEAPON_ID_WINCHESTER1894:
			AchievementManager::WriteToStat(pClient, "BBX_KI_TRAPPER");
			break;

		case WEAPON_ID_REMINGTON700:
			AchievementManager::WriteToStat(pClient, "BBX_KI_REM700");
			break;

		case WEAPON_ID_REMINGTON870:
			AchievementManager::WriteToStat(pClient, "BBX_KI_870");
			break;
		case WEAPON_ID_BENELLIM4:
			AchievementManager::WriteToStat(pClient, "BBX_KI_BENELLIM4");
			break;
		case WEAPON_ID_SAWEDOFF:
		case WEAPON_ID_SAWEDOFF_AKIMBO:
			AchievementManager::WriteToStat(pClient, "BBX_KI_SAWOFF");
			break;

		case WEAPON_ID_MINIGUN:
			AchievementManager::WriteToStat(pClient, "BBX_KI_MINIGUN");
			break;
		case WEAPON_ID_FLAMETHROWER:
			AchievementManager::WriteToStat(pClient, "BBX_KI_FLAMETHROWER");
			break;

		case WEAPON_ID_MAC11:
			AchievementManager::WriteToStat(pClient, "BBX_KI_MAC11");
			break;
		case WEAPON_ID_MP7:
			AchievementManager::WriteToStat(pClient, "BBX_KI_HKMP7");
			break;
		case WEAPON_ID_MP5:
			AchievementManager::WriteToStat(pClient, "BBX_KI_HKMP5");
			break;
		case WEAPON_ID_MICROUZI:
			AchievementManager::WriteToStat(pClient, "BBX_KI_UZI");
			break;

		case WEAPON_ID_HANDS:
			AchievementManager::WriteToStat(pClient, "BBX_KI_FISTS");
			break;
		case WEAPON_ID_ZOMBHANDS:
			break;
		case WEAPON_ID_BRICK:
			AchievementManager::WriteToStat(pClient, "BBX_KI_BRICK");
			break;
		case WEAPON_ID_KICK:
			AchievementManager::WriteToStat(pClient, "BBX_KI_KICK");
			break;
		case WEAPON_ID_M9BAYONET:
			AchievementManager::WriteToStat(pClient, "BBX_KI_M9PHROBIS");
			break;
		case WEAPON_ID_FIREAXE:
			AchievementManager::WriteToStat(pClient, "BBX_KI_FIREAXE");
			break;
		case WEAPON_ID_MACHETE:
			AchievementManager::WriteToStat(pClient, "BBX_KI_MACHETE");
			break;
		case WEAPON_ID_HATCHET:
			AchievementManager::WriteToStat(pClient, "BBX_KI_HATCHET");
			break;
		case WEAPON_ID_SLEDGEHAMMER:
			AchievementManager::WriteToStat(pClient, "BBX_KI_SLEDGEHAMMER");
			break;
		case WEAPON_ID_BASEBALLBAT:
			AchievementManager::WriteToStat(pClient, "BBX_KI_BASEBALLBAT");
			break;

		case WEAPON_ID_PROPANE:
			break;
		case WEAPON_ID_FRAG:
			break;
		}
	}

	AchievementManager::WriteToStat(pClient, "BBX_ST_KILLS");
	if (pVictim->IsNPC() && (pVictim->Classify() == CLASS_MILITARY))
		AchievementManager::WriteToStat(pClient, "BBX_KI_BANDITS");
}

////////////////////////////////////////////////
// Purpose:
// The game is over, we're changing map! Give achievs?
///////////////////////////////////////////////
void CGameBaseShared::OnGameOver(float timeLeft, int iWinner)
{
	if ((GameBaseServer()->CanStoreSkills() != PROFILE_GLOBAL) || (iWinner != TEAM_HUMANS))
		return;

	int iPlayersInGame = (HL2MPRules()->GetTeamSize(TEAM_HUMANS) + HL2MPRules()->GetTeamSize(TEAM_DECEASED));
	const char *currMap = HL2MPRules()->szCurrentMap;
	bool bTimeOut = (timeLeft <= 0.0f);
	bool bCanGiveMapAchiev = (!bTimeOut && ((iPlayersInGame >= 4)));

	char pchAchievement[64]; pchAchievement[0] = 0;
	if (bCanGiveMapAchiev)
	{
		for (int i = 0; i < ACHIEVEMENTS::GetNumAchievements(); i++)
		{
			const achievementStatItem_t *pAchiev = ACHIEVEMENTS::GetAchievementItem(i);
			if (pAchiev && pAchiev->szMapLink && pAchiev->szMapLink[0] && !strcmp(currMap, pAchiev->szMapLink))
			{
				Q_strncpy(pchAchievement, pAchiev->szAchievement, sizeof(pchAchievement));
				break;
			}
		}
	}

	if (!bTimeOut)
	{
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CHL2MP_Player *pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));
			if (!pPlayer || pPlayer->IsBot() || !pPlayer->HasFullySpawned())
				continue;

			if (pchAchievement && pchAchievement[0])
				AchievementManager::WriteToAchievement(pPlayer, pchAchievement);

			const char *pSpecialAchievement = NULL;
			if (!pPlayer->HasPlayerUsedFirearm() && (pPlayer->GetTotalScore() > 0))
			{
				if (HL2MPRules()->GetCurrentGamemode() == MODE_OBJECTIVE)
					pSpecialAchievement = "ACH_OBJ_NOFIREARMS";
				else if (HL2MPRules()->GetCurrentGamemode() == MODE_ARENA)
					pSpecialAchievement = "ACH_ENDGAME_NOFIREARMS";
			}

			if (pSpecialAchievement == NULL)
				continue;

			AchievementManager::WriteToAchievement(pPlayer, pSpecialAchievement);
		}
	}
}

////////////////////////////////////////////////
// Purpose:
// Handle client and server commands.
///////////////////////////////////////////////
bool CGameBaseShared::ClientCommand(const CCommand &args)
{
	return false;
}

////////////////////////////////////////////////
// Purpose:
// Notify everyone that we have a new player in our game!
///////////////////////////////////////////////
void CGameBaseShared::NewPlayerConnection(bool bState, int index)
{
	IGameEvent* event = gameeventmanager->CreateEvent("player_connection");
	if (event)
	{
		event->SetBool("state", bState); // False - Connected, True - Disconnected.
		event->SetInt("index", index);
		gameeventmanager->FireEvent(event);
	}
}

void CGameBaseShared::ComputePlayerWeight(CHL2MP_Player *pPlayer)
{
	if (!pPlayer)
		return;

	if (pPlayer->IsPerkFlagActive(PERK_POWERUP_CHEETAH) || (pPlayer->GetTeamNumber() == TEAM_DECEASED))
	{
		pPlayer->m_BB2Local.m_flCarryWeight = 0.0f;
		return;
	}

	float m_flWeight = 0.0f;

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetWeapon(i);
		if (!pWeapon || (pWeapon->GetSlot() >= MAX_WEAPON_SLOTS) || !pWeapon->VisibleInWeaponSelection())
			continue;

		m_flWeight += pWeapon->GetWpnData().m_flPhysicalWeight;
	}

	if (pPlayer->IsHuman() && (pPlayer->GetSkillValue(PLAYER_SKILL_HUMAN_LIGHTWEIGHT) > 0))
		m_flWeight -= ((m_flWeight / 100.0f) * pPlayer->GetSkillValue(PLAYER_SKILL_HUMAN_LIGHTWEIGHT, TEAM_HUMANS));

	if (m_flWeight <= 0.0f)
		m_flWeight = 0.0f;

	pPlayer->m_BB2Local.m_flCarryWeight = m_flWeight;
}
#endif

#ifndef CLIENT_DLL
CON_COMMAND(reload_gamebase_server, "Reload the game base content.(full reparse)")
{
	if (!sv_cheats || !sv_cheats->GetBool())
		return;

	GameBaseShared()->LoadBase();
	GameBaseShared()->GetSharedGameDetails()->Precache();
	GameBaseServer()->LoadSharedInfo();

	IGameEvent *event = gameeventmanager->CreateEvent("reload_game_data");
	if (event)
		gameeventmanager->FireEvent(event);
};
#else
CON_COMMAND(reload_gamebase_client, "Reload the game base content.(full reparse)")
{
	static ConVarRef cheats("sv_cheats");
	if (!cheats.GetBool())
		return;

	GameBaseShared()->LoadBase();
	GameBaseShared()->GetSharedGameDetails()->Precache();
	GameBaseShared()->GetSharedGameDetails()->LoadClientModels();

	IGameEvent *event = gameeventmanager->CreateEvent("reload_game_data");
	if (event)
		gameeventmanager->FireEventClientSide(event);
};
#endif