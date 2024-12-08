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
#define CWeaponM9 C_WeaponM9
#endif

//=========================================================
//=========================================================
class CWeaponM9 : public CWeaponPistolBase
{
public:
	DECLARE_CLASS(CWeaponM9, CWeaponPistolBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponM9() { }

	int GetWeaponID(void) const { return WEAPON_M9; }

private:
	CWeaponM9(const CWeaponM9 &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponM9, DT_WeaponM9)

BEGIN_NETWORK_TABLE(CWeaponM9, DT_WeaponM9)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponM9)
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(weapon_m9, CWeaponM9);
PRECACHE_WEAPON_REGISTER(weapon_m9);
REGISTER_WEAPON_DATA(WEAPON_M9, WEAPONTYPE_SECONDARY, weapon_m9, "M9");
REGISTER_WEAPON_DATAHELPER(WEAPON_M9, BallisticWeaponInfo_t);