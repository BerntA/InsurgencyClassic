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
#define CWeaponKabar C_WeaponKabar
#endif

//=========================================================
//=========================================================
class CWeaponKabar : public CWeaponMeleeBase 
{ 
public: 
	DECLARE_CLASS(CWeaponKabar, CWeaponMeleeBase); 
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE(); 
	
	CWeaponKabar() { }

	int GetWeaponID(void) const { return WEAPON_KABAR; } 

private: 
	CWeaponKabar(const CWeaponKabar &); 
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED(WeaponKabar, DT_WeaponKabar) 
BEGIN_NETWORK_TABLE(CWeaponKabar, DT_WeaponKabar) 
END_NETWORK_TABLE() 
BEGIN_PREDICTION_DATA(CWeaponKabar) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(weapon_kabar, CWeaponKabar); 

PRECACHE_WEAPON_REGISTER(weapon_kabar);
REGISTER_WEAPON_DATA(WEAPON_KABAR, WEAPONTYPE_MELEE, weapon_kabar, "Kabar");
REGISTER_WEAPON_DATAHELPER(WEAPON_KABAR, MeleeWeaponInfo_t);