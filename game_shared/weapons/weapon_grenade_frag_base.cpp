//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_grenade_frag_base.h"

#ifdef GAME_DLL

#include "ins_utils.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CFragGrenade::Detonate( void )
{
	Explode( );
}

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CWeaponGrenadeFragBase::OnGrenadeCooked( void )
{
	CBasePlayer *pOwner = GetOwner( );

	if( !pOwner )
		return;

	Vector vecOrigin;
	CalculateThrowOrigin( vecOrigin, NULL );

	const CGrenadeData &GrenadeData = GetGrenadeData( );
	UTIL_CreateExplosion( vecOrigin, pOwner, this, GrenadeData.m_iDamage, GrenadeData.m_iDamageRadius, 0, GrenadeData.m_szDetonationSound );
}

#endif

//=========================================================
//=========================================================
void CWeaponGrenadeFragBase::FinishAttack( WeaponSound_t Sound )
{
#ifdef GAME_DLL

	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer )
		pPlayer->SendAction( PACTION_FRAGOUT );

#endif

	BaseClass::FinishAttack( Sound );
}