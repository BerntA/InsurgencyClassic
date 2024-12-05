//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "steamidcheck.h"
#include "ins_player_shared.h"
#include "ins_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define STEAMID_LOCAL "STEAM_ID_LAN"

//=========================================================
//=========================================================

// TODO: encrypt these inside the DLL

const char *g_pszDeveloperSteamIDs[ ] = {
	"STEAM_0:0:5576382",	// Pongles
	"STEAM_0:1:3957883",	// Jeremy
    "STEAM_0:0:664042",     // xENO_
	NULL
};

const char *g_pszBannedSteamIDs[ ] = {
	"STEAM_0:0:662426",		// Russ
	"STEAM_0:0:136386",		// myg0t person
	"STEAM_0:1:4796928",	// awful teamkiller
	"STEAM_0:0:59695336",	// myg0t person
	"STEAM_0:1:175398",		// myg0t person
	"STEAM_0:1:8960128",	// myg0t person
	"STEAM_0:0:7137429",	// myg0t person
	"STEAM_0:0:776673",		// myg0t person
	"STEAM_0:1:3915798",	// myg0t person
	"STEAM_0:1:1544714",	// myg0t person
	"STEAM_0:1:5076411",	// myg0t person
	NULL
};

const char *g_pszGimpedSteamIDs[ ] = {
	NULL
};

//=========================================================
//=========================================================
int UTIL_FindSteamIDType( CINSPlayer *pPlayer )
{
	const char *pszSteamID = pPlayer->GetNetworkIDString( );

	if( !pszSteamID )
		return STEAMIDTYPE_NORMAL;

	if( Q_strcmp( pszSteamID, STEAMID_LOCAL ) == 0 )
		return STEAMIDTYPE_NORMAL;

	if( UTIL_FindInList( g_pszDeveloperSteamIDs, pszSteamID ) )
		return STEAMIDTYPE_DEVELOPER;

	if( UTIL_FindInList( g_pszBannedSteamIDs, pszSteamID ) )
		return STEAMIDTYPE_BANNED;

	if( UTIL_FindInList( g_pszGimpedSteamIDs, pszSteamID ) )
		return STEAMIDTYPE_GIMPED;

	return STEAMIDTYPE_NORMAL;
}
