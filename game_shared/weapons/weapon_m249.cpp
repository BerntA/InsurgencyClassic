//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "weapon_lmg_belt_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponM249 C_WeaponM249
#endif

//=========================================================
//=========================================================
class CWeaponM249 : public CWeaponLMGBeltBase
{
public:
	DECLARE_CLASS(CWeaponM249, CWeaponLMGBeltBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponM249() { }
	 
	int GetWeaponID(void) const { return WEAPON_M249; }

private:
	CWeaponM249(const CWeaponM249 &);

#ifdef GAME_DLL
	DECLARE_ACTTABLE();
#endif
}; 

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponM249, DT_WeaponM249)

BEGIN_NETWORK_TABLE(CWeaponM249, DT_WeaponM249)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponM249)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_m249, CWeaponM249);
PRECACHE_WEAPON_REGISTER(weapon_m249);
REGISTER_WEAPON_DATA(WEAPON_M249, WEAPONTYPE_PRIMARY, weapon_m249, "M249");
REGISTER_WEAPON_DATAHELPER(WEAPON_M249, LMGBaseWeaponInfo_t);