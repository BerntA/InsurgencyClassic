//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "weapon_rifle_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponAK47 C_WeaponAK47
#endif

//=========================================================
//=========================================================
class CWeaponAK47 : public CWeaponRifleBase
{
public:
	DECLARE_CLASS(CWeaponAK47, CWeaponRifleBase);

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponAK47() { }

	int GetWeaponID(void) const { return WEAPON_AK47; }

private:
	CWeaponAK47(const CWeaponAK47 &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponAK47, DT_WeaponAK47)

BEGIN_NETWORK_TABLE(CWeaponAK47, DT_WeaponAK47)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponAK47)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_ak47, CWeaponAK47);
PRECACHE_WEAPON_REGISTER(weapon_ak47);
REGISTER_WEAPON_DATA(WEAPON_AK47, WEAPONTYPE_PRIMARY, weapon_ak47, "AK47");
REGISTER_WEAPON_DATAHELPER(WEAPON_AK47, BallisticWeaponInfo_t);