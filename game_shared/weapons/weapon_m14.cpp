//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "weapon_sniper_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponM14 C_WeaponM14 
#endif

//=========================================================
//=========================================================
class CWeaponM14 : public CWeaponSniperBase
{
public:
	DECLARE_CLASS(CWeaponM14, CWeaponSniperBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
public:
	CWeaponM14() { }

	int GetWeaponID(void) const { return WEAPON_M14;  }

private:
	CWeaponM14(const CWeaponM14 &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponM14, DT_WeaponM14)

BEGIN_NETWORK_TABLE(CWeaponM14, DT_WeaponM14)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponM14 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_m14, CWeaponM14);
PRECACHE_WEAPON_REGISTER(weapon_m14);
REGISTER_WEAPON_DATA(WEAPON_M14, WEAPONTYPE_PRIMARY, weapon_m14, "M14");
REGISTER_WEAPON_DATAHELPER(WEAPON_M14, BallisticWeaponInfo_t);