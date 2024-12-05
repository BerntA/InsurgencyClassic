//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapon data file parsing for developer weapons
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_DEVELOPER_H
#define WEAPON_DEVELOPER_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_ins_base.h"

//=========================================================
//=========================================================
class DeveloperWeaponInfo_t : public INSWeaponInfo_t
{
	typedef FileWeaponInfo_t BaseClass;

public:
	DeveloperWeaponInfo_t() { }

	bool ShouldParse(void) const { return false; }
};

//=========================================================
//=========================================================
class CWeaponDeveloperBase : public CWeaponINSBase
{
	DECLARE_CLASS(CWeaponDeveloperBase, CWeaponINSBase);

public:
	CWeaponDeveloperBase() { }

	bool CanDeploy(void);
};

#endif // WEAPON_DEVELOPER_H