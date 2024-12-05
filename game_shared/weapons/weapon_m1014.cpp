//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "weapon_shotgun_base.h"

#ifdef CLIENT_DLL
#define CWeaponM1014 C_WeaponM1014
#endif

//=========================================================
//=========================================================
class CWeaponM1014 : public CWeaponShotgunBase
{
public:
	DECLARE_CLASS(CWeaponM1014, CWeaponShotgunBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponM1014() { }
	 
	int GetWeaponID(void) const { return WEAPON_M1014; }

private:
	CWeaponM1014(const CWeaponM1014 &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponM1014, DT_WeaponM1014)

BEGIN_NETWORK_TABLE(CWeaponM1014, DT_WeaponM1014)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponM1014)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_m1014, CWeaponM1014);
PRECACHE_WEAPON_REGISTER( weapon_m1014 );
REGISTER_WEAPON_DATA(WEAPON_M1014, WEAPONTYPE_PRIMARY, weapon_m1014, "M1014");
REGISTER_WEAPON_DATAHELPER(WEAPON_M1014, BallisticWeaponInfo_t);