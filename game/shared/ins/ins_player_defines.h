//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef INS_PLAYER_DEFINES_H
#define INS_PLAYER_DEFINES_H

#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#define FL_PLAYER_IRONSIGHTS	(1<<0)
#define FL_PLAYER_BIPOD			(1<<1)
#define FL_PLAYER_SPRINTING		(1<<2)
#define FL_PLAYER_WALKING		(1<<3)

#define FL_PLAYER_BITS			4

//=========================================================
//=========================================================
#define PLAYER_JUMPTIME_MAX 2.25f
#define PLAYER_JUMPTIME 0.85f

//=========================================================
//=========================================================
#define PLAYER_MAXSPEED_WALK 166
#define PLAYER_MAXSPEED_SPRINT 205
#define PLAYER_MAXSPEED_BURST 264

//=========================================================
//=========================================================

// PNOTE: due to the logic in ins_gamemovement, the grouped values *must* increase per define

#define PLAYER_FRACSPEED_STAND_IRONSIGHT 0.65f
#define PLAYER_FRACSPEED_STAND_SHUFFLE 0.75f
#define PLAYER_FRACSPEED_STAND_RELOADING 0.79f

#define PLAYER_FRACSPEED_CROUCHED_IRONSIGHT 0.35f
#define PLAYER_FRACSPEED_CROUCHED 0.5f

#define PLAYER_FRACSPEED_PRONE 0.35f

#define PLAYER_FRACSPEED_DMGDECAY 0.6f

//=========================================================
//=========================================================
#define PLAYER_VOFFSET_CIRONSIGHT 3.0f

//=========================================================
//=========================================================
enum HealthType_t
{
	HEALTHTYPE_UNINJURED = 0,
	HEALTHTYPE_FINE,
	HEALTHTYPE_INJURED,
	HEALTHTYPE_SERIOUS,
	HEALTHTYPE_COUNT
};

//=========================================================
//=========================================================
#define STIME_STAND_TO_CROUCH  0.15f
#define STIME_CROUCH_TO_PRONE  0.3f
#define STIME_CROUCH_TO_STAND  STIME_STAND_TO_CROUCH
#define STIME_PRONE_TO_CROUCH  STIME_CROUCH_TO_PRONE

#define PTIME_STAND 1.35f
#define PTIME_CROUCH 0.85f

//=========================================================
//=========================================================
enum PlayerActionsTypes_t
{
	INVALID_PACTION = -1,
	PACTION_RELOADING = 0,
	PACTION_HIT,
	PACTION_OUTOFAMMO,
	PACTION_FRAGOUT,
	PACTION_MANDOWN,
	PACTION_COUNT
};

//=========================================================
//=========================================================
#define PLAYER_BANDAGE_LENGTH_FWD 32.0f
#define PLAYER_BANDAGE_LENGTH_BWD -10.0f

#define PLAYER_BANDAGE_STARTTICK 1.0
#define PLAYER_BANDAGE_TICK 0.5

#define PLAYER_BANDAGE_TIME 4.0f
#define PLAYER_BANDAGE_MAXHEALTH 30

#define PLAYER_BANDAGE_MAX 2

//=========================================================
//=========================================================
#define PLAYER_FIXEDFADE 0.5f
#define PLAYER_DEATHFADEOUT DEATH_ANIMATION_TIME * 0.5
#define PLAYER_DEATHFADEIN_QUICK 0.5f
#define PLAYER_DEATHFADEIN 1.5f

//=========================================================
//=========================================================
enum PlayerDeathType_t
{
	PDEATHTYPE_SELF = 0,
	PDEATHTYPE_KIA,
	PDEATHTYPE_FF,
	PDEATHTYPE_SOULSTOLEN,
	PDEATHTYPE_TELEFRAG
};

//=========================================================
//=========================================================
enum PlayerStatusType_t
{
	INVALID_PSTATUSTYPE = -1,
	PSTATUSTYPE_ORDER_OBJ = 0,
	PSTATUSTYPE_ORDER_UNIT,
	PSTATUSTYPE_ORDER_PLAYER,
	PSTATUSTYPE_OBJECTIVE,
	PSTATUSTYPE_COUNT
};

#define INVALID_PSTATUSID -1

#define MAX_PSTATUSTYPE_BITS 3
#define MAX_PSTATUSID_BITS 4

//=========================================================
//=========================================================
enum StatusObjState_t
{
	PSTATUS_OBJ_CAPTURING = 0,
	PSTATUS_OBJ_DEFENDING,
	PSTATUS_OBJ_COUNT
};

//=========================================================
//=========================================================
#define PLAYER_JUMP_HEIGHT 52.0f

//=========================================================
//=========================================================
#define STAMINA_MAX 100
#define STAMINA_MAX_BITS 7

#define STAMINA_NEEDED_BURST 75
#define STAMINA_LENGTH_BURST 10.0f

#define STAMINA_UPDATE_THRESHOLD 0.5f

#define STAMINA_REGAIN 9
#define STAMINA_REGAIN_FACTOR_STAND 0.25f
#define STAMINA_REGAIN_FACTOR_CROUCH 0.5f
#define STAMINA_REGAIN_FACTOR_PRONE 1.0f

#define STAMINA_TAKE 2

#define STAMINA_JUMP_COST 15
#define STAMINA_JUMP_WAIT 1.25f

#define STAMINA_LOW 35
#define STAMINA_HIGH 85

//=========================================================
//=========================================================
#define MAX_SHAKE_RADIUS 6.0f
#define MAX_SHAKE_XYSIZE 3.0f

//=========================================================
//=========================================================
#define PLAYER_HEADTIME_HIT 20.0f 
#define PLAYER_HEADTIME_SHOT 8.0f 

//=========================================================
//=========================================================
#define PLAYER_TALK_INTERVAL 0.66f

//=========================================================
//=========================================================
#define PLAYER_NAME_INTERVAL 1.0f

//=========================================================
//=========================================================
#define VMHIT_DAMAGELIMIT 35

//=========================================================
//=========================================================

// TODO: move into same stats .h
#ifdef GAME_DLL

class CGamePlayerStats
{
public:
	CGamePlayerStats( );

	CGamePlayerStats &operator=( const CGamePlayerStats &Src );

	void Reset( void );

	void Increment( int iType, int iValue );

public:
	int m_iGamePoints;
	int m_iKillPoints, m_iKills, m_iFriendlyKills;
	int m_iDeaths;
};

#endif

#endif // INS_PLAYER_DEFINES_H