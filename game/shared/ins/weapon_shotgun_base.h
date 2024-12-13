//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_SHOTGUN_BASE_H
#define WEAPON_SHOTGUN_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "weapon_rifle_base.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponShotgunBase C_WeaponShotgunBase

#endif

//=========================================================
//=========================================================
class CWeaponShotgunBase : public CWeaponRifleBase
{
	DECLARE_CLASS( CWeaponShotgunBase, CWeaponRifleBase );

public:
	CWeaponShotgunBase( ) { }

private:
	int GetWeaponClass( void ) const { return WEAPONCLASS_SHOTGUN; }

	int ForcedFiremode( void ) const;

	int GetShots( void ) const;

	bool IsSingleLoad( void ) const { return true; }

	int GetShellType( void ) const;
};

#endif // WEAPON_SHOTGUNBASE_H