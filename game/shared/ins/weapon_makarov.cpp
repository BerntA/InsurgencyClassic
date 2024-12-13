//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "weapon_pistol_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponMakarov C_WeaponMakarov
#endif

//=========================================================
//=========================================================
class CWeaponMakarov : public CWeaponPistolBase
{
public:
	DECLARE_CLASS( CWeaponMakarov, CWeaponPistolBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponMakarov() { }
	 
	int GetWeaponID( void ) const { return WEAPON_MAKAROV; }

private:
	CWeaponMakarov(const CWeaponMakarov &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponMakarov, DT_WeaponMakarov)

BEGIN_NETWORK_TABLE(CWeaponMakarov, DT_WeaponMakarov)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponMakarov)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_makarov, CWeaponMakarov);
PRECACHE_WEAPON_REGISTER( weapon_makarov );
REGISTER_WEAPON_DATA(WEAPON_MAKAROV, WEAPONTYPE_SECONDARY, weapon_makarov, "Makarov");
REGISTER_WEAPON_DATAHELPER(WEAPON_MAKAROV, BallisticWeaponInfo_t);