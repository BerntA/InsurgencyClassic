//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_LMG_BELT_BASE_H
#define WEAPON_LMG_BELT_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_lmg_base.h"
 
#ifdef CLIENT_DLL
#define CWeaponLMGBeltBase C_WeaponLMGBeltBase
#endif

//=========================================================
//=========================================================
class LMGBaseWeaponInfo_t : public BallisticWeaponInfo_t
{
	typedef BallisticWeaponInfo_t BaseClass;

public:
	LMGBaseWeaponInfo_t();

	virtual void Parse(KeyValues *pKeyValuesData, const char *pszWeaponName);

public:
	int iVisibleBelt;
};

//=========================================================
//=========================================================
class CWeaponLMGBeltBase : public CWeaponLMGBase
{
public:
	DECLARE_CLASS(CWeaponLMGBeltBase, CWeaponLMGBase);
	 
public:
	CWeaponLMGBeltBase();

	virtual void Deploy(void);
	virtual void PrimaryAttack(void);
	virtual void Reload(void);
	virtual void FinishReload(void);

	void MiddleReload(void);

#ifdef GAME_DLL

	void HandleAnimEvent(animevent_t *pEvent);

#else

	bool OnFireEvent(C_BaseViewModel *pViewModel, const Vector& origin, const QAngle& angles, int event, const char *options);

#endif

private:
	void UpdateBelt(int iForceClip = -1);

private:
	int m_iOldBulletID;
}; 

#endif // WEAPON_LMG_BELT_BASE_H



