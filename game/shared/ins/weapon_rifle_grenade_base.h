//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_RIFLE_GRENADE_BASE_H
#define WEAPON_RIFLE_GRENADE_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_rifle_base.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponRifleGrenadeBase C_WeaponRifleGrenadeBase

#endif

//=========================================================
//=========================================================
class RifleGrenadeInfo_t : public BallisticWeaponInfo_t
{
	DECLARE_BASECLASS( BallisticWeaponInfo_t );

public:
	RifleGrenadeInfo_t( );

	void Parse( KeyValues *pKeyValuesData, const char *pszWeaponName );

public:

#ifdef CLIENT_DLL

	char szGrenadeAmmoTex[ MAX_WEAPON_STRING ];
	int iGrenadeAmmoTexID;

#endif

};

//=========================================================
//=========================================================
class CWeaponRifleGrenadeBase : public CWeaponRifleBase
{
public:
	DECLARE_CLASS( CWeaponRifleGrenadeBase, CWeaponRifleBase );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

public:
	CWeaponRifleGrenadeBase( );

protected:
	virtual Activity GetSightDrawFullActivity( void ) const;
	virtual Activity GetSightHolsterFullActivity( void ) const;
	virtual Activity GetSightIdleActivity( void ) const;
	virtual Activity GetSightCrawlActivity( void ) const;
	virtual Activity GetSightDownActivity( void ) const;
	virtual Activity GetSightShootIronsightsActivity( void ) const;
	virtual Activity GetSightShootHippedActivity( void ) const;	
	virtual Activity GetSightHolsterActivity( void ) const;
	virtual Activity GetSightDrawDrawActivity( void ) const;
	virtual Activity GetSightDrawReadyActivity( void ) const;
	virtual Activity GetSightReloadActivity( void ) const;
	virtual Activity GetSightIronsightInActivity( void ) const;
	virtual Activity GetSightIronsightIdleActivity( void ) const;
	virtual Activity GetSightIronsightOutActivity( void ) const;

private:
	void HandleDeploy( void );

	bool AllowSprintStart( void );
	bool AllowJump( void );

	bool IsEmptyAttack( void );

	bool CanAttack( void );
	bool CanPrimaryAttack( void );

	void PrimaryAttack( void );
	void SecondaryAttack( void );

	void SetSights( bool bState );
	void LoadGrenade( void );
	bool InSightChange( void );

	bool CanReload( void );
	bool IsEmptyReload( void );
	void FinishReload( void );

	void GiveAmmo( int iCount );
	bool HasAmmo( void ) const;
	bool HasGrenades( void ) const;
	int GetAmmoCount( void ) const;

	Activity GetDrawActivity( void );
	Activity GetHolsterActivity( void );
	Activity GetNormalActivity( void ) const;
	Activity GetCrawlActivity( void ) const;
	Activity GetDownActivity( void ) const;
	Activity GetReloadActivity( void ) const;
	Activity GetIronsightInActivity( void ) const;
	Activity GetIronsightOutActivity( void ) const;
	Activity GetSightDrawActivity( void );
	Activity GetSightShootActivity( void ) const;

#ifdef CLIENT_DLL

	void PrecacheResources( void );

	int GetAmmoTexID( void ) const;

#endif

protected:
	virtual int GetLaunchedMissileID( void ) const;

private:
	CNetworkVar( bool, m_bSightsUp );
	CNetworkVar( float, m_flSightChangeTime );

	CNetworkVar( bool, m_bGrenadeLoaded );
	CNetworkVar( int, m_iGrenades );
};

#endif // WEAPON_RIFLE_GRENADE_BASE_H
