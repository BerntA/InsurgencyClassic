//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef INS_PLAYER_SHARED_H
#define INS_PLAYER_SHARED_H

#ifdef _WIN32
#pragma once
#endif

#include "studio.h"
#include "ammodef.h"

//=========================================================
//=========================================================
#ifdef GAME_DLL

class CINSPlayer;

#else

class C_INSPlayer;

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

class C_BaseAnimatingOverlay;
#define CBaseAnimatingOverlay C_BaseAnimatingOverlay
#define CINSPlayer C_INSPlayer

#endif

//=========================================================
//=========================================================
#define DAMAGE_UPDATE_TIME 0.35f
#define DAMAGE_REDUCTION 5
#define MAX_DAMAGEDECAY 100

//=========================================================
//=========================================================
#define MAX_PLAYER_LEAN 10

//=========================================================
//=========================================================
enum PlayerWeaponSlots_t
{
	WEAPONSLOT_INVALID = -1,
	WEAPONSLOT_PRIMARY = 0,
	WEAPONSLOT_SECONDARY,
	WEAPONSLOT_MELEE,
	WEAPONSLOT_EQUIPMENT1,
	WEAPONSLOT_EQUIPMENT2,
	WEAPONSLOT_COUNT
};

#define PLAYER_WEAPONSLOT_BITS 3

//=========================================================
//=========================================================
#include "ins_player_defines.h"

//=========================================================
//=========================================================
struct HitGroupData_t
{
	HitGroupData_t( )
	{
		m_iHitTolerance = 1;
		m_flMultiplyer = 1.0f;
	}

	int m_iHitTolerance;
	float m_flMultiplyer;
};

struct LoadPlayerData_t
{
	LoadPlayerData_t( )
	{
		//memset( m_iMaxCarry, 0, sizeof( m_iMaxCarry ) );
	}

#ifdef GAME_DLL

	HitGroupData_t m_HitGroupData[ HIRGROUP_COUNT ];

#endif
};

//=========================================================
//=========================================================
extern ConVar drawdebugmuzzle;
extern void UTIL_DrawDebugMuzzle( Vector &vecMuzzle, QAngle &angMuzzle );

//=========================================================
//=========================================================
#define PLAYER_CLASSPREFERENCE_COUNT 3 
typedef int ClassPreferences_t[ PLAYER_CLASSPREFERENCE_COUNT ];

//=========================================================
//=========================================================
#define VIEWHEIGHT_CROUCHED Vector( 0, 0, 8.0f )

//=========================================================
//=========================================================
#define MAX_LASTAREA_LENGTH 48

//=========================================================
//=========================================================
#define PCMD_INITALSPAWN	"initialspawn"
#define PCMD_PORESPONSE		"poresponse"
#define PCMD_FINISHDI		"cdeathinfo"
#define PCMD_VOICECYCLE		"voicecycle"
#define PCMD_PSTATSLOGIN	"pstatslogin"
#define PCMD_NEEDSHELP		"needshelp"
#define PCMD_STATUSBCAST	"statusbcast"

//=========================================================
//=========================================================
class CPlayerAnimState
{
public:
	enum
	{
		TURN_NONE = 0,
		TURN_LEFT,
		TURN_RIGHT
	};

	CPlayerAnimState( CINSPlayer *outer );

	Activity			BodyYawTranslateActivity( Activity activity );

	void				Update();

	const QAngle&		GetRenderAngles();
				
	void				GetPoseParameters( float poseParameter[MAXSTUDIOPOSEPARAM] );

	CINSPlayer		*GetOuter();

private:
	void				GetOuterAbsVelocity( Vector& vel );

	int					ConvergeAngles( float goal,float maxrate, float dt, float& current );

	void				EstimateYaw( void );
	void				ComputePoseParam_BodyYaw( void );
	void				ComputePoseParam_BodyPitch( void );
	void				ComputePoseParam_BodyLookYaw( void );

	void				ComputePlaybackRate();

	CINSPlayer		*m_pOuter;

	float				m_flGaitYaw;
	float				m_flStoredCycle;

	// The following variables are used for tweaking the yaw of the upper body when standing still and
	//  making sure that it smoothly blends in and out once the player starts moving
	// Direction feet were facing when we stopped moving
	float				m_flGoalFeetYaw;
	float				m_flCurrentFeetYaw;

	float				m_flCurrentTorsoYaw;

	// To check if they are rotating in place
	float				m_flLastYaw;
	// Time when we stopped moving
	float				m_flLastTurnTime;

	// One of the above enums
	int					m_nTurningInPlace;

	QAngle				m_angRender;

	float				m_flTurnCorrectionTime;

    // xENO [
    Vector2D            m_vLastMovePose;
    float CalcMovementPlaybackRate();
    // ] xENO
};

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#include "c_ins_player.h"

#else

#include "ins_player.h"

#endif

//=========================================================
//=========================================================
typedef CHandle< CINSPlayer > CINSPlayerHandle;

#endif // INS_PLAYER_SHARED_H