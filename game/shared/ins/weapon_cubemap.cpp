//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_ins_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL
#define CWeaponCubemap C_WeaponCubemap
#endif

//=========================================================
//=========================================================
class CWeaponCubemap : public CWeaponINSBase
{
	DECLARE_CLASS( CWeaponCubemap, CWeaponINSBase );

	DECLARE_NETWORKCLASS( ); 
	DECLARE_PREDICTABLE( );

public:
	CWeaponCubemap( ) { }

	int GetWeaponID(void) const { return WEAPON_AR2; }
	int GetWeaponClass(void) const { return WEAPONCLASS_MISC; }

	bool CanUseIronsights( void ) { return false; }
		
private:
	CWeaponCubemap( const CWeaponCubemap & );
};

//=========================================================
//=========================================================
class CubemapWeaponInfo_t : public INSWeaponInfo_t
{
	typedef FileWeaponInfo_t BaseClass;

public:
	CubemapWeaponInfo_t( )
	{
		Q_strncpy( szClassName, "weapon_cubemap", MAX_WEAPON_STRING );
		Q_strncpy( szViewModel, "models/shadertest/envballs.mdl", MAX_WEAPON_STRING );
		Q_strncpy( szWorldModel, "models/weapons/w_irifle.mdl", MAX_WEAPON_STRING );
		Q_strncpy( szAnimationSuffix, "AK47", MAX_WEAPON_PREFIX );
	}

private:
	bool ShouldParse( void ) const { return false; }
};


//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponCubemap, DT_WeaponCubemap )

BEGIN_NETWORK_TABLE( CWeaponCubemap, DT_WeaponCubemap )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CWeaponCubemap )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_cubemap, CWeaponCubemap );
PRECACHE_WEAPON_REGISTER( weapon_cubemap );
REGISTER_WEAPON_DATA( WEAPON_CUBEMAP, WEAPONTYPE_EQUIPMENT, weapon_cubemap, "Cubemap" );
REGISTER_WEAPON_DATAHELPER( WEAPON_CUBEMAP, CubemapWeaponInfo_t );