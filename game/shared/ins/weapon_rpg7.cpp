//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "weapon_rpg_base.h"

#ifdef GAME_DLL

#include "missile_rocket_base.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponRPG7 C_WeaponRPG7

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

class CRPG7Missile : public CBaseRocketMissile
{
public:
	DECLARE_CLASS( CRPG7Missile, CBaseRocketMissile );

	CRPG7Missile( ) { }

private:
	CRPG7Missile( const CRPG7Missile & );
}; 

LINK_ENTITY_TO_CLASS( missile_rpg7, CRPG7Missile );
REGISTER_DETONATOR( AMMOTYPE_MISSILE, MISSILE_OG7V, missile_rpg7 );

#endif

//=========================================================
//=========================================================
class CWeaponRPG7 : public CWeaponRPGBase
{
public:
	DECLARE_CLASS( CWeaponRPG7, CWeaponRPGBase );
	DECLARE_NETWORKCLASS( ); 
	DECLARE_PREDICTABLE( );

public:
	CWeaponRPG7( ) { }

	int GetWeaponID( void ) const { return WEAPON_RPG7; };
	int GetMissileID( void ) const { return MISSILE_OG7V; }

private:
	CWeaponRPG7( const CWeaponRPG7 & );
}; 

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponRPG7, DT_WeaponRPG7)

BEGIN_NETWORK_TABLE(CWeaponRPG7, DT_WeaponRPG7)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponRPG7)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_rpg7, CWeaponRPG7);
PRECACHE_WEAPON_REGISTER(weapon_rpg7);
REGISTER_WEAPON_DATA(WEAPON_RPG7, WEAPONTYPE_PRIMARY, weapon_rpg7, "RPG7");
REGISTER_WEAPON_DATAHELPER(WEAPON_RPG7, INSWeaponInfo_t);