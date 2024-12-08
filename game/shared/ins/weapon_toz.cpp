//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "weapon_shotgun_base.h"

#ifdef CLIENT_DLL
#define CWeaponTOZ C_WeaponTOZ
#endif

//=========================================================
//=========================================================
class CWeaponTOZ : public CWeaponShotgunBase
{
public:
	DECLARE_CLASS(CWeaponTOZ, CWeaponShotgunBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponTOZ() { }
	 
	int GetWeaponID(void) const { return WEAPON_TOZ; }

private:
	CWeaponTOZ(const CWeaponTOZ &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponTOZ, DT_WeaponTOZ)

BEGIN_NETWORK_TABLE(CWeaponTOZ, DT_WeaponTOZ)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponTOZ)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_toz, CWeaponTOZ);
PRECACHE_WEAPON_REGISTER(weapon_toz);
REGISTER_WEAPON_DATA(WEAPON_TOZ, WEAPONTYPE_PRIMARY, weapon_toz, "TOZ");
REGISTER_WEAPON_DATAHELPER(WEAPON_TOZ, BallisticWeaponInfo_t);