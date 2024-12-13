//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_RIFLE_BASE_H
#define WEAPON_RIFLE_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_ballistic_base.h"
 
//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponRifleBase C_WeaponRifleBase

#endif

//=========================================================
//=========================================================
class CWeaponRifleBase : public CWeaponBallisticBase
{
public:
	DECLARE_CLASS( CWeaponRifleBase, CWeaponBallisticBase );

public:
	CWeaponRifleBase( ) { }

	virtual int GetWeaponClass( void ) const { return WEAPONCLASS_RIFLE; }

protected:
	virtual float GetCycleTime( void );

	virtual void CheckReload( void );
	virtual void FinishReload( void );
	virtual Activity GetReloadActivity( void ) const;

	virtual bool IsBoltAction( void ) const { return false; }
	virtual bool IsSingleLoad( void ) const { return false; }
};

#endif // WEAPON_RIFLE_BASE_H