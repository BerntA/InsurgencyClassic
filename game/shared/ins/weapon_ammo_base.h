//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_AMMO_BASE_H
#define WEAPON_AMMO_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_ins_base.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponAmmoBase C_WeaponAmmoBase

#endif

//=========================================================
//=========================================================
class CWeaponAmmoBase : public CWeaponINSBase
{
	DECLARE_CLASS( CWeaponAmmoBase, CWeaponINSBase );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

public:
	CWeaponAmmoBase( );

	virtual bool HasAmmo( void ) const;
	virtual int GetAmmoCount( void ) const;

	void GiveAmmo( int iCount );

protected:
	void StripAmmo( void );

private:
	CNetworkVar( int, m_iAmmoCount );
};

#endif // WEAPON_AMMO_BASE_H
