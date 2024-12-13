//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_touch.h"
#include "ins_player_shared.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
void CINSTouch::Spawn( void )
{
	SetSolid( SOLID_BSP );
	SetMoveType( MOVETYPE_NONE );
	SetModel( STRING( GetModelName( ) ) );

	if( NotSolidSpawn( ) )
		MakeNotSolid( );

	if( TouchSetupSpawn( ) )
		EnableTouch( );

	if( HideBrush( ) )
		AddEffects( EF_NODRAW );
}

//=========================================================
//=========================================================
bool CINSTouch::HideBrush( void ) const
{
#ifdef _DEBUG

	return false;

#else

	return true;

#endif
}

//=========================================================
//=========================================================
void CINSTouch::MakeNotSolid( void )
{
	AddSolidFlags( FSOLID_NOT_SOLID );
}

//=========================================================
//=========================================================
void CINSTouch::EnableTouch( void )
{
	AddSolidFlags( FSOLID_TRIGGER );
	PhysicsTouchTriggers( );
}

//=========================================================
//=========================================================
void CINSTouch::DisableTouch( void )
{
	RemoveSolidFlags( FSOLID_TRIGGER );
}

//=========================================================
//=========================================================
void CINSTouch::StartTouch( CBaseEntity *pOther )
{
	TouchHook( pOther, &CINSTouch::PlayerStartTouch );
}

//=========================================================
//=========================================================
void CINSTouch::EndTouch( CBaseEntity *pOther )
{
	TouchHook( pOther, &CINSTouch::PlayerEndTouch );
}

//=========================================================
//=========================================================
void CINSTouch::TouchHook( CBaseEntity *pOther, INSTouchCall_t TouchCall )
{
	// don't do anything when we're not running
	CINSRules *pRules = INSRules( );

	if( !pRules || !pRules->IsModeRunning( ) )
		return;

	// make sure its a player thats touched
	if( !pOther->IsPlayer( ) )
		return;

	// find INS player
	CINSPlayer *pPlayer = ToINSPlayer( pOther );

	// make sure they're a valid team
	if( pPlayer->OnPlayTeam( ) )
		( *this.*TouchCall )( pPlayer );
}
