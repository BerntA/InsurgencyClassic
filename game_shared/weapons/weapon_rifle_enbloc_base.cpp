//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rifle_enbloc_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
bool CWeaponRifleEnblocBase::CanReload( void )
{
	if( !BaseClass::CanReload( ) )
		return false;

	return ( m_iClip == 0 );
}

//=========================================================
//=========================================================
/*Activity CWeaponRifleEnblocBase::GetEmptyReloadActivity(void) const
{
	return ACT_VM_RELOADEMPTY;
}*/