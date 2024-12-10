//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_SHARED_H
#define INS_SHARED_H

#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
enum ShellTypes_t
{
	INS_SHELL_PISTOL = 0,
	INS_SHELL_RIFLE,
	INS_SHELL_SHOTGUN,
};

//=========================================================
//=========================================================
#define MAX_CHATBUFFER_LENGTH 256
#define MAX_CHATMSG_LENGTH 92

enum SayTypes_t
{
	SAYTYPE_INVALID = -1,
	SAYTYPE_SERVER = 0,
	SAYTYPE_GLOBAL,
	SAYTYPE_TEAM,
	SAYTYPE_SQUAD,
	SAYTYPE_COUNT
};

//=========================================================
//=========================================================
enum FireTypes_t
{
	FIRETYPE_ALWAYS = 0,	// can always fire
	FIRETYPE_WARMUP,		// can't fire during warmup
	FIRETYPE_NODAMAGE,		// can fire, but never take any damage
	FIRETYPE_NEVER			// can't ever fire
};

//=========================================================
//=========================================================
enum DeathInfoTypes_t
{
	DEATHINFOTYPE_SHOW_BLACK = 0,	// show the menu
	DEATHINFOTYPE_SHOW,				// show the menu but keep a black screen until closed
	DEATHINFOTYPE_NONE,				// don't show anything
	DEATHINFOTYPE_COUNT
};

//=========================================================
//=========================================================

// PNOTE: radius (so half of hull) with a little extra
#define SPAWNCOLLIDE_DISTANCE 12

//=========================================================
//=========================================================
enum InflictorTypes_t
{
	INVALID_INFLICTORTYPE = -1,

	INFLICTORTYPE_WEAPON = 0,
	INFLICTORTYPE_GRENADE,
	INFLICTORTYPE_MISSILE,

	INFLICTOR_COUNT
};

#define INVALID_INFLICTORID -1

//=========================================================
//=========================================================
#define INVALID_FIREMODE -1

enum FireModes_t
{
	FIREMODE_SEMI = 0,
	FIREMODE_3RNDBURST,
	FIREMODE_FULLAUTO,
	FIREMODE_COUNT
};

#endif // INS_SHARED_H
