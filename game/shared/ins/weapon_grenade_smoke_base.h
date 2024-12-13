//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_SMOKE_GRENADE_H
#define WEAPON_SMOKE_GRENADE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_grenade_base.h"

#ifdef GAME_DLL

#include "grenade_thrown_base.h"
#include "smoke_trail.h"

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

class CSmokeGrenade : public CGrenadeThrownBase
{
	DECLARE_CLASS( CSmokeGrenade, CGrenadeThrownBase );

private:
	void Spawn( void );

	void UpdateOnRemove( void );

	void Detonate( void );

	void CreateSmokeTrail( void );
	void RemoveSmokeTrail( void );

private:
	CHandle< SmokeTrail > m_hSmokeTrail;
};

#endif

//=========================================================
//=========================================================
class CWeaponGrenadeSmokeBase : public CWeaponGrenadeBase
{
	DECLARE_CLASS( CWeaponGrenadeSmokeBase, CWeaponGrenadeBase );

private:
	bool ValidAttack( int iAttackType ) const;
};

#endif // WEAPON_SMOKE_GRENADE_H