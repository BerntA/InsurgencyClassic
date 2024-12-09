//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: BrainBread 2 Shared Game Handler: Handles .bbd files for data parsing, reading, storing, etc...
//
//========================================================================================//

#include "cbase.h"
#include "GameBase_Shared.h"
#include "particle_parse.h"
#include "checksum_sha1.h"
#include "weapon_defines_shared.h"

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
	m_pSharedGameDefinitions = NULL;
	m_pSharedGameMapData = NULL;
	LoadBase();

#ifdef CLIENT_DLL
#else
	m_pServerWorkshopData = new CGameDefinitionsWorkshop();
#endif
}

void CGameBaseShared::LoadBase()
{
	// Not NULL? Release.
	if (m_pSharedGameDefinitions)
		delete m_pSharedGameDefinitions;

	if (m_pSharedGameMapData)
		delete m_pSharedGameMapData;

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
#else
	if (m_pServerWorkshopData)
		delete m_pServerWorkshopData;
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

	CBasePlayer *pClient = ToBasePlayer(pKiller);
	if (!pClient || pClient->IsBot())
		return;

	int uniqueWepID = forcedWeaponID;

	if (uniqueWepID > INVALID_WEAPON) // TODO
	{
		switch (uniqueWepID)
		{
		case WEAPON_M9:
			AchievementManager::WriteToStat(pClient, "BBX_KI_BERETTA");
			break;
		}
	}

	AchievementManager::WriteToStat(pClient, "BBX_ST_KILLS");
}
#endif

#ifndef CLIENT_DLL
CON_COMMAND(reload_gamebase_server, "Reload the game base content.(full reparse)")
{
	if (!sv_cheats || !sv_cheats->GetBool())
		return;

	GameBaseShared()->LoadBase();
	GameBaseShared()->GetSharedGameDetails()->Precache();

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