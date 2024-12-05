//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_BIPODBASE_H
#define WEAPON_BIPODBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_rifle_base.h"
 
//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponBipodBase C_WeaponBipodBase

#endif

//=========================================================
//=========================================================
class CWeaponBipodBase : public CWeaponRifleBase
{
public:
	DECLARE_CLASS( CWeaponBipodBase, CWeaponRifleBase );
	 
public:
	CWeaponBipodBase( );

	bool CanBipod( Vector *pViewOffset = NULL );

protected:
	virtual bool AllowBipod( void ) const { return true; }

	virtual Activity GetCrawlActivity( void ) const;
	virtual Activity GetNormalActivity( void ) const;

	virtual Activity GetPrimaryAttackActivity( void ) const;
	virtual Activity GetPrimaryAttackActivityRecoil( Activity BaseAct, int iRecoilState ) const;
	virtual Activity GetDryFireActivity( void ) const;

	virtual Activity GetReloadActivity( void ) const;

	virtual Activity GetIronsightInActivity( void ) const;
	virtual Activity GetIronsightOutActivity( void ) const;

	float CalcOffsetTime( int iEyeOffset ) const;

private:
	bool CanHolster( void );

	void SecondaryAttack( void );

	bool AttemptBipod( void );

	bool Use( void );
}; 

#endif // WEAPON_BIPODBASE_H



