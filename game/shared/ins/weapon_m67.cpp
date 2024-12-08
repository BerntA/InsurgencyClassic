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
#define CGrenadeM67 C_GrenadeM67
#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

LINK_ENTITY_TO_CLASS( grenade_m67, CFragGrenade );
REGISTER_DETONATOR( AMMOTYPE_GRENADE, GRENADE_M67, grenade_m67 );

#endif

//=========================================================
//=========================================================
class CGrenadeM67 : public CWeaponGrenadeFragBase
{
	DECLARE_CLASS( CGrenadeM67, CWeaponGrenadeFragBase );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

public:
	CGrenadeM67( ) { }

private:
	CGrenadeM67( const CGrenadeM67 & );

	int GetWeaponID( void ) const { return WEAPON_M67; }

#ifdef GAME_DLL

	int GetGrenadeID( void ) const { return GRENADE_M67; }

#endif
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(GrenadeM67, DT_GrenadeM67)

BEGIN_NETWORK_TABLE(CGrenadeM67, DT_GrenadeM67)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CGrenadeM67)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m67, CGrenadeM67 );
PRECACHE_WEAPON_REGISTER( weapon_m67 );
REGISTER_WEAPON_DATA( WEAPON_M67, WEAPONTYPE_EQUIPMENT, weapon_m67, "M67" );
REGISTER_WEAPON_DATAHELPER( WEAPON_M67, INSWeaponInfo_t );
