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
#define CWeaponL42A1 C_WeaponL42A1
#endif

//=========================================================
//=========================================================
class CWeaponL42A1 : public CWeaponSniperBase
{
public:
	DECLARE_CLASS(CWeaponL42A1, CWeaponSniperBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
public:
	CWeaponL42A1() { }

	int GetWeaponID(void) const { return WEAPON_L42A1;  }

	bool IsBoltAction(void) const { return true; }
	bool IsSingleLoad(void) const { return true; }

	bool AllowBipod( void ) const { return false; }

private:
	CWeaponL42A1(const CWeaponL42A1 &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponL42A1, DT_WeaponL42A1)

BEGIN_NETWORK_TABLE(CWeaponL42A1, DT_WeaponL42A1)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponL42A1)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_l42a1, CWeaponL42A1);
PRECACHE_WEAPON_REGISTER(weapon_l42a1);
REGISTER_WEAPON_DATA(WEAPON_L42A1, WEAPONTYPE_PRIMARY, weapon_l42a1, "L42A1");
REGISTER_WEAPON_DATAHELPER(WEAPON_L42A1, BallisticWeaponInfo_t);