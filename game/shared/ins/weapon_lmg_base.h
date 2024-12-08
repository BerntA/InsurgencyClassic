//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_LMG_BASE_H
#define WEAPON_LMG_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_bipod_base.h"
 
//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponLMGBase C_WeaponLMGBase

#endif

//=========================================================
//=========================================================
class CWeaponLMGBase : public CWeaponBipodBase
{
public:
	DECLARE_CLASS( CWeaponLMGBase, CWeaponBallisticBase );
	 
public:
	CWeaponLMGBase( );

	int GetWeaponClass( void ) const { return WEAPONCLASS_LMG; }

	bool ShowTracers( void ) const { return true; }
}; 

#endif // WEAPON_LMG_BASE_H



