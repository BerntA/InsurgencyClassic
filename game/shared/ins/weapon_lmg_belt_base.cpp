//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_lmg_belt_base.h"
#include "keyvalues.h"
#include "npcevent.h"
#include "clipdef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
LMGBaseWeaponInfo_t::LMGBaseWeaponInfo_t()
{
	iVisibleBelt = 0;
}

//=========================================================
//=========================================================
void LMGBaseWeaponInfo_t::Parse(KeyValues *pKeyValuesData, const char *pszWeaponName)
{
	BaseClass::Parse(pKeyValuesData, pszWeaponName);
	iVisibleBelt = pKeyValuesData->GetInt("visible_belt", 0);
}

//=========================================================
//=========================================================
CWeaponLMGBeltBase::CWeaponLMGBeltBase()
{
}

//=========================================================
//=========================================================
void CWeaponLMGBeltBase::Deploy()
{
	BaseClass::Deploy();

	m_iOldBulletID = -1;

	UpdateBelt();
}

//=========================================================
//=========================================================
void CWeaponLMGBeltBase::PrimaryAttack( void )
{
	BaseClass::PrimaryAttack( );

	UpdateBelt( );
}

//=========================================================
//=========================================================
void CWeaponLMGBeltBase::Reload( void )
{
	BaseClass::Reload( );

	// UpdateBelt(GET_WEAPON_DATA_CUSTOM(LMGBaseWeaponInfo_t, iVisibleBelt));
}

//=========================================================
//=========================================================
void CWeaponLMGBeltBase::FinishReload(void)
{
	BaseClass::FinishReload();

	UpdateBelt();
}

//=========================================================
//=========================================================
#define BULLETS_GROUP "bullets"

void CWeaponLMGBeltBase::UpdateBelt(int iForceClip)
{
	int iVisibleBelt = GET_WEAPON_DATA_CUSTOM(LMGBaseWeaponInfo_t, iVisibleBelt);

	// no belt?
	if(iVisibleBelt == 0)
		return;

	int iClip = (iForceClip != -1) ? iForceClip : m_iClip;

	int iBulletID = iVisibleBelt - iClip;
	iBulletID = clamp(iBulletID, 0, iVisibleBelt);

	// don't bother setting something thats the same
	if(m_iOldBulletID == iBulletID)
		return;

	CBasePlayer *pPlayer = GetOwner();

	if(!pPlayer)
		return;
	
	CBaseViewModel *pViewModel = pPlayer->GetViewModel();
	
	if(!pViewModel)
		return;

	// now, set the bullets part
	int iBulletsGroup = pViewModel->FindBodygroupByName(BULLETS_GROUP);

	if(iBulletsGroup == -1)
		return;

	pViewModel->SetBodygroup(iBulletsGroup, iBulletID);
	m_iOldBulletID = iBulletID;
}

//=========================================================
//=========================================================
void CWeaponLMGBeltBase::MiddleReload(void)
{
	CINSPlayer *pPlayer = GetINSPlayerOwner();

	if( !pPlayer )
		return;

	UpdateBelt( NextClip( ) );
}

//=========================================================
//=========================================================
#define UPDATEBELT_EVENT 1337

#ifdef GAME_DLL

void CWeaponLMGBeltBase::HandleAnimEvent(animevent_t *pEvent)
{
	if(pEvent->event == UPDATEBELT_EVENT)
		MiddleReload();
	else
		BaseClass::HandleAnimEvent(pEvent);
}

#else

//=========================================================
//=========================================================
bool CWeaponLMGBeltBase::OnFireEvent(C_BaseViewModel *pViewModel, const Vector& origin, const QAngle& angles, int event, const char *options)
{
	if(event == UPDATEBELT_EVENT)
	{
		MiddleReload();
		return true;
	}

	return false;
}

#endif