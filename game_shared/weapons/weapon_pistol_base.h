//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_PISTOL_BASE_H
#define WEAPON_PISTOL_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_ballistic_base.h"
 
#ifdef CLIENT_DLL
#define CWeaponPistolBase C_WeaponPistolBase
#endif

//=========================================================
//=========================================================
class CWeaponPistolBase : public CWeaponBallisticBase
{
public:
	DECLARE_CLASS( CWeaponPistolBase, CWeaponBallisticBase );

public:
	CWeaponPistolBase();

	int GetWeaponClass(void) const { return WEAPONCLASS_PISTOL; }

	float GetCycleTime(void);
	float GetBurstCycleTime(void) const;

	bool EnableEmptyAnimations(void) const { return true; }

	int GetShellType( void ) const;

	Activity GetPrimaryAttackActivityRecoil(Activity BaseAct, int iRecoilState) const;

	bool UseLastShoot( void ) const { return true; }

	void CalcViewmodelInteraction( float &flLength, float &flHalfWidth );
};

#endif // WEAPON_PISTOLBASE_H