//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef IMC_LOADER_K_H
#define IMC_LOADER_K_H

#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
DECLARE_STRING_LOOKUP_CONSTANTS( int, theatertypes )
#define GetTheaterType( k, j ) STRING_LOOKUP( theatertypes, k, j )

DECLARE_STRING_LOOKUP_CONSTANTS( int, teamclass )
#define GetTeamClass( k, j ) STRING_LOOKUP( teamclass, k, j )

DECLARE_STRING_LOOKUP_CONSTANTS( int, gamemodes )
#define GetGameMode( k, j) STRING_LOOKUP(gamemodes, k, j )

DECLARE_STRING_LOOKUP_CONSTANTS( Color, indicolors )
#define GetIMCColor( k, j ) STRING_LOOKUP( indicolors, k, j )

#ifdef GAME_DLL

DECLARE_STRING_LOOKUP_CONSTANTS(int, teamstart )
#define GetTeamStart( k, j ) STRING_LOOKUP( teamstart, k, j )

#endif

//=========================================================
//=========================================================
#define MAP_VERSION		"version"
#define MAP_NAME		"mapname"
#define MAP_THEATER		"theater"
#define MAP_GRAVITY		"gravity"
#define MAP_VGUINAME	"vmapname"
#define MAP_OVERVIEW	"mapoverview"
#define MAP_OPDATE		"timestamp"
#define MAP_NCOLORF		"mapncolorf"
#define MAP_NCOLORB		"mapncolorb"
#define MAP_AMAPSUPPORT	"amapsupport"

#define TEAM_SQUADS		"Squads"
#define TEAM_NAME		"name"
#define TEAM_BRIEFING	"briefing"

#define SQUADS_DEFINED	"Squad"
#define SQUADS_DATA		"SquadData"

#define SLOT_DATA		"SlotData"
extern const char *g_pszSquadSlotNames[ MAX_SQUAD_SLOTS ];

#define WEAPONCACHE_DEFINED "WeaponCache"
#define WEAPONCACHE_DATA	"WeaponCacheData"

#define INVENTORYSTORE_DATA "InventoryStoreData"

#define TEAM_TYPE		"type"
#define TEAM_NUMWAVES	"numwaves"
#define TEAM_TIMEWAVE	"timewave"
#define TEAM_EWAVES		"ewaves"
#define TEAM_OBJWAVES	"objwaves"

#define OBJ_PHONETIC	"phonetic"
#define OBJ_COLOR		"color"

#define PROFILE_GAMETYPE			"gametype"
#define PROFILE_ROUNDLENGTH			"roundlength"
#define PROFILE_MOVINGSPAWNS		"movingspawns"
#define PROFILE_CAPTURINGFALLBACK	"capturingfallback"
#define PROFILE_CORRECTION			"colorcorrection"

#define PROFILE_OBJECTIVEDATA		"ObjectiveData"
#define PROFILE_WEAPONCACHEDATA		"WeaponCacheData"
#define PROFILE_TEAMONE				"TeamOne"
#define PROFILE_TEAMTWO				"TeamTwo"

#define PROFILE_OBJ_SPAWNGROUP					"spawngroup"
#define PROFILE_OBJ_SPAWNGROUP_REINFORCEMENT	"rspawngroup"
#define PROFILE_OBJ_REQPERCENT					"reqpercent"
#define PROFILE_OBJ_CAPTIME						"captime"
#define PROFILE_OBJ_INVISTIME					"invistime"
#define PROFILE_OBJ_TEAMSTART					"teamstart"
#define PROFILE_OBJ_STARTSPAWN					"startspawn"
#define PROFILE_OBJ_HIDDEN						"hidden"
#define PROFILE_OBJ_RSPAWNS						"reinforcementspawns"
#define PROFILE_OBJ_KOTH						"koth"
#define PROFILE_OBJ_MIXEDSPAWNS					"mixedspawns"

#define PROFILE_WINC_TEAMONE	"teamone"
#define PROFILE_WINC_TEAMTWO	"teamtwo"

#define WCACHE_PARENTOBJ		"parentobj"
#define WCACHE_NORESPAWN		"norespawn"
#define WCACHE_SPAWNPARENT		"spawnparent"
#define WCACHE_DEPLOYSTOCK		"deploystock"
#define WCACHE_TEAMONESPAWN		"teamonespawn"
#define WCACHE_TEAMTWOSPAWN		"teamtwospawn"
#define WCACHE_RANDOM			"randomspawn"
#define WCACHE_RANDOM_MIN		"minrandom"
#define WCACHE_RANDOM_MAX		"maxrandom"

#endif // IMC_LOADER_B_H
