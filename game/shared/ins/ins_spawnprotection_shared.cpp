//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "ins_spawnprotection_shared.h"
#include "ins_gamerules.h"
#include "ins_utils.h"

#ifdef GAME_DLL

#include "ins_obj.h"
#include "hint_helper.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_spawnprotection, CSpawnProtection );

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( SpawnProtection, DT_SpawnProtection )

BEGIN_NETWORK_TABLE( CSpawnProtection, DT_SpawnProtection )

#ifdef GAME_DLL

	SendPropInt( SENDINFO( m_iObjID ) ),
	SendPropVector( SENDINFO( m_vecStop ) ),
	SendPropInt( SENDINFO( m_iStopLength ) ),

	SendPropVector( SENDINFO( m_vecPlaneNormal ) ),

#else

	RecvPropInt( RECVINFO( m_iObjID ) ),
	RecvPropVector( RECVINFO( m_vecStop ) ),
	RecvPropInt( RECVINFO( m_iStopLength ) ),

	RecvPropVector( RECVINFO( m_vecPlaneNormal ) ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
#ifdef GAME_DLL

BEGIN_DATADESC( CSpawnProtection )

	DEFINE_KEYFIELD( m_iObjID, FIELD_INTEGER, "parentobj" ),

END_DATADESC( )

//=========================================================
//=========================================================
bool CSpawnProtection::Init( void )
{
	return UTIL_ValidObjectiveID( m_iObjID );
}

//=========================================================
//=========================================================
void CSpawnProtection::PlayerStartTouch( CINSPlayer *pPlayer )
{
	CINSRules *pRules = INSRules( );

	if( !pRules )
		return;

	// make sure its an active spawn
	/*int iT1CurrentSpawn, iT2CurrentSpawn;
	INSRules()->GetRunningMode()->GetCurrentSpawns( iT1CurrentSpawn, iT2CurrentSpawn );

	if( iT1CurrentSpawn + 1 != m_iObjID && iT2CurrentSpawn + 1 != m_iObjID )
		return;*/

	// check teams (when the teams are the same, let him through ... no questions asked)
	CINSObjective *pObjective = pRules->GetUnorderedObjective( m_iObjID );

	if( !pObjective || !pObjective->IsValid( ) )
		return;

	if( pPlayer->GetTeamID( ) == pObjective->GetCapturedTeam( ) )
		return;

	UTIL_SendHint( pPlayer, HINT_SPAWNPROTECTION );

	pPlayer->SetClippingEntity( this );

#ifdef _DEBUG

	Assert( m_Players.Find( pPlayer ) == m_Players.InvalidIndex( ) );
	m_Players.AddToTail( pPlayer );

#endif
}

//=========================================================
//=========================================================
void CSpawnProtection::PlayerEndTouch( CINSPlayer *pPlayer )
{
	pPlayer->RemoveClippingEntity( );

#ifdef _DEBUG

	m_Players.FindAndRemove( pPlayer );

#endif
}

//=========================================================
//=========================================================
void CSpawnProtection::FoundProtectedSurface( Vector &vecNormal, float flPerpLength, Vector &vecCentre )
{
	m_vecPlaneNormal = vecNormal;
	m_vecStop = vecCentre;
	m_iStopLength = RoundFloatToInt( flPerpLength );
}

#endif

//=========================================================
//=========================================================
void CSpawnProtection::ClampMoveVelocity( CINSPlayer *pPlayer, Vector &vecVelocity )
{
	float flDistance = GetInternalPlaneDistance( m_vecPlaneNormal, m_vecStop, pPlayer->GetAbsOrigin( ) );

	if( flDistance > m_iStopLength )
		return;

	if( UTIL_IsMoving( vecVelocity ) )
	{
		if( DotProduct( vecVelocity, m_vecPlaneNormal ) <= 0 )
			return;
	}
	else
	{
		Vector vecFacing;
		AngleVectors( pPlayer->GetAbsAngles( ), &vecFacing );

		if( DotProduct( vecFacing, m_vecPlaneNormal ) <= 0 )
			return;
	}

	if( flDistance < 5.0f )
	{
		vecVelocity = vec3_origin;
		return;
	}

	vecVelocity *= ( flDistance / m_iStopLength );
}