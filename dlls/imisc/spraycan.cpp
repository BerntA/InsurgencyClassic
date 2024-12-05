//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "spraycan.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( spraycan, CSprayCan );
PRECACHE_REGISTER( spraycan );

//=========================================================
//=========================================================
void CSprayCan::Spawn( CBasePlayer *pOwner )
{
	SetLocalOrigin( pOwner->WorldSpaceCenter( ) + Vector ( 0, 0, 32 ) );
	SetLocalAngles( pOwner->EyeAngles( ) );
	SetOwnerEntity( pOwner );
	SetNextThink( gpGlobals->curtime );
	EmitSound( "SprayCan.Paint" );
}

//=========================================================
//=========================================================
void CSprayCan::Precache( void )
{
	BaseClass::Precache( );

	PrecacheScriptSound( "SprayCan.Paint" );
}

//=========================================================
//=========================================================
void CSprayCan::Think( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwnerEntity( ) );

	Vector vecForward;
	AngleVectors( GetAbsAngles( ), &vecForward );

	trace_t	tr;
	UTIL_TraceLine( GetAbsOrigin( ), GetAbsOrigin( ) + vecForward * 128.0f, MASK_SOLID_BRUSHONLY, pPlayer, COLLISION_GROUP_NONE, &tr );

	UTIL_PlayerDecalTrace( &tr, GetOwnerEntity( )->entindex( ) );

	UTIL_Remove( this );
}

//=========================================================
//=========================================================
int	CSprayCan::ObjectCaps( void )
{
	return FCAP_DONT_SAVE;
}