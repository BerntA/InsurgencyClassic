//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_grenade_frag_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CGrenadeRGD5 C_GrenadeRGD5
#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

LINK_ENTITY_TO_CLASS( grenade_rgd5, CFragGrenade );
REGISTER_DETONATOR( AMMOTYPE_GRENADE, GRENADE_RGD5, grenade_rgd5 );

#endif

//=========================================================
//=========================================================
class CGrenadeRGD5 : public CWeaponGrenadeFragBase
{
	DECLARE_CLASS( CGrenadeRGD5, CWeaponGrenadeFragBase );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

public:
	CGrenadeRGD5( ) { }

private:
	CGrenadeRGD5( const CGrenadeRGD5 & );

	int GetWeaponID( void ) const { return WEAPON_RGD5; }

#ifdef GAME_DLL

	int GetGrenadeID( void ) const { return GRENADE_RGD5; }

#endif
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(GrenadeRGD5, DT_GrenadeRGD5)

BEGIN_NETWORK_TABLE(CGrenadeRGD5, DT_GrenadeRGD5)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CGrenadeRGD5)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_rgd5, CGrenadeRGD5);
PRECACHE_WEAPON_REGISTER(weapon_rgd5);
REGISTER_WEAPON_DATA(WEAPON_RGD5, WEAPONTYPE_EQUIPMENT, weapon_rgd5, "RGD5");
REGISTER_WEAPON_DATAHELPER(WEAPON_RGD5, INSWeaponInfo_t );
