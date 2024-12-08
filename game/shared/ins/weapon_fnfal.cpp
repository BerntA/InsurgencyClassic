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
#define CWeaponFNFAL C_WeaponFNFAL
#endif

//=========================================================
//=========================================================
class CWeaponFNFAL : public CWeaponRifleBase
{
public:
	DECLARE_CLASS(CWeaponFNFAL, CWeaponRifleBase);

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponFNFAL() { }
	 
	int GetWeaponID(void) const { return WEAPON_FNFAL; }

private:
	CWeaponFNFAL(const CWeaponFNFAL &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponFNFAL, DT_WeaponFNFAL )

BEGIN_NETWORK_TABLE(CWeaponFNFAL, DT_WeaponFNFAL)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponFNFAL)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_fnfal, CWeaponFNFAL);
PRECACHE_WEAPON_REGISTER(weapon_fnfal);
REGISTER_WEAPON_DATA(WEAPON_FNFAL, WEAPONTYPE_PRIMARY, weapon_fnfal, "FN FAL");
REGISTER_WEAPON_DATAHELPER(WEAPON_FNFAL, BallisticWeaponInfo_t);