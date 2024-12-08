//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_EXCHAUSTIBLEBASE_H
#define WEAPON_EXCHAUSTIBLEBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_loadable_base.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponExhaustibleBase C_WeaponExhaustibleBase

#endif

//=========================================================
//=========================================================
class CWeaponExhaustibleBase : public CWeaponLoadableBase
{
	DECLARE_CLASS(CWeaponExhaustibleBase, CWeaponLoadableBase);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

public:
	CWeaponExhaustibleBase();

	virtual void HandleDeploy(void);

	virtual bool CanHolster( void );
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

	virtual void ItemPostFrame(void);

#ifdef GAME_DLL
	virtual bool Drop(const Vector *pvecTarget = NULL, const Vector *pvecVelocity = NULL) { return false; }
#endif

	virtual bool CanReload(void);
	virtual void Reload(void);

	void UpdateIdleState( void );

#ifdef CLIENT_DLL

	bool AllowViewmodelInteraction( void ) const { return false; }

#endif

protected:
	virtual bool IsExhaustible(void) { return true; }

protected:
	CNetworkVar(bool, m_bRedraw);
};

#endif // WEAPON_EXCHAUSTIBLEBASE_H