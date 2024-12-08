//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "weapon_rifle_base.h"
#include "weapon_defines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL
#define CWeaponM4 C_WeaponM4
#endif

class CWeaponM4 : public CWeaponRifleBase
{
public:
	DECLARE_CLASS(CWeaponM4, CWeaponRifleBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponM4() { }
	
	int GetWeaponID(void) const { return WEAPON_M4; }

#ifdef CLIENT_DLL
    bool HasScope( void ) const { return true; }
#endif

private:
	CWeaponM4(const CWeaponM4 &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponM4, DT_WeaponM4)

BEGIN_NETWORK_TABLE(CWeaponM4, DT_WeaponM4)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponM4)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_m4, CWeaponM4);
PRECACHE_WEAPON_REGISTER(weapon_m4);
REGISTER_WEAPON_DATA(WEAPON_M4, WEAPONTYPE_PRIMARY, weapon_m4, "M4");
REGISTER_WEAPON_DATAHELPER(WEAPON_M4, BallisticWeaponInfo_t);