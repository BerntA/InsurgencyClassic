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
#define CWeaponM16A4 C_WeaponM16A4 
#endif

//=========================================================
//=========================================================
class CWeaponM16A4 : public CWeaponRifleBase
{
public:
	DECLARE_CLASS( CWeaponM16A4, CWeaponRifleBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponM16A4() { }
	
	int GetWeaponID(void) const	{ return WEAPON_M16A4; }

private:
	CWeaponM16A4(const CWeaponM16A4 &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponM16A4, DT_WeaponM16A4)

BEGIN_NETWORK_TABLE(CWeaponM16A4, DT_WeaponM16A4)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponM16A4)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_m16a4, CWeaponM16A4);
PRECACHE_WEAPON_REGISTER(weapon_m16a4);
REGISTER_WEAPON_DATA(WEAPON_M16A4, WEAPONTYPE_PRIMARY, weapon_m16a4, "M16A4");
REGISTER_WEAPON_DATAHELPER(WEAPON_M16A4, BallisticWeaponInfo_t);