//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef IMC_FORMAT_H
#define IMC_FORMAT_H

#include "stringlookup.h"
#include "color.h"

//=========================================================
//=========================================================
#define ALPHABET_SIZE 26

#define UNLIMITED_SUPPLIES -1

//=========================================================
//=========================================================
enum TheaterType_t
{
	THEATERTYPE_UNKNOWN = 0,
	THEATERTYPE_IRAQ,
	THEATERTYPE_AFGHANISTAN,
	THEATERTYPE_COUNT
};

//=========================================================
//=========================================================
enum TeamClasses_t
{
	TEAM_USMC = 0,
	TEAM_IRAQI,
	MAX_LOOKUP_TEAMS
};

// NOTE: UPDATE WEAPONCACHE BITFIELD CODE IF YOU CHANGE ABOVE ENUM

enum TeamStarts_t
{
	TEAMSTART_ONE = 0,
	TEAMSTART_TWO,
	TEAMSTART_NEUTRAL
};

//=========================================================
//=========================================================
enum GameTypes_t
{
	GAMETYPE_INVALID = -1,

	// Offical
	GAMETYPE_BATTLE = 0,
	GAMETYPE_FIREFIGHT,
	GAMETYPE_PUSH,
	GAMETYPE_STRIKE,

	// Other
	GAMETYPE_TDM,
	GAMETYPE_POWERBALL,

	MAX_GAMETYPES
};

extern const char *g_pszGameTypes[ MAX_GAMETYPES ];

//=========================================================
//=========================================================
#define INVALID_OBJECTIVE -1
#define MAX_OBJECTIVES ALPHABET_SIZE
#define MAX_OBJECTIVES_BITS 5

//=========================================================
//=========================================================
enum Squads_t
{
	INVALID_SQUAD = -1,
	SQUAD_ONE = 0,
	SQUAD_TWO,
	MAX_SQUADS
};

// NOTE: if anymore squads are added remember to check g_pszDefaultNames

#define DISABLED_SQUAD INVALID_SQUAD	// A Disabled Squad

#define MAX_SQUAD_SLOTS 8
#define INVALID_SLOT INVALID_SQUAD	// An Invalid Slot in a Squad
#define DEFAULT_SLOT 0

#define MAX_TEAM_SLOTS ( MAX_SQUAD_SLOTS * MAX_SQUADS )

typedef int SlotData_t[ MAX_SQUAD_SLOTS ];

//=========================================================
//=========================================================
#define INVALID_CLASS -1

//=========================================================
//=========================================================
#define MAX_IMC_PROFILES 8

#define INVALID_PROFILE -1

#define PROFILE_OBJ_FLAG_STARTINGSPAWN			(1<<0)
#define PROFILE_OBJ_FLAG_HIDDEN					(1<<1)
#define PROFILE_OBJ_FLAG_TEAMSTART_ONE			(1<<2)
#define PROFILE_OBJ_FLAG_TEAMSTART_TWO			(1<<3)
#define PROFILE_OBJ_FLAG_TEAMSTART_NEUTRAL		(1<<4)
#define PROFILE_OBJ_FLAG_HASREINFORCEMENTS		(1<<5)
#define PROFILE_OBJ_FLAG_KOTH					(1<<6)
#define PROFILE_OBJ_FLAG_MIXEDSPAWNS			(1<<7)

//=========================================================
//=========================================================
#define IMCCOLOR_BLACK	Color( 0, 0, 0 )
#define IMCCOLOR_WHITE	Color( 255, 255, 255 )
#define IMCCOLOR_RED	Color( 255, 0, 0 )
#define IMCCOLOR_BLUE	Color( 0, 0, 255 )
#define IMCCOLOR_GREEN	Color( 0, 255, 0 )
#define IMCCOLOR_YELLOW	Color( 255, 255, 0 )
#define IMCCOLOR_PURPLE	Color( 255, 0, 255 )
#define IMCCOLOR_GREY	Color( 128, 128, 128 )
#define IMCCOLOR_ORANGE	Color( 255, 120, 30 )

#define IMC_DEFAULT_FCOLOR IMCCOLOR_WHITE
#define IMC_DEFAULT_BCOLOR IMCCOLOR_BLACK

//=========================================================
//=========================================================
#define WCACHE_NOPARENTOBJ -1

#define WCACHE_FLAG_NORESPAWN		(1<<0)		// never respawn once destroyed
#define WCACHE_FLAG_SPAWNPARENT		(1<<1)		// never spawn unless parent has a team assigned
#define WCACHE_FLAG_DEPLOYSTOCK		(1<<2)		// redeploy stock when a reinforcement is called in
#define WCACHE_FLAG_TEAMONESPAWN	(1<<3)		// only spawn when team one owns it
#define WCACHE_FLAG_TEAMTWOSPAWN	(1<<4)		// only spawn when team two owns it

//=========================================================
//=========================================================
#define MAX_MAPNAME_LENGTH 256
#define MAX_MAPOVERVIEW_LENGTH 8192
#define MAX_MAPBRIEFING_LENGTH MAX_MAPOVERVIEW_LENGTH
#define MAX_TEAMNAME_LENGTH 64
#define MAX_COLORCORRECTION_LENGTH 64

#define MAX_OBJNAME_LENGTH 32

#define MIN_WAVES 0
#define MAX_WAVES 500
#define MAX_TIMEWAVE 3800
#define MIN_TIMEWAVE 5
#define MIN_EWAVES 0
#define MAX_EWAVES MAX_WAVES
#define MAX_SQUADNAME_LENGTH 128

#define MAX_WEAPONDATA_LENGTH 64

#define MAX_PROFILENAME_LENGTH MAX_OBJNAME_LENGTH

#define MIN_ROUNDLENGTH 15
#define MAX_ROUNDLENGTH 60000

#define MAX_OBJ_REQPERCENT 100 
#define MIN_OBJ_CAPTIME	1
#define MAX_OBJ_CAPTIME	300
#define MIN_OBJ_INVTIME	0
#define MAX_OBJ_INVTIME	30

#endif // IMC_FORMAT_H
