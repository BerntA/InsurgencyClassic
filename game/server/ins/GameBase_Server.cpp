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

ConVar ins_active_workshop_item("ins_active_workshop_item", "0", FCVAR_REPLICATED | FCVAR_HIDDEN, "Fetch the active workshop map item ID running on the server.");

static CGameBaseServer gServerMode;
CGameBaseServer* GameBaseServer()
{
	return &gServerMode;
}

void CGameBaseServer::Init()
{
	m_flPostLoadTimer = 0.0f;
}

void CGameBaseServer::Release()
{
}

// Set current workshop ID regardless of SteamUGC state!
// Use workshop file to find the right ID.
void CGameBaseServer::SetCurrentMapAddon(const char* map)
{
	if (engine->IsDedicatedServer() == false)
	{
		ins_active_workshop_item.SetValue(0);
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

	ins_active_workshop_item.SetValue(pchWorkshopID);
}

void CGameBaseServer::PostInit()
{
}

void CGameBaseServer::OnUpdate(int iClientsInGame)
{
	// PostLoad - When we run a dedicated server we want to re-load the map data when SteamAPI is up running...
	if ((m_flPostLoadTimer > 0) && (m_flPostLoadTimer < engine->Time()))
	{
		m_flPostLoadTimer = 0.0f;

		if (GameBaseShared()->GetSharedMapData())
			GameBaseShared()->GetSharedMapData()->FetchMapData();

		if (GameBaseShared()->GetServerWorkshopData())
			GameBaseShared()->GetServerWorkshopData()->Initialize();
	}

	if (GameBaseShared()->GetServerWorkshopData())
		GameBaseShared()->GetServerWorkshopData()->DownloadThink();
}