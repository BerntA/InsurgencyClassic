//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Switches to the best weapon that is also better than the given weapon.
// Input  : pCurrent - The current weapon used by the player.
// Output : Returns true if the weapon was switched, false if there was no better
//			weapon to switch to.
//-----------------------------------------------------------------------------
bool CBaseCombatCharacter::SwitchToNextBestWeapon(CBaseCombatWeapon* pCurrent)
{
#ifdef GAME_DLL
	if (pCurrent && pCurrent->IsExhaustible() && !pCurrent->HasAmmo())
	{
		RemoveWeapon(pCurrent);
		pCurrent = NULL;
	}
#endif

	CBaseCombatWeapon* pNewWeapon = GetNextBestWeapon(pCurrent);
	if (IsAlive() && (pNewWeapon != NULL) && (pNewWeapon != pCurrent))
		return Weapon_Switch(pNewWeapon);

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Switches to the given weapon (providing it has ammo)
// Input  :
// Output : true is switch succeeded
//-----------------------------------------------------------------------------
bool CBaseCombatCharacter::Weapon_Switch(CBaseCombatWeapon* pWeapon, bool bForce)
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether or not we can switch to the given weapon.
// Input  : pWeapon - 
//-----------------------------------------------------------------------------
bool CBaseCombatCharacter::Weapon_CanSwitchTo(CBaseCombatWeapon* pWeapon)
{
	if (!pWeapon || !pWeapon->CanDeploy())
		return false;

	CBaseCombatWeapon* pActiveWeapon = m_hActiveWeapon;
	if (pActiveWeapon && !pActiveWeapon->CanHolster())
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBaseCombatWeapon
//-----------------------------------------------------------------------------
CBaseCombatWeapon* CBaseCombatCharacter::GetNextBestWeapon(CBaseCombatWeapon* pCurrentWeapon)
{
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBaseCombatWeapon
//-----------------------------------------------------------------------------
CBaseCombatWeapon* CBaseCombatCharacter::GetActiveWeapon() const
{
	return m_hActiveWeapon;
}

//-----------------------------------------------------------------------------
// Purpose: Returns weapon if already owns a weapon of this class
//-----------------------------------------------------------------------------
CBaseCombatWeapon* CBaseCombatCharacter::Weapon_OwnsThisType(int iWeaponID) const
{
	for (int i = 0; i < MAX_PWEAPONS; i++)
	{
		CBaseCombatWeapon* pWeapon = m_hMyWeapons[i];
		if (pWeapon && pWeapon->GetWeaponID() == iWeaponID)
			return m_hMyWeapons[i];
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Returns weapons with the appropriate weapon slot.
//-----------------------------------------------------------------------------
CBaseCombatWeapon* CBaseCombatCharacter::Weapon_GetBySlot(int slot) const
{
	if ((slot < 0) || (slot >= MAX_PWEAPONS))
		return NULL;
	m_hMyWeapons[slot].Get();
}

int CBaseCombatCharacter::BloodColor()
{
	return m_bloodColor;
}

//-----------------------------------------------------------------------------
// Blood color (see BLOOD_COLOR_* macros in baseentity.h)
//-----------------------------------------------------------------------------
void CBaseCombatCharacter::SetBloodColor(int nBloodColor)
{
	m_bloodColor = nBloodColor;
}