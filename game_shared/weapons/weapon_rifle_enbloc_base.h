//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_RIFLE_ENBLOC_BASE_H
#define WEAPON_RIFLE_ENBLOC_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_rifle_base.h"
 
#ifdef CLIENT_DLL
#define CWeaponRifleClipBase C_WeaponRifleEnblocBase
#endif

//=========================================================
//=========================================================
class CWeaponRifleEnblocBase : public CWeaponRifleBase
{
public:
	DECLARE_CLASS(CWeaponRifleEnblocBase, CWeaponRifleBase);

public:
	CWeaponRifleEnblocBase() { }

	bool CanReload(void);

	bool EnableEmptyAnimations(void) const { return true; }

	bool UseLastShoot( void ) const { return true; }
};

#endif // WEAPON_RIFLE_ENBLOC_BASE_H