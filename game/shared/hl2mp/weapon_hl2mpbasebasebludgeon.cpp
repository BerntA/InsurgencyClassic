//=========       Copyright © Reperio Studios 2013-2018 @ Bernt Andreas Eide!       ============//
//
// Purpose: Melee HL2MP base wep class.
//
//==============================================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasebasebludgeon.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "GameBase_Shared.h"
#include "npcevent.h"

#if defined( CLIENT_DLL )
#include "c_hl2mp_player.h"
#else
#include "hl2mp_player.h"
#include "ilagcompensationmanager.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(BaseHL2MPBludgeonWeapon, DT_BaseHL2MPBludgeonWeapon)

BEGIN_NETWORK_TABLE(CBaseHL2MPBludgeonWeapon, DT_BaseHL2MPBludgeonWeapon)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CBaseHL2MPBludgeonWeapon)
END_PREDICTION_DATA()

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CBaseHL2MPBludgeonWeapon::CBaseHL2MPBludgeonWeapon()
{
	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the weapon
//-----------------------------------------------------------------------------
void CBaseHL2MPBludgeonWeapon::Spawn(void)
{
	m_fMinRange1 = 0;
	m_fMinRange2 = 0;
	m_fMaxRange1 = 64;
	m_fMaxRange2 = 64;
	
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Precache the weapon
//-----------------------------------------------------------------------------
void CBaseHL2MPBludgeonWeapon::Precache(void)
{
	BaseClass::Precache();
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CBaseHL2MPBludgeonWeapon::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	BaseClass::HandleWeaponSelectionTime();
	BaseClass::MeleeAttackUpdate();

	bool bCanAttack = ((m_flNextPrimaryAttack <= gpGlobals->curtime) && (m_flNextSecondaryAttack <= gpGlobals->curtime));

	if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		if (CanDoPrimaryAttack())
			PrimaryAttack();
	}
	else if ((pOwner->m_nButtons & IN_ATTACK2) && bCanAttack)
	{
		if (CanDoSecondaryAttack())
			SecondaryAttack();
	}
	else
	{
		WeaponIdle();
	}

	if ((m_flNextSecondaryAttack < gpGlobals->curtime) && (m_flMeleeCooldown > 0.0f))
		m_flMeleeCooldown = 0.0f;
}

float CBaseHL2MPBludgeonWeapon::GetFireRate(void)
{
	return GetWpnData().m_flFireRate;
}

float CBaseHL2MPBludgeonWeapon::GetRange(void)
{
	return ((float)GetWpnData().m_iRangeMax);
}

float CBaseHL2MPBludgeonWeapon::GetSpecialAttackDamage(void)
{
	return GetSpecialDamage();
}

float CBaseHL2MPBludgeonWeapon::GetDamageForActivity(Activity hitActivity)
{
	float flNewDamage = GetHL2MPWpnData().m_iPlayerDamage;
	
	CHL2MP_Player *pClient = ToHL2MPPlayer(this->GetOwner());
	if (pClient)
	{
		int iTeamBonus = (pClient->m_BB2Local.m_iPerkTeamBonus - 5);
		flNewDamage += ((flNewDamage / 100.0f) * (((float)pClient->GetSkillValue(PLAYER_SKILL_HUMAN_MELEE_MASTER)) * GetWpnData().m_flSkillDamageFactor));

		if (pClient->IsPerkFlagActive(PERK_HUMAN_BLOODRAGE))
			flNewDamage += ((flNewDamage / 100.0f) * (pClient->GetSkillValue(PLAYER_SKILL_HUMAN_BLOOD_RAGE, TEAM_HUMANS)));

		if (iTeamBonus > 0)
			flNewDamage += ((flNewDamage / 100.0f) * (iTeamBonus * GameBaseShared()->GetSharedGameDetails()->GetPlayerSharedData()->iTeamBonusDamageIncrease));
	}

	bool bSpecialAttack = (hitActivity >= ACT_VM_SPECIALATTACK0 && hitActivity <= ACT_VM_SPECIALATTACK10);
	if (bSpecialAttack)
		flNewDamage += GetSpecialAttackDamage();

	return flNewDamage;
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CBaseHL2MPBludgeonWeapon::PrimaryAttack()
{
	CHL2MP_Player *pOwner = ToHL2MPPlayer(GetOwner());
	if (!pOwner)
		return;

	int swingAct = GetCustomActivity(false);
	WeaponSound(SINGLE);
	SendWeaponAnim(swingAct);
	pOwner->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY, swingAct);

	// Setup our next attack times GetFireRate() <- old
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CBaseHL2MPBludgeonWeapon::SecondaryAttack()
{
	CHL2MP_Player *pOwner = ToHL2MPPlayer(GetOwner());
	if (!pOwner)
		return;

	int swingAct = GetCustomActivity(true);
	WeaponSound(SINGLE);
	SendWeaponAnim(swingAct);
	pOwner->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY, swingAct);

	// Setup our next attack times GetFireRate() <- old
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + SpecialPunishTime() + GetViewModelSequenceDuration();
	m_flMeleeCooldown = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseHL2MPBludgeonWeapon::ImpactEffect(trace_t &traceHit)
{
	// See if we hit water (we don't do the other impact effects in this case)
	if (ImpactWater(traceHit.startpos, traceHit.endpos))
		return;

	UTIL_ImpactTrace(&traceHit, GetMeleeDamageType());
}

Activity CBaseHL2MPBludgeonWeapon::GetCustomActivity(int bIsSecondary)
{
	int iActivity = ((bIsSecondary >= 1) ? ACT_VM_SPECIALATTACK0 : ACT_VM_PRIMARYATTACK0);

	CHL2MP_Player *pOwner = ToHL2MPPlayer(GetOwner());
	if (pOwner && (pOwner->GetTeamNumber() == TEAM_HUMANS))
		iActivity += pOwner->GetSkillValue(PLAYER_SKILL_HUMAN_MELEE_SPEED);

	if (HL2MPRules() && HL2MPRules()->IsFastPacedGameplay())
		iActivity = ((bIsSecondary >= 1) ? ACT_VM_SPECIALATTACK10 : ACT_VM_PRIMARYATTACK10);

	return (Activity)iActivity;
}

float CBaseHL2MPBludgeonWeapon::SpecialPunishTime()
{
	return GetWpnData().m_flSecondaryAttackCooldown;
}

void CBaseHL2MPBludgeonWeapon::AddViewKick(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	QAngle punchAng;

	punchAng.x = SharedRandomFloat("crowbarpax", 1.0f, 2.0f);
	punchAng.y = SharedRandomFloat("crowbarpay", -2.0f, -1.0f);
	punchAng.z = 0.0f;

	pPlayer->ViewPunch(punchAng);
}

#ifndef CLIENT_DLL
int CBaseHL2MPBludgeonWeapon::GetMeleeSkillFlags(void)
{
	return 0;
}
#endif