//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_GAMERULES_H
#define INS_GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#include "gamerules.h"
#include "ins_shared.h"

//=========================================================
//=========================================================
enum GRModes_t
{
	GRMODE_NONE = -1,
	GRMODE_IDLE = 0,
	GRMODE_SQUAD,
	GRMODE_RUNNING,
	GRMODE_COUNT
};

extern const char *g_pszGRModes[ GRMODE_COUNT ];

//=========================================================
//=========================================================
enum GameModeWarnings_t
{
	// IMC
	GRWARN_IMC_INVALIDFILE = 1,
	GRWARN_IMC_INVALIDFORMAT,
	GRWARN_IMC_INVALIDTEAM,
	GRWARN_IMC_MISSINGDATA,
	GRWARN_IMC_INVALIDGLOBALOBJ,
	GRWAWN_IMC_NOOBJECTIVES,
	GRWARN_IMC_NOPROFILES,
	GRWARN_IMC_INVALIDPROFILE,
	GRWARN_IMC_BOUNDS,
	GRWARN_IMC_NOSQUADS,
	GRWARN_IMC_INVALIDSQUAD,
	GRWARN_IMC_NOSLOTS,
	GRWARN_IMC_INVALIDOBJECTIVES,
	GRWARN_IMC_INVALIDCACHES,
	GRWARN_IMC_NONMOVINGFALLBACK,

	// Generic
	GMWARN_GENERAL_NOTENOUGHOBJS,
	GMWARN_GENERAL_INVALID_TEAM_STARTS,
	GMWARN_GENERAL_INVALIDOBJS,
	GMWARN_GENERAL_FAILEDSETUP,
	GMWARN_GENERAL_INVALIDCACHE,

	// Fire-Fight
	GMWARN_FF_NO_HIDDEN_OBJS,
	GMWARN_FF_INCORRECT_HIDDEN,
	GMWARN_FF_NEUTRAL_HIDDEN_OBJS,
	GMWARN_FF_BAD_HIDDEN_OBJ_TEAMS,

	// Push
	GMWARN_PUSH_NO_HIDDEN_OBJ,
	GMWARN_PUSH_BOTH_HIDDEN,
	GMWARN_PUSH_NEUTRAL_OBJS,
	GMWARN_PUSH_BAD_HIDDEN_OBJ_TEAMS,
	GMWARN_PUSH_MISPLACED_HIDDEN_OBJ,
	GMWARN_PUSH_BAD_DEFENDER_TEAM,

	// PowerBall
	GMWARN_PB_MOVINGSPAWNS,
	GMWARN_PB_HIDDENOBJS,
	GMWARN_PB_TEAMNEUTRAL,
	GMWARN_PB_TEAMSAME,
	GMWARN_PB_NOPOWERBALL,

	MAX_OFFSET_GMWARNS
};

#define MAX_GMWARNS MAX_OFFSET_GMWARNS - 1

//=========================================================
//=========================================================
enum TeamSelect_t
{
	TEAMSELECT_INVALID = -1,
	TEAMSELECT_ONE = 0,
	TEAMSELECT_TWO,
	TEAMSELECT_AUTOASSIGN,
	TEAMSELECT_SPECTATOR,
	TEAMSELECT_COUNT
};

//=========================================================
//=========================================================
enum SquadOrderTypes_t
{
	SQUADORDER_NONE = 0,		// doesn't change squads
	SQUADORDER_FIRSTROUND,		// reorders at the first round
	SQUADORDER_ALWAYS			// always reorder
};

//=========================================================
//=========================================================
enum EndGame_WinConditions_t
{
	ENDGAME_WINCONDITION_OBJ = 0,
	ENDGAME_WINCONDITION_DEATH,
	ENDGAME_WINCONDITION_TIMER,
	ENDGAME_WINCONDITION_EXTENDED,
	ENDGAME_WINCONDITION_CUSTOM,
	ENDGAME_WINCONDITION_COUNT
};

enum EndGame_String_Types_t
{
	ENDGAME_STRINGTYPES_ROUNDLENGTH = 0,
	ENDGAME_STRINGTYPES_BALANCE,
	ENDGAME_STRINGTYPES_COUNT
};

#define ENDGAME_STRINGTYPES_GROUP_COUNT 3

enum EndGame_RoundLength_t
{
	ENDGAME_ROUNDLENGTH_QUICK = 0,
	ENDGAME_ROUNDLENGTH_MEDIUM,
	ENDGAME_ROUNDLENGTH_LONG
};

#define ENDGAME_ROUNDLENGTH_INVALID (ENDGAME_ROUNDLENGTH_LONG + 1)

enum EndGame_Balance_t
{
	ENDGAME_BALANCE_BALANCED = 0,
	ENDGAME_BALANCE_SUPERIOR,
	ENDGAME_BALANCE_OWNED
};

//=========================================================
//=========================================================
#define GRCMD_TEAMSETUP			"pteamsetup"
#define GRCMD_SQUADSETUP		"psquadsetup"
#define GRCMD_FULLSETUP			"pfullsetup"
#define GRCMD_INVALIDSCRIPTS	"invalidscripts"
#define GRCMD_REGDI				"regdi"
#define GRCMD_MODIFYLAYOUT		"modifylayout"
#define GRCMD_CREINFORCE		"creinforce"
#define GRCMD_OBJORDERS			"objorders"
#define GRCMD_UNITORDERS		"unitorders"
#define GRCMD_PLAYERORDERS		"playerorders"
#define GRCMD_PLAYERSTATUS		"playerstatus"
#define GRCMD_VOICECHAT			"vchat"
#define GRCMD_REGLEADER			"regleader"
#define GRCMD_CLANREADY			"clanready"
#define GRCMD_ACCEPTPROMO		"acceptpromo"
#define GRCMD_DECLINEPROMO		"declinepromo"

//=========================================================
//=========================================================

// see http://forums.insmod.net/index.php?showtopic=370
//All players
#define MORALE_OBJECTIVE_FOLLOWING_ORDERS 5
#define MORALE_OBJECTIVE_LONE_WOLF 2
#define MORALE_OBJECTIVE_PUSH_FOLLOWING_ORDERS 10
#define MORALE_OBJECTIVE_PUSH_LONE_WOLF 5
#define MORALE_VICTORY 5
#define MORALE_TEAM_KILL (-5)
#define MORALE_KILL 1
#define MORALE_MOST_SKILLED 5
#define MORALE_BEST_SQUAD 2

//ES/NCO bonuses
#define MORALE_FOLLOWING_LEADER 1
#define MORALE_CALL_REINFORCEMENTS 1
#define MORALE_OBJECTIVE_LEADERSHIP 1
#define MORALE_ORDERS_ISSUED 1

//=========================================================
//=========================================================

#ifdef CLIENT_DLL

#define CINSRulesProxy C_INSRulesProxy

#endif

class CINSRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CINSRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS( );
};

//=========================================================
//=========================================================
extern ConVar allowspectators;
extern ConVar friendlyfire;
extern ConVar autoteambalance;
extern ConVar limitteams;
extern ConVar locksquads;
extern ConVar chattime;
extern ConVar firetype;
extern ConVar motdmsg;
extern ConVar ventrequired;
extern ConVar decalfrequency;

//=========================================================
//=========================================================
#include "imc_format.h"

#include "ins_gamerules_squad_shared.h"
#include "ins_gamerules_running_shared.h"


#ifdef GAME_DLL

#include "ins_gamerules_server.h"

#include "ins_gamerules_idle.h"
#include "ins_gamerules_squad.h"
#include "ins_gamerules_running.h"

#else

#include "c_ins_gamerules_client.h"

#endif

//=========================================================
//=========================================================
class CModeBase;

typedef CModeBase *( *ModeHelper_t )( void );
extern ModeHelper_t g_ModeHelper[ GRMODE_COUNT ];

class CModeHelper
{
public:
	CModeHelper( int iMode, ModeHelper_t ModeHelper )
	{
		g_ModeHelper[ iMode ] = ModeHelper;
	}
};

#define DECLARE_MODEMANAGER( mode, className ) \
	static CModeBase *CreateMode__##className( void ) { \
		return new className; \
	} \
	CModeHelper g_ModeManager__##className( mode, CreateMode__##className );

#endif // INS_GAMERULES_H
