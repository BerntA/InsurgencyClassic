//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_grenade_smoke_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CGrenadeM18 C_GrenadeM18
#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

LINK_ENTITY_TO_CLASS( grenade_m18, CSmokeGrenade );
REGISTER_DETONATOR( AMMOTYPE_GRENADE, GRENADE_M18, grenade_m18 );

#endif

//=========================================================
//=========================================================
class CGrenadeM18 : public CWeaponGrenadeSmokeBase
{
	DECLARE_CLASS( CGrenadeM18, CWeaponGrenadeSmokeBase );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

public:
	CGrenadeM18( ) { }

private:
	CGrenadeM18(const CGrenadeM18 &);

	int GetWeaponID( void ) const { return WEAPON_M18; }

#ifdef GAME_DLL

	int GetGrenadeID( void ) const { return GRENADE_M18; }

#endif
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(GrenadeM18, DT_GrenadeM18)

BEGIN_NETWORK_TABLE(CGrenadeM18, DT_GrenadeM18)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CGrenadeM18)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_m18, CGrenadeM18);
PRECACHE_WEAPON_REGISTER(weapon_m18);
REGISTER_WEAPON_DATA(WEAPON_M18, WEAPONTYPE_EQUIPMENT, weapon_m18, "M18");
REGISTER_WEAPON_DATAHELPER(WEAPON_M18, INSWeaponInfo_t);