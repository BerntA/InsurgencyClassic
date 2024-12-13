//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_SNIPER_BASE_H
#define WEAPON_SNIPER_BASE_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "weapon_bipod_base.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponSniperBase C_WeaponSniperBase

#endif

//=========================================================
//=========================================================
class CWeaponSniperBase : public CWeaponBipodBase
{
public:
	DECLARE_CLASS(CWeaponSniperBase, CWeaponBipodBase);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

public:
	CWeaponSniperBase();

	int GetWeaponClass(void) const { return WEAPONCLASS_SNIPER; }

	void HandleDeploy(void);

	bool ShouldDrawViewModel(void) const;

	void ItemPostFrame(void);

	void SetIronsightsState(bool bState);
	void FinishedIronsights(void);

	bool IgnoreRange(void) const { return false; }

	void PrimaryAttack(void);

#ifdef CLIENT_DLL

	bool HasScope(void) const { return true; }

	bool UseFreeaim(void) const;

#endif

	bool CanUseIronsights(void);

#ifdef GAME_DLL

	bool SendIronsightsHint(void) const { return false; }

#endif

private:
	bool Using3DScopes(void) const;

private:
	CNetworkVar(float, m_flFinishIronsight);
	CNetworkVar(bool, m_bHideViewModel);

#ifdef GAME_DLL

	bool m_bResetFOV;

#endif
};

#endif // WEAPON_SNIPER_BASE_H