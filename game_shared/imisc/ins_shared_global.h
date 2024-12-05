//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Definitions that are shared by the game DLL and the client DLL.
//
// $NoKeywords: $
//=============================================================================//

#ifndef INS_SHAREDDEFS_H
#define INS_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#include "stats_global.h"

//=========================================================
//=========================================================
#define INS_DEFAULT_GRAVITY "800"

//=========================================================
//=========================================================
#define TEAM_NEUTRAL TEAM_UNASSIGNED

#define MAX_PLAY_TEAMS 2
#define MAX_VIEW_TEAMS 2

//=========================================================
//=========================================================
#define INS_HOSTNAME_STRING "hostname"
#define INS_HOSTNAME_LENGTH 1024

#define MOTD_STRING "motd"
#define MOTD_LENGTH 2048
#define MAX_MOTDMSG_LENGTH 128

//=========================================================
//=========================================================
enum DeveloperModes_t
{
	DEVMODE_OFF = 0,	// you suck
	DEVMODE_ON_SILENT,	// i'm a developer, but I'm not going to tell you!
	DEVMODE_ON			// I'M A DEVELOPER - KISS MY SHOES UNDERLING!!!
};

//=========================================================
//=========================================================
#define OBS_ALLOWTARGETS_ALL			0	// allow all targets
#define OBS_ALLOWTARGETS_TEAM			1	// allow only his team (disallow OBS_ALLOWMODE_ALL)
#define OBS_ALLOWTARGETS_SQUAD			2	// allow only own team (disallow OBS_ALLOWMODE_ALL)

#define OBS_ALLOWMODE_ALL				0   // allow all modes
#define OBS_ALLOWMODE_INEYECHASE		1   // allow OBS_MODE_IN_EYE and OBS_MODE_CHASE
#define OBS_ALLOWMODE_INEYE				2   // allow OBS_MODE_IN_EYE only
#define OBS_ALLOWMODE_NONE				3	// allow no-one (force to fixed and doesn't fade in dead)

//=========================================================
//=========================================================
enum Stance_t
{
	STANCE_INVALID = -1,
	STANCE_STAND = 0,
	STANCE_CROUCH,
	STANCE_PRONE,
	STANCE_COUNT
};

//=========================================================
//=========================================================
#define INS_WEAPONFOV_DEFAULT 54

//=========================================================
//=========================================================
#define DECLARE_BASECLASS( baseClassName ) \
	typedef baseClassName BaseClass;

//=========================================================
//=========================================================
extern const unsigned char *GetEncryptionKey( void );

#endif // INS_SHAREDDEFS_H