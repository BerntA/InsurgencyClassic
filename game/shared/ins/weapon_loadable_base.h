//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_LOADABLE_BASE_H
#define WEAPON_LOADABLE_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_ammo_base.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponLoadableBase C_WeaponLoadableBase

#endif

//=========================================================
//=========================================================
class CWeaponLoadableBase : public CWeaponAmmoBase
{
	DECLARE_CLASS( CWeaponLoadableBase, CWeaponAmmoBase );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

public:
	CWeaponLoadableBase( );

	void Spawn( void );

	bool HasAmmo( void ) const;
	int GetAmmoCount( void ) const;

	bool IsEmptyAttack( void );

	virtual bool CanReload( void );
	bool IsEmptyReload( void );
	void FinishReload( void );

	bool IsEmpty( void ) const;

	void ForceReady( void );

protected:
	void Load( void );

protected:
	CNetworkVar( bool, m_bLoaded );
};

#endif // WEAPON_LOADABLE_BASE_H	

		