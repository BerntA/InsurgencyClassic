//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: Handle map specific data on the server & client. @UGC stuff.
//
//========================================================================================//

#ifndef GAME_DEFINITIONS_MAPDATA_H
#define GAME_DEFINITIONS_MAPDATA_H

#ifdef _WIN32
#pragma once
#endif

#include <steam/steam_api.h>

#define WORKSHOP_KV_PAIR_SIZE 96

struct gameMapItem_t
{
	char pszMapName[MAX_MAP_NAME];
	PublishedFileId_t workshopID;
};

class CGameDefinitionsMapData
{
public:
	CGameDefinitionsMapData();
	~CGameDefinitionsMapData();

	bool FetchMapData(void);
	bool QueryUGC(int page = 1);
	gameMapItem_t* GetMapData(const char* pszMap);
	int GetMapIndex(const char* pszMap);

#ifndef CLIENT_DLL
	CUtlVector<PublishedFileId_t> pszWorkshopItemList;
#endif

	void OnReceiveUGCQueryResultsAll(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure);
	UGCQueryHandle_t queryHandler;
	CCallResult< CGameDefinitionsMapData, SteamUGCQueryCompleted_t > m_SteamCallResultUGCQuery;

	CUtlVector<gameMapItem_t> pszGameMaps;

protected:
	int m_iMatchingItems;
	int m_iCurrentPage;
	bool m_bSatRequestInfo;
};

#endif // GAME_DEFINITIONS_MAPDATA_H