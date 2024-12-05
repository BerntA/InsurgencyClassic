//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "weapon_rifle_grenade_base.h"

#ifdef GAME_DLL

#include "missile_launched_base.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponM16M203 C_WeaponM16M203 

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

class CM403Grenade : public CBaseLaunchedMissile
{
public:
	DECLARE_CLASS( CM403Grenade, CBaseLaunchedMissile );

public:
	
};

LINK_ENTITY_TO_CLASS( m403_grenade, CM403Grenade );
PRECACHE_WEAPON_REGISTER( m403_grenade );
REGISTER_DETONATOR( AMMOTYPE_MISSILE, MISSILE_M406, m403_grenade );

#endif

//=========================================================
//=========================================================
class CWeaponM16M203 : public CWeaponRifleGrenadeBase
{
public:
	DECLARE_CLASS( CWeaponM16M203, CWeaponRifleGrenadeBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponM16M203() { }
	
	int GetWeaponID( void ) const { return WEAPON_M16M203; }

	Activity GetSightDrawFullActivity( void ) const;
	Activity GetSightHolsterFullActivity( void ) const;
	Activity GetSightIdleActivity( void ) const;
	Activity GetSightCrawlActivity( void ) const;
	Activity GetSightDownActivity( void ) const;
	Activity GetSightShootIronsightsActivity( void ) const;
	Activity GetSightShootHippedActivity( void ) const;
	Activity GetSightHolsterActivity( void ) const;
	Activity GetSightDrawDrawActivity( void ) const;
	Activity GetSightDrawReadyActivity( void ) const;
	Activity GetSightReloadActivity( void ) const;
	Activity GetSightIronsightInActivity( void ) const;
	Activity GetSightIronsightIdleActivity( void ) const;
	Activity GetSightIronsightOutActivity( void ) const;

protected:
	int GetLaunchedMissileID( void ) const { return MISSILE_M406; }

private:
	CWeaponM16M203( const CWeaponM16M203 & );
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponM16M203, DT_WeaponM16M203)

BEGIN_NETWORK_TABLE(CWeaponM16M203, DT_WeaponM16M203)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponM16M203)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_m16m203, CWeaponM16M203);
PRECACHE_WEAPON_REGISTER(weapon_m16m203);
REGISTER_WEAPON_DATA(WEAPON_M16M203, WEAPONTYPE_PRIMARY, weapon_m16m203, "M16M203");
REGISTER_WEAPON_DATAHELPER(WEAPON_M16M203, RifleGrenadeInfo_t);

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightDrawFullActivity(void) const
{
	return ACT_VM_DRAWFULL_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightHolsterFullActivity(void) const
{
	return ACT_VM_HOLSTERFULL_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightIdleActivity(void) const
{
	return ACT_VM_IDLE_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightCrawlActivity( void ) const
{
	return ACT_VM_CRAWL_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightDownActivity( void ) const
{
	return ACT_VM_DOWN_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightShootIronsightsActivity( void ) const
{
	return ACT_VM_ISHOOT_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightShootHippedActivity( void ) const
{
	return ACT_VM_SECONDARYATTACK;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightHolsterActivity(void) const
{
	return ACT_VM_HOLSTER_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightDrawDrawActivity(void) const
{
	return ACT_VM_DRAW_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightDrawReadyActivity(void) const
{
	return ACT_VM_READY_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightReloadActivity(void) const
{
	return ACT_VM_RELOAD_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightIronsightInActivity(void) const
{
	return ACT_VM_IIN_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightIronsightIdleActivity(void) const
{
	return ACT_VM_IIDLE_M203;
}

//=========================================================
//=========================================================
Activity CWeaponM16M203::GetSightIronsightOutActivity(void) const
{
	return ACT_VM_IOUT_M203;
}