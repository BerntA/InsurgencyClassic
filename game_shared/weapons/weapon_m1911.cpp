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
#define CWeaponM1911 C_WeaponM1911
#endif

//=========================================================
//=========================================================
/*class CWeaponM1911 : public CWeaponPistolBase
{
public:
	DECLARE_CLASS(CWeaponM1911, CWeaponPistolBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponM1911() { }
	 
	virtual int GetWeaponID(void) const { return WEAPON_M1911; }

private:
	CWeaponM1911(const CWeaponM1911 &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponM1911, DT_WeaponM1911)

BEGIN_NETWORK_TABLE(CWeaponM1911, DT_WeaponM1911)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponM1911)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_m1911, CWeaponM1911);
PRECACHE_WEAPON_REGISTER(weapon_m1911);
REGISTER_WEAPON_DATA(WEAPON_M1911, WEAPONTYPE_SECONDARY, weapon_m1911, "M1911");
REGISTER_WEAPON_DATAHELPER(WEAPON_M1911, BallisticWeaponInfo_t);*/
