//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "weapon_lmg_base.h"
#include "weapon_defines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponRPK C_WeaponRPK
#endif

//=========================================================
//=========================================================
class CWeaponRPK : public CWeaponLMGBase
{
public:
	DECLARE_CLASS(CWeaponRPK, CWeaponLMGBase);

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	CWeaponRPK() { }
	 
	int GetWeaponID(void) const	{ return WEAPON_RPK; }

private:
	CWeaponRPK(const CWeaponRPK &);
}; 

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponRPK, DT_WeaponRPK)

BEGIN_NETWORK_TABLE(CWeaponRPK, DT_WeaponRPK)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponRPK)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_rpk, CWeaponRPK);
PRECACHE_WEAPON_REGISTER(weapon_rpk);
REGISTER_WEAPON_DATA(WEAPON_RPK, WEAPONTYPE_PRIMARY, weapon_rpk, "RPK");
REGISTER_WEAPON_DATAHELPER(WEAPON_RPK, BallisticWeaponInfo_t);