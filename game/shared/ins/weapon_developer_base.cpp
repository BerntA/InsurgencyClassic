//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_developer_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
bool CWeaponDeveloperBase::CanDeploy( void )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );
	return ( pPlayer && pPlayer->IsDeveloper( ) );
}