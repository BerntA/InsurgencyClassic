//========= Copyright © 2005, Insurgency, All rights reserved. ================//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h" 
#include "weapon_melee_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponBayonet C_WeaponBayonet
#endif

//=========================================================
//=========================================================
class CWeaponBayonet : public CWeaponMeleeBase 
{ 
public: 
	DECLARE_CLASS(CWeaponBayonet, CWeaponMeleeBase); 
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE(); 
	
	CWeaponBayonet() { }

	int GetWeaponID(void) const { return WEAPON_BAYONET; } 

private: 
	CWeaponBayonet(const CWeaponBayonet &); 
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponBayonet, DT_WeaponBayonet) 
BEGIN_NETWORK_TABLE(CWeaponBayonet, DT_WeaponBayonet) 
END_NETWORK_TABLE() 
BEGIN_PREDICTION_DATA(CWeaponBayonet) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(weapon_bayonet, CWeaponBayonet);

PRECACHE_WEAPON_REGISTER(weapon_bayonet);
REGISTER_WEAPON_DATA(WEAPON_BAYONET, WEAPONTYPE_MELEE, weapon_bayonet, "Bayonet");
REGISTER_WEAPON_DATAHELPER(WEAPON_BAYONET, MeleeWeaponInfo_t);