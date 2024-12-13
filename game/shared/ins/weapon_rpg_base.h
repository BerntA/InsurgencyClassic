//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_RPG_BASE_H
#define WEAPON_RPG_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_loadable_base.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponRPGBase C_WeaponRPGBase

#endif

#ifdef GAME_DLL

class RocketTrail;

#endif

//=========================================================
//=========================================================
class CWeaponRPGBase : public CWeaponLoadableBase
{
	DECLARE_CLASS( CWeaponRPGBase, CWeaponLoadableBase );

public:
	CWeaponRPGBase( );

	int GetWeaponClass( void ) const { return WEAPONCLASS_RPG; }

	void HandleDeploy( void );
	void OnDeployReady( void );

	bool HasPrimaryAttack( void ) { return true; }
	void PrimaryAttack( void );

	void FinishReload( void );

#ifdef GAME_DLL

	void HandleAnimEvent( animevent_t *pEvent );

#endif

	bool EnableEmptyAnimations( void ) const { return true; }
	bool ShouldEmptyAnimate( void ) const;

	bool ShouldHideWeapon( void );
	bool CanMoveShoot( void ) const { return false; }

	Activity GetDrawActivity( void );
	Activity GetEmptyReloadActivity( void ) const;

	virtual int GetMissileID( void ) const = 0;

#ifdef GAME_DLL

	void UpdateProjectile( void );

#endif

	bool AllowBreathing( void ) { return true; }

	bool UseFreeaim( void ) const { return true; }

	bool LowerViewmodelInteraction( void ) { return true; }
	void CalcViewmodelInteraction( float &flLength, float &flHalfWidth );

private:

#ifdef GAME_DLL

	int m_iProjectileGroupID;

#endif
};

#endif // WEAPON_RPG_BASE_H