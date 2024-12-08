//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_GRENADE_BASE_H
#define WEAPON_GRENADE_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_exhaustible_base.h"
#include "grenadedef.h"

#ifdef GAME_DLL

#include "npcevent.h"

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponGrenadeBase C_WeaponGrenadeBase

#endif

class CINSPlayer;

#ifdef GAME_DLL

class CBaseProjectile;

#endif

//=========================================================
//=========================================================
enum PausedType_t
{
	ATTACKTYPE_NONE = 0,
	ATTACKTYPE_THROW_HOLD,
	ATTACKTYPE_THROW_COOK,
	ATTACKTYPE_LOB,
	ATTACKTYPE_COUNT
};

//=========================================================
//=========================================================
class CWeaponGrenadeBase : public CWeaponExhaustibleBase
{
	DECLARE_CLASS( CWeaponGrenadeBase, CWeaponExhaustibleBase );
	DECLARE_NETWORKCLASS( ); 
	DECLARE_PREDICTABLE( );

public:
	CWeaponGrenadeBase( );

protected:
	virtual bool ValidAttack( int iAttackType ) const;
	virtual void FinishAttack( WeaponSound_t Sound );

#ifdef GAME_DLL

	bool CalculateThrowOrigin( Vector &vecOrigin, Vector *pForward );

	virtual const char *GetGrenadeClassname( void ) const { return NULL; }
	virtual int GetGrenadeID( void ) const;

	virtual void OnGrenadeCooked( void ) { }

#endif

	const CGrenadeData &GetGrenadeData( void ) const;

private:
	int GetWeaponClass( void ) const { return WEAPONCLASS_GRENADE; }

	void Spawn( void );

	bool AllowJumpAttack( void ) const { return true; }
	bool CanUseIronsights( void ) { return false; }
	bool AllowWaterAttack( void ) const { return true; }
	bool UseFreeaim( void ) const;

#ifdef GAME_DLL

	bool CanDrop( void );

#endif

	void HandleDeploy( void );

	bool CanHolster( void );
	bool Holster( CBaseCombatWeapon *pSwitchingTo );

	void ItemPostFrame( void );

	bool CanAttack( void );

	bool CanPrimaryAttack( void );
	bool CanSecondaryAttack( void );
	bool CanTertiaryAttack( void );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void TertiaryAttack( void );

	int PrimaryAttackType( void );
	int SecondaryAttackType( void );
	int TertiaryAttackType( void );

#ifdef GAME_DLL

	void HandleAnimEvent( animevent_t *pEvent );

#endif

	void Redraw( void );

	void ResetAttack( void );
	bool InAttack( void ) const;
	void StartAttack( int iAttackType );
	bool AttackForces( const Vector &vecForward, Vector &vecVelocity, AngularImpulse &angImpulse );

#ifdef GAME_DLL

	void EmitGrenade( Vector &vecOrigin, Vector &vecVelocity, AngularImpulse &angImpulse );
	void CheckThrowPosition( const Vector &vecEye, Vector &vecSrc );

	int GetInflictorType( void ) const;
	int GetInflictorID( void ) const;

#endif

	bool AllowSprintStart( void );

	bool AllowPlayerStance( int iFromStance, int iToStance );
	bool AllowPlayerMovement( void ) const;

private:
	CNetworkVar( int, m_iAttackType );
	CNetworkVar( bool, m_bDrawbackFinished );

#ifdef GAME_DLL

	float m_flCookThreshold;

#endif
};

#endif // WEAPON_GRENADE_BASE_H