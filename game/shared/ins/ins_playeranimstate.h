//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_PLAYERANIMSTATE_H
#define INS_PLAYERANIMSTATE_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#include "convar.h"
#include "iplayeranimstate.h"
#include "base_playeranimstate.h"

#ifdef CLIENT_DLL

class C_BaseAnimatingOverlay;
class C_BaseCombatWeapon;
#define CBaseAnimatingOverlay C_BaseAnimatingOverlay
#define CBaseCombatWeapon C_BaseCombatWeapon

#else

class CBaseAnimatingOverlay;
class CBaseCombatWeapon;

#endif

//=========================================================
//=========================================================
enum PlayerAnimEvent_e
{
	// weapon events
	PLAYERANIMEVENT_WEAP_FIRE1,   // primary fire
	PLAYERANIMEVENT_WEAP_FIRE2,   // secondary fire
	PLAYERANIMEVENT_WEAP_RELOAD,  // weapon reload

	// player events
	PLAYERANIMEVENT_PLAY_JUMP,    // jump

	PLAYERANIMEVENT_COUNT
};

#include "gamemovement.h"
	
//=========================================================
//=========================================================
class IINSPlayerAnimState : virtual public IPlayerAnimState
{
public:
	// this is called by both the client and the server in the same way to trigger events for
	// players firing, jumping, throwing grenades, etc.
	virtual void DoAnimationEvent( PlayerAnimEvent_e event ) = 0;
	
	virtual void EnsureValidLayers( void ) = 0;
};

//=========================================================
//=========================================================
enum PlayerLeaningType_t
{
	PLEANING_NONE = 0,
	PLEANING_RIGHT,
	PLEANING_LEFT
};

//=========================================================
//=========================================================
class IINSPlayerAnimStateHelper
{
public:
	virtual CBaseCombatWeapon *INSAnim_GetActiveWeapon( void ) = 0;

	virtual int INSAnim_GetPlayerFlags( void ) = 0;

	virtual bool INSAnim_InStanceTransition( void ) = 0;
    virtual int INSAnim_CurrentStance( void ) = 0;
	virtual int INSAnim_LastStance( void ) = 0;
	virtual int INSAnim_OldStance( void ) = 0;

	virtual int INSAnim_LeaningType( void ) = 0;

};

//=========================================================
//=========================================================
extern IINSPlayerAnimState *CreatePlayerAnimState( CBaseAnimatingOverlay *pEntity, IINSPlayerAnimStateHelper *pHelper, LegAnimType_t legAnimType, bool bUseAimSequences );

//=========================================================
//=========================================================

// if this is set, then the game code needs to make sure to send player animation events
// to the local player if he's the one being watched.
extern ConVar cl_showanimstate;

#endif // SDK_PLAYERANIMSTATE_H
