//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_MELEE_BASE_H
#define WEAPON_MELEE_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_ins_base.h"
#include "effect_dispatch_data.h" 

#ifdef GAME_DLL

#include "npcevent.h"

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL
#define CWeaponMeleeBase C_WeaponMeleeBase 
#endif

//=========================================================
//=========================================================
#define MELEE_EVENT 1338

//=========================================================
//=========================================================
class MeleeWeaponInfo_t : public INSWeaponInfo_t
{
	typedef INSWeaponInfo_t BaseClass;

public:
	MeleeWeaponInfo_t( );

	void Parse( KeyValues *pKeyValuesData, const char *pszWeaponName );

public:
	float flDamage;
};

//=========================================================
//=========================================================
class CWeaponMeleeBase : public CWeaponINSBase
{ 
	DECLARE_CLASS( CWeaponMeleeBase, CWeaponINSBase ); 

public:
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( ); 

public:
	CWeaponMeleeBase( ); 

private:
	CWeaponMeleeBase( const CWeaponMeleeBase & ); 

	int GetWeaponClass( void ) const { return WEAPONCLASS_MELEE; }

	void PrimaryAttack( void ); 
	bool CanFireUnderWater( void ) const { return true; }

	void Swing( void ); 
	void FindHit( void );
	void FindHullIntersection( trace_t &tr, const Vector &mins, const Vector &maxs, CBasePlayer *pPlayer ); 
	void Hit( trace_t &tr );
	void AddViewKick( void );
	bool ImpactWater( const Vector &ecvStart, const Vector &vecEnd ); 
	void ImpactEffect( trace_t &tr );

	bool CanUseIronsights( void ) { return false; }

	bool HasAmmo( void ) const;
	
#ifdef GAME_DLL

	void HandleAnimEvent( animevent_t *pEvent );

	bool IsInflictorDistance( void ) { return false; }

	bool AllowViewmodelInteraction( void ) const { return false; }

#else

	bool OnFireEvent( C_BaseViewModel *pViewModel, const Vector& origin, const QAngle& angles, int event, const char *options );

#endif
}; 

#endif // WEAPON_MELEE_BASE_H