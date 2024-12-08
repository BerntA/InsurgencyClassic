//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: Handle map specific data on the server & client. @UGC stuff.
//
//========================================================================================//

#include "cbase.h"
#include "GameDefinitions_MapData.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "GameBase_Shared.h"

#ifndef CLIENT_DLL
#include "gameinterface.h"
#include "GameBase_Server.h"

#define STEAM_API_INTERFACE steamgameserverapicontext
#else
#include "vgui/ILocalize.h"
#include "vgui/VGUI.h"

#define STEAM_API_INTERFACE steamapicontext
#endif

CGameDefinitionsMapData::CGameDefinitionsMapData()
{
	queryHandler = NULL;
	FetchMapData();
}

CGameDefinitionsMapData::~CGameDefinitionsMapData()
{
	pszGameMaps.Purge();
#ifndef CLIENT_DLL
	pszWorkshopItemList.Purge();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Load official maps, custom maps and workshop maps. Parse data.
//-----------------------------------------------------------------------------
bool CGameDefinitionsMapData::FetchMapData(void)
{
	pszGameMaps.Purge();
#ifndef CLIENT_DLL
	pszWorkshopItemList.Purge();
#endif

	m_iMatchingItems = 0;
	m_iCurrentPage = 1;
	m_bSatRequestInfo = false;

#ifndef CLIENT_DLL
	if (GameBaseShared() && GameBaseShared()->GetServerWorkshopData())
		GameBaseShared()->GetServerWorkshopData()->GetListOfAddons(pszWorkshopItemList, true);

	m_iMatchingItems = pszWorkshopItemList.Count();
	m_bSatRequestInfo = true;
#endif

	return QueryUGC(m_iCurrentPage);
}

bool CGameDefinitionsMapData::QueryUGC(int page)
{
#ifndef CLIENT_DLL
	if (!STEAM_API_INTERFACE || (STEAM_API_INTERFACE && !STEAM_API_INTERFACE->SteamUGC()) || !engine->IsDedicatedServer())
#else
	if (!STEAM_API_INTERFACE || (STEAM_API_INTERFACE && !STEAM_API_INTERFACE->SteamUGC()))
#endif
	{
#ifdef CLIENT_DLL
		Warning("Client SteamAPI Interface is unavailable!\n");
#else
		Warning("Server SteamAPI Interface is unavailable!\n");
#endif
		return false;
	}

	if (queryHandler)
	{
		STEAM_API_INTERFACE->SteamUGC()->ReleaseQueryUGCRequest(queryHandler);
		queryHandler = NULL;
	}

#ifdef CLIENT_DLL
	AccountID_t steamAccountID = STEAM_API_INTERFACE->SteamUser()->GetSteamID().GetAccountID();
	queryHandler = STEAM_API_INTERFACE->SteamUGC()->CreateQueryUserUGCRequest(steamAccountID, k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_Items, k_EUserUGCListSortOrder_TitleAsc, (AppId_t)382990, (AppId_t)346330, page);
#else
	page--;
	if ((pszWorkshopItemList.Count() <= 0) || (pszWorkshopItemList.Count() <= (page * 50)))
		return false;

	queryHandler = STEAM_API_INTERFACE->SteamUGC()->CreateQueryUGCDetailsRequest(&pszWorkshopItemList[(page * 50)], pszWorkshopItemList.Count());
#endif

	STEAM_API_INTERFACE->SteamUGC()->SetReturnKeyValueTags(queryHandler, true);
	SteamAPICall_t apiCallback = STEAM_API_INTERFACE->SteamUGC()->SendQueryUGCRequest(queryHandler);
	m_SteamCallResultUGCQuery.Set(apiCallback, this, &CGameDefinitionsMapData::OnReceiveUGCQueryResultsAll);

	return true;
}

gameMapItem_t* CGameDefinitionsMapData::GetMapData(const char* pszMap)
{
	int index = GetMapIndex(pszMap);
	return ((index != -1) ? &pszGameMaps[index] : NULL);
}

int CGameDefinitionsMapData::GetMapIndex(const char* pszMap)
{
	for (int i = 0; i < pszGameMaps.Count(); i++)
	{
		if (!strcmp(pszGameMaps[i].pszMapName, pszMap))
			return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Fetch workshop items and their details
//-----------------------------------------------------------------------------
void CGameDefinitionsMapData::OnReceiveUGCQueryResultsAll(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure)
{
	if (!m_bSatRequestInfo)
	{
		m_iMatchingItems = pCallback->m_unTotalMatchingResults;
		m_bSatRequestInfo = true;
		Msg("Loaded %i workshop addons.\n", pCallback->m_unTotalMatchingResults);
	}

	char mapTagKey[WORKSHOP_KV_PAIR_SIZE], mapTagValue[WORKSHOP_KV_PAIR_SIZE];

	for (uint32 i = 0; i < pCallback->m_unNumResultsReturned; ++i)
	{
		SteamUGCDetails_t WorkshopItem;
		if (!STEAM_API_INTERFACE->SteamUGC()->GetQueryUGCResult(pCallback->m_handle, i, &WorkshopItem))
			continue;

		uint32 uAmountOfKeys = STEAM_API_INTERFACE->SteamUGC()->GetQueryUGCNumKeyValueTags(pCallback->m_handle, i);
		if (uAmountOfKeys == 0) continue;

		// Scan for map_name keys, populate vector list:
		for (uint32 items = 0; items < uAmountOfKeys; items++)
		{
			STEAM_API_INTERFACE->SteamUGC()->GetQueryUGCKeyValueTag(pCallback->m_handle, i, items, mapTagKey, WORKSHOP_KV_PAIR_SIZE, mapTagValue, WORKSHOP_KV_PAIR_SIZE);
			if (strcmp(mapTagKey, "map_name") != 0) continue;

			const int iExistingMapIndex = GetMapIndex(mapTagValue);

			if (iExistingMapIndex != -1)
				pszGameMaps[iExistingMapIndex].workshopID = WorkshopItem.m_nPublishedFileId;
			else
			{
				gameMapItem_t mapItem;
				mapItem.workshopID = WorkshopItem.m_nPublishedFileId;
				Q_strncpy(mapItem.pszMapName, mapTagValue, MAX_MAP_NAME);
				pszGameMaps.AddToTail(mapItem);
			}
		}
	}

	// Cleanup
	if (queryHandler)
	{
		STEAM_API_INTERFACE->SteamUGC()->ReleaseQueryUGCRequest(queryHandler);
		queryHandler = NULL;
	}

	m_iMatchingItems -= pCallback->m_unNumResultsReturned;
	if (m_iMatchingItems > 0)
	{
		m_iCurrentPage++;
		QueryUGC(m_iCurrentPage);
		return;
	}

	// TODO - callback when done, if necessary.
}