//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_rpg_base.h"
#include "ammodef.h"

#include "engine/ivdebugoverlay.h"

#ifdef CLIENT_DLL
#include "baseviewmodel_shared.h"
#include "iinput.h"
#include "pain_helper.h"
#else
#include "npcevent.h"
#include "explode.h"
#include "smoke_trail.h"
#include "triggers.h"
#include "ins_utils.h"
#include "missile_rocket_base.h"
#endif // CLIENT_DLL

#include "missiledef.h"

#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define BODYGROUP_PROJECTILE "projectile"

//=========================================================
//=========================================================
CWeaponRPGBase::CWeaponRPGBase( )
{
#ifdef GAME_DLL

	m_iProjectileGroupID = -1;

#endif
}

//=========================================================
//=========================================================
void CWeaponRPGBase::HandleDeploy( void )
{
	BaseClass::HandleDeploy( );

#ifdef GAME_DLL

	m_iProjectileGroupID = FindBodygroupByName( BODYGROUP_PROJECTILE );

	UpdateProjectile( );

#endif
}

//=========================================================
//=========================================================
void CWeaponRPGBase::OnDeployReady( void )
{
	Load( );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

#define RPG_EMIT_EVENT 1337

void CWeaponRPGBase::HandleAnimEvent( animevent_t *pEvent )
{
	if( pEvent->event == RPG_EMIT_EVENT )
	{
		CINSPlayer *pOwner = GetINSPlayerOwner( );

		if( !pOwner )
			return;

		Vector vecOrigin, vecDirection;
		QAngle angMuzzle;

		pOwner->GetMuzzle( vecOrigin, angMuzzle );

		AngleVectors( angMuzzle, &vecDirection );
		VectorMA( vecOrigin, MISSILE_ROCKET_LENGTH, vecDirection, vecOrigin );

		CBaseRocketMissile::CreateRocketMissile( pOwner, GetMissileID( ), vecOrigin, angMuzzle, vecDirection );
	}
	else
	{
		BaseClass::HandleAnimEvent(pEvent);
	}
}

#endif

//=========================================================
//=========================================================
void CWeaponRPGBase::PrimaryAttack( void )
{
#ifdef GAME_DLL

	RegisterShot( );

#endif

	SendWeaponAnim( GetPrimaryAttackActivity( ) );

    DoAnimationEvent( PLAYERANIMEVENT_WEAP_FIRE1 );

	WeaponSound( SHOT_SINGLE );

#ifdef CLIENT_DLL

	if( DoClientEffects( ) )
		g_PainHelper.CreatePain( PAINTYPE_CLIGHT );

#endif

	m_bLoaded = false;

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );

#ifdef GAME_DLL

	UpdateProjectile( );

#endif

	SetIronsights( false, true );
}

//=========================================================
//=========================================================
void CWeaponRPGBase::FinishReload( void )
{
	BaseClass::FinishReload( );

#ifdef GAME_DLL

	UpdateProjectile( );

#endif
}

//=========================================================
//=========================================================
bool CWeaponRPGBase::ShouldHideWeapon( void )
{
	if( BaseClass::ShouldHideWeapon( ) )
		return true;

	CINSPlayer *pPlayer = GetINSPlayerOwner( );
	return ( pPlayer && pPlayer->IsProned( ) );
}

//=========================================================
//=========================================================
Activity CWeaponRPGBase::GetDrawActivity( void )
{
	if( !m_bLoaded )
	{
		if( HasAmmo( ) )
			return ACT_VM_READY;
		else
			return ACT_VM_DRAW_EMPTY;
	}
	else
	{
		return ACT_VM_DRAW;
	}
}

//=========================================================
//=========================================================
Activity CWeaponRPGBase::GetEmptyReloadActivity( void ) const
{
	return ACT_VM_RELOAD;
}

//=========================================================
//=========================================================
bool CWeaponRPGBase::ShouldEmptyAnimate( void ) const
{
	return IsEmpty( );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CWeaponRPGBase::UpdateProjectile( void )
{
	if( m_iProjectileGroupID == -1 )
		return;

	SetBodygroup( m_iProjectileGroupID, m_bLoaded ? 1 : 0 );
}

#endif

//=========================================================
//=========================================================
#define VM_RPG_LENGTH 10.0f
#define VM_RPG_LENGTH_DOWN 20.0f
#define VM_RPG_HALFWIDTH 1.5f

void CWeaponRPGBase::CalcViewmodelInteraction( float &flLength, float &flHalfWidth )
{
	flLength = VM_RPG_LENGTH;
	flHalfWidth = VM_RPG_HALFWIDTH;
}