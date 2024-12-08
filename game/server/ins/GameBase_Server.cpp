//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: BrainBread 2 Server Handler - Handles shared data, npc/player hitgroup scaling, map change, etc...
//
//========================================================================================//

#include "cbase.h"
#include "GameBase_Server.h"
#include "filesystem.h"
#include "GameBase_Shared.h"
#include "inetchannelinfo.h"
#include "movevars_shared.h"
#include "world.h"
#include "tier0/icommandline.h"

ConVar bb2_tutorial_mode("bb2_tutorial_mode", "0", FCVAR_CHEAT | FCVAR_GAMEDLL, "Force-enable tutorial mode for any map, useful for testing.", true, 0.0f, true, 1.0f);

static CGameBaseServer gServerMode;
CGameBaseServer* GameBaseServer()
{
	return &gServerMode;
}

void CGameBaseServer::Init()
{
	m_flPostLoadTimer = 0.0f;
	m_bFoundCheats = false;
	m_bShouldChangeMap = false;
	szNextMap[0] = 0;
	m_pSharedDataList.Purge();
}

void CGameBaseServer::Release()
{
	m_bShouldChangeMap = false;
	szNextMap[0] = 0;
	m_pSharedDataList.Purge();
}

// Handle global stuff here:

void CGameBaseServer::LoadSharedInfo(void)
{
	m_pSharedDataList.Purge();
	KeyValues* pkvAdminData = new KeyValues("AdminList");
	if (pkvAdminData->LoadFromFile(filesystem, "data/settings/admins.txt", "MOD"))
	{
		char pchTemp[16]; pchTemp[0] = 0;
		for (KeyValues* sub = pkvAdminData->GetFirstSubKey(); sub; sub = sub->GetNextKey())
		{
			int mask = ADMIN_LEVEL_NONE;
			Q_strncpy(pchTemp, sub->GetName(), sizeof(pchTemp));

			if (Q_stristr(pchTemp, "a"))
				mask |= ADMIN_LEVEL_KICK;

			if (Q_stristr(pchTemp, "b"))
				mask |= ADMIN_LEVEL_BAN;

			if (Q_stristr(pchTemp, "c"))
				mask |= ADMIN_LEVEL_MISC;

			AddItemToSharedList(sub->GetString(), DATA_SECTION_SERVER_ADMIN, mask);
		}
	}
	pkvAdminData->deleteThis();
}

void CGameBaseServer::AddItemToSharedList(const char* str, int type, int param)
{
	sharedDataItem_t item;
	Q_strncpy(item.szInfo, str, 128);
	item.iType = type;
	item.iParam = param;
	m_pSharedDataList.AddToTail(item);
}

const sharedDataItem_t* CGameBaseServer::FindItemInSharedList(const char* str, int type) const
{
	for (int i = 0; i < m_pSharedDataList.Count(); i++)
	{
		if ((type == m_pSharedDataList[i].iType) && !strcmp(str, m_pSharedDataList[i].szInfo))
			return &m_pSharedDataList[i];
	}
	return NULL;
}

// Set current workshop ID regardless of SteamUGC state!
// Use workshop file to find the right ID.
void CGameBaseServer::SetCurrentMapAddon(const char* map)
{
	if (engine->IsDedicatedServer() == false)
	{
		bb2_active_workshop_item.SetValue(0);
		return;
	}

	char pchWorkshopID[80], pchTargetFile[MAX_PATH];
	Q_strncpy(pchWorkshopID, "0", 80);

	CUtlVector<PublishedFileId_t> workshopMaps;
	if (GameBaseShared() && GameBaseShared()->GetServerWorkshopData())
		GameBaseShared()->GetServerWorkshopData()->GetListOfAddons(workshopMaps, true);

	for (int i = 0; i < workshopMaps.Count(); i++)
	{
		Q_snprintf(pchTargetFile, MAX_PATH, "workshop/content/346330/%llu/maps/%s.bsp", workshopMaps[i], map);
		if (filesystem->FileExists(pchTargetFile, "MOD"))
		{
			Q_snprintf(pchWorkshopID, 80, "%llu", workshopMaps[i]);
			break;
		}
	}

	workshopMaps.Purge();

	bb2_active_workshop_item.SetValue(pchWorkshopID);
}

// Send a message to all the clients in-game.
void CGameBaseServer::GameAnnouncement(const char* format, const char* arg1, const char* arg2)
{
	CRecipientFilter filter;
	filter.AddAllPlayers();
	filter.MakeReliable();
	UTIL_ClientPrintFilter(filter, HUD_PRINTTALK, format, arg1, arg2);
}

void CGameBaseServer::NewPlayerConnection(CHL2MP_Player* pClient)
{
	if (!pClient)
		return;

	char steamID[80];
	Q_snprintf(steamID, 80, "%llu", pClient->GetSteamIDAsUInt64());

	const sharedDataItem_t* pAdminItem = FindItemInSharedList(steamID, DATA_SECTION_SERVER_ADMIN);
	pClient->SetAdminLevel(pAdminItem ? pAdminItem->iParam : 0);

	if (engine->IsDedicatedServer())
	{
		if (FindItemInSharedList(steamID, DATA_SECTION_DEVELOPER))
			pClient->AddGroupIDFlag(GROUPID_IS_DEVELOPER);

		if (FindItemInSharedList(steamID, DATA_SECTION_DONATOR))
			pClient->AddGroupIDFlag(GROUPID_IS_DONATOR);

		if (FindItemInSharedList(steamID, DATA_SECTION_TESTER))
			pClient->AddGroupIDFlag(GROUPID_IS_TESTER);
	}
}

float CGameBaseServer::GetDamageScaleForEntity(CBaseEntity* pAttacker, CBaseEntity* pVictim, int damageType, int customDamageType)
{
	return 1.0f;
}

bool CGameBaseServer::IsTutorialModeEnabled(void)
{
	if (engine->IsDedicatedServer())
		return false;

	return ((GetWorldEntity() && GetWorldEntity()->IsTutorialMap()) || bb2_tutorial_mode.GetBool());
}

bool CGameBaseServer::IsStoryMode(void)
{
	return (GetWorldEntity() && GetWorldEntity()->IsStoryMap() && (HL2MPRules() && (HL2MPRules()->GetCurrentGamemode() == MODE_OBJECTIVE)));
}

// Late init.
void CGameBaseServer::PostInit()
{
	m_bFoundCheats = false;
	m_bShouldChangeMap = false;
	m_flLastProfileSystemStatusUpdateCheck = 0.0f;
	GameBaseServer()->LoadSharedInfo();
}

// Are we allowed to manipulate steam stats?
bool CGameBaseServer::CanEditSteamStats()
{
	return (!m_bFoundCheats && engine->IsDedicatedServer() && sv_cheats && !sv_cheats->GetBool() && (gpGlobals->maxClients > 1));
}

// Are you allowed to store your skills?
int CGameBaseServer::CanStoreSkills()
{
	// Are we even allowed to use skills?
	if (HL2MPRules() && !HL2MPRules()->CanUseSkills())
		return PROFILE_NONE;

	int profileType = bb2_allow_profile_system.GetInt();

	if (profileType == PROFILE_GLOBAL)
	{
		if (!CanEditSteamStats())
			return PROFILE_NONE;
	}

	if (IsTutorialModeEnabled())
		return PROFILE_NONE;

	return profileType;
}

// Think func for the server mode:
void CGameBaseServer::OnUpdate(int iClientsInGame)
{
	// Hunt for sv_cheats & sv_lan here: (disables skill saving throughout the game) //
	if (sv_cheats->GetBool())
		m_bFoundCheats = true;

	if (gpGlobals->curtime > m_flLastProfileSystemStatusUpdateCheck)
	{
		bb2_profile_system_status.SetValue(CanStoreSkills());
		m_flLastProfileSystemStatusUpdateCheck = gpGlobals->curtime + 1.0f;
	}

	// PostLoad - When we run a dedicated server we want to re-load the map data when SteamAPI is up running...
	if ((m_flPostLoadTimer > 0) && (m_flPostLoadTimer < engine->Time()))
	{
		m_flPostLoadTimer = 0.0f;

		// If we didn't fetch workshop data, but only regular map data via data/maps/*, load tags and check stuff right away.
		if (GameBaseShared()->GetSharedMapData())
			GameBaseShared()->GetSharedMapData()->FetchMapData();

		if (GameBaseShared()->GetServerWorkshopData())
			GameBaseShared()->GetServerWorkshopData()->Initialize();
	}

	// Check for map changes:
	if (m_bShouldChangeMap && (gpGlobals->curtime > m_flTimeToChangeLevel))
	{
		m_bShouldChangeMap = false;
		engine->ChangeLevel(szNextMap, NULL);
		return;
	}

	if (GameBaseShared()->GetServerWorkshopData())
		GameBaseShared()->GetServerWorkshopData()->DownloadThink();
}

void CGameBaseServer::DoMapChange(const char* map)
{
	Q_strncpy(szNextMap, map, MAX_MAP_NAME);

	IGameEvent* event = gameeventmanager->CreateEvent("changelevel");
	if (event)
	{
		event->SetString("map", szNextMap);
		gameeventmanager->FireEvent(event);
	}

	m_bShouldChangeMap = true;
	m_flTimeToChangeLevel = gpGlobals->curtime + 1.25f; // Wait X sec until changing lvl.
}

inline const char* CGameBaseServer::GetPublicIP(uint32 unIP) const
{
	static char s[4][64];
	static int nBuf = 0;
	unsigned char* ipByte = (unsigned char*)&unIP;
	Q_snprintf(s[nBuf], sizeof(s[nBuf]), "%u.%u.%u.%u", (int)(ipByte[3]), (int)(ipByte[2]), (int)(ipByte[1]), (int)(ipByte[0]));
	const char* pchRet = s[nBuf];
	++nBuf;
	nBuf %= ((sizeof(s) / sizeof(s[0])));
	return pchRet;
}

CON_COMMAND_F(player_vote_map, "Vote for a map.", FCVAR_HIDDEN)
{
	if (args.ArgC() != 2)
		return;

	CHL2MP_Player* pClient = ToHL2MPPlayer(UTIL_GetCommandClient());
	if (!pClient || !HL2MPRules())
		return;

	const char* map = args[1];
	if (!(map && map[0]))
		return;

	HL2MPRules()->CreateMapVote(pClient, map);
}

CON_COMMAND_F(player_vote_kickban, "Vote to kick or ban someone.", FCVAR_HIDDEN)
{
	if (args.ArgC() != 3)
		return;

	int iTarget = atoi(args[1]);
	int iType = atoi(args[2]);

	CHL2MP_Player* pClient = ToHL2MPPlayer(UTIL_GetCommandClient());
	if (!pClient || !HL2MPRules())
		return;

	HL2MPRules()->CreateBanKickVote(pClient, UTIL_PlayerByUserId(iTarget), (iType != 0));
}

static bool IsValidAdmin(CHL2MP_Player* pClient, int mask)
{
	if (!pClient || pClient->IsBot())
		return false;

	if (!pClient->GetAdminLevel())
	{
		ClientPrint(pClient, HUD_PRINTCONSOLE, "You are not an admin on this server!");
		return false;
	}

	if (!pClient->HasAdminLevel(mask))
	{
		ClientPrint(pClient, HUD_PRINTCONSOLE, "You do not have the privilege to execute this command!");
		return false;
	}

	return true;
}

CON_COMMAND(admin_kick_id, "Admin Kick Command")
{
	CHL2MP_Player* pClient = ToHL2MPPlayer(UTIL_GetCommandClient());
	if (!IsValidAdmin(pClient, ADMIN_LEVEL_KICK))
		return;

	if (args.ArgC() < 3)
	{
		ClientPrint(pClient, HUD_PRINTCONSOLE, "Please specify the <userID> and the <reason>!");
		return;
	}

	int userID = atoi(args[1]);
	const char* reason = args.ArgS();
	reason += strlen(args[1]); // strip away userID.

	char pchCommand[80];
	Q_snprintf(pchCommand, 80, "kickid %i \"%s\"\n", userID, reason);
	engine->ServerCommand(pchCommand);
}

CON_COMMAND(admin_ban_id, "Admin Ban Command")
{
	CHL2MP_Player* pClient = ToHL2MPPlayer(UTIL_GetCommandClient());
	if (!IsValidAdmin(pClient, ADMIN_LEVEL_BAN))
		return;

	if (args.ArgC() < 4)
	{
		ClientPrint(pClient, HUD_PRINTCONSOLE, "Please specify the <ban time in minutes> <userID> and the <reason>!");
		return;
	}

	int banTime = atoi(args[1]);
	int userID = atoi(args[2]);
	const char* reason = args.ArgS();
	reason += strlen(args[1]);
	reason += strlen(args[2]);

	char pchCommand[80];

	Q_snprintf(pchCommand, 80, "banid %i %i\n", banTime, userID);
	engine->ServerCommand(pchCommand);
	engine->ServerExecute(); // Add to ban list right away.

	Q_snprintf(pchCommand, 80, "kickid %i \"%s\"\n", userID, reason);
	engine->ServerCommand(pchCommand); // Kick afterwards.

	engine->ServerCommand("writeid;writeip\n"); // Save the new ids already, in case the server crashes b4 level change..
}

CON_COMMAND(admin_changelevel, "Admin Changelevel Command")
{
	CHL2MP_Player* pClient = ToHL2MPPlayer(UTIL_GetCommandClient());
	if (!IsValidAdmin(pClient, ADMIN_LEVEL_MISC))
		return;

	if (args.ArgC() != 2)
	{
		ClientPrint(pClient, HUD_PRINTCONSOLE, "Please specify the <mapName> to change to!");
		return;
	}

	const char* mapName = args[1];

	char pszMapPath[80];
	Q_snprintf(pszMapPath, 80, "maps/%s.bsp", mapName);
	if (!filesystem->FileExists(pszMapPath, "MOD"))
	{
		ClientPrint(pClient, HUD_PRINTCONSOLE, "The map you specified doesn't exist on the server!");
		return;
	}

	if (mapName && mapName[0])
		GameBaseServer()->DoMapChange(mapName);
}

CON_COMMAND(admin_spectate, "Admin Spectate Command")
{
	CHL2MP_Player* pClient = ToHL2MPPlayer(UTIL_GetCommandClient());
	if (!IsValidAdmin(pClient, ADMIN_LEVEL_MISC))
		return;

	if (!HL2MPRules()->IsFastPacedGameplay())
	{
		ClientPrint(pClient, HUD_PRINTCONSOLE, "This command may only be used in Deathmatch & Elimination mode!");
		return;
	}

	pClient->m_bWantsToDeployAsHuman = true;
	pClient->SetRespawnTime(0.0f);
	pClient->HandleCommand_JoinTeam(TEAM_SPECTATOR, true);
	pClient->ForceRespawn();
}