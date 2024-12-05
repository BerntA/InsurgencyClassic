//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "weapon_rifle_enbloc_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponSKS C_WeaponSKS
#endif

//=========================================================
//=========================================================
class CWeaponSKS : public CWeaponRifleEnblocBase
{
public:
	DECLARE_CLASS(CWeaponSKS, CWeaponRifleEnblocBase);

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponSKS() { }
	 
	int GetWeaponID(void) const { return WEAPON_SKS; }

private:
	CWeaponSKS(const CWeaponSKS &);
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSKS, DT_WeaponSKS)

BEGIN_NETWORK_TABLE(CWeaponSKS, DT_WeaponSKS)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponSKS)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_sks, CWeaponSKS);
PRECACHE_WEAPON_REGISTER(weapon_sks);
REGISTER_WEAPON_DATA(WEAPON_SKS, WEAPONTYPE_PRIMARY, weapon_sks, "SKS");
REGISTER_WEAPON_DATAHELPER(WEAPON_SKS, BallisticWeaponInfo_t);