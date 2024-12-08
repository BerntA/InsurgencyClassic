
#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_pistol_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CWeaponPistolBase::CWeaponPistolBase()
{
}

float CWeaponPistolBase::GetCycleTime(void)
{
	// how fast can you pull a trigger?
	return 0.125f;
}

float CWeaponPistolBase::GetBurstCycleTime(void) const
{
	return 0.08f;
}

Activity CWeaponPistolBase::GetPrimaryAttackActivityRecoil(Activity BaseAct,int iRecoilState) const
{
	if(iRecoilState == 3)
		return ACT_VM_RECOIL3;

	return BaseClass::GetPrimaryAttackActivityRecoil(BaseAct, iRecoilState);
}

int CWeaponPistolBase::GetShellType( void ) const
{
	return INS_SHELL_PISTOL;
}

#define VM_PISTOL_LENGTH 9.0f
#define VM_PISTOL_HALFWIDTH 1.5f

void CWeaponPistolBase::CalcViewmodelInteraction( float &flLength, float &flHalfWidth )
{
	flLength = VM_PISTOL_LENGTH;
	flHalfWidth = VM_PISTOL_HALFWIDTH;
}