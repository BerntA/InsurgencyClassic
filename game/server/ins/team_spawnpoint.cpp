//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team spawnpoint handling
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "team_spawnpoint.h"
#include "imc_format.h"
#include "play_team_shared.h"
#include "ins_player_shared.h"
#include "ins_squad_shared.h"
#include "ins_shared.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
BEGIN_DATADESC( CSpawnPoint )

	DEFINE_KEYFIELD( m_iParentObjective, FIELD_INTEGER, "parentobj" ),
	DEFINE_KEYFIELD( m_iSpawnGroup, FIELD_INTEGER, "spawngroup" ),
	DEFINE_KEYFIELD( m_iTeam, FIELD_INTEGER, "team" ),
	DEFINE_KEYFIELD( m_iSquad, FIELD_INTEGER, "squad" ),
	DEFINE_KEYFIELD( m_bReinforcement, FIELD_BOOLEAN, "reinforcement" ),

END_DATADESC( )

LINK_ENTITY_TO_CLASS( info_player_start, CPointEntity );
LINK_ENTITY_TO_CLASS( ins_viewpoint, CViewPoint );
LINK_ENTITY_TO_CLASS( ins_spawnpoint, CSpawnPoint );

//=========================================================
//=========================================================
CUtlVector< CViewPoint* > CViewPoint::m_Points;

CSpawnPoint *CSpawnPoint::m_pFirstPoint = NULL;
CUtlVector< CSpawnPoint* > CSpawnPoint::m_Points;

//=========================================================
//=========================================================
CViewPoint *CViewPoint::GetPoint( int iID )
{
	Assert( m_Points.IsValidIndex( iID ) );
	return m_Points[ iID ];
}

//=========================================================
//=========================================================
int CViewPoint::CountPoints( void )
{
	return m_Points.Count( );
}

//=========================================================
//=========================================================
void CViewPoint::CleanPoints( void )
{
	m_Points.Purge( );
}

//=========================================================
//=========================================================
void CViewPoint::Activate( void )
{
	BaseClass::Activate( );

	m_Points.AddToTail( this );
}

//=========================================================
//=========================================================
CSpawnPoint::CSpawnPoint( )
{
	m_iParentObjective = 0;
	m_iSpawnGroup = 0;
	m_iTeam = TEAM_ONE;
	m_iSquad = INVALID_SQUAD;
	m_bReinforcement = false;
	m_iStance = STANCE_STAND;
}

//=========================================================
//=========================================================
CSpawnPoint *CSpawnPoint::GetFirstSpawn( void )
{
	return m_pFirstPoint;
}

//=========================================================
//=========================================================
CSpawnPoint *CSpawnPoint::GetSpawn( int iID )
{
	Assert( m_Points.IsValidIndex( iID ) );
	return m_Points[ iID ];
}

//=========================================================
//=========================================================
int CSpawnPoint::CountSpawns( void )
{
	return m_Points.Count( );
}

//=========================================================
//=========================================================
void CSpawnPoint::CleanSpawns( void )
{
	m_pFirstPoint = NULL;
	m_Points.Purge( );
}

//=========================================================
//=========================================================
void CSpawnPoint::TrimSpawn( CSpawnPoint *pSpawnPoint )
{
	int iID = m_Points.Find( pSpawnPoint );
	
	if( !m_Points.IsValidIndex( iID ) )
	{
		Assert( false );
		return;
	}

	m_Points.Remove( iID );
}

//=========================================================
//=========================================================
bool CSpawnPoint::KeyValue( const char *szKeyName, const char *szValue )
{
	if( FStrEq( szKeyName, "stance" ) )
	{
		if( atoi( szValue ) == 1 )
			m_iStance = STANCE_PRONE;
		else
			m_iStance = STANCE_STAND;

		return true;
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}

//=========================================================
//=========================================================
void CSpawnPoint::Activate( void )
{
	BaseClass::Activate( );

	// PNOTE: once upon a time the FGD worked with -1
	// in a number of places ... so here some legacy hacks
	m_iSpawnGroup = min( 0, m_iSpawnGroup );

	// PNOTE: shift it all by one and clamp
	if( m_iSquad <= 0 )
		m_iSquad = INVALID_SQUAD;
	else
		m_iSquad = clamp( m_iSquad - 1, 0, MAX_SQUADS - 1 );

	if( IsValid( ) )
	{
		m_Points.AddToTail( this );

		m_pFirstPoint = this;
	}
}

//=========================================================
//=========================================================
CINSObjective *CSpawnPoint::GetParent( void ) const
{
	return INSRules( )->GetUnorderedObjective( m_iParentObjective );
}

//=========================================================
//=========================================================
void CSpawnPoint::KillColliding( CINSPlayer *pPlayer ) const
{
	CBaseEntity *pEntity = NULL;

	// kill any players inside
	for( CEntitySphereQuery sphere( GetAbsOrigin( ), SPAWNCOLLIDE_DISTANCE ); ( pEntity = sphere.GetCurrentEntity( ) ) != NULL; sphere.NextEntity( ) )
	{
		if( pEntity && pEntity->IsPlayer( ) && pEntity != pPlayer )
			pPlayer->CommitSuicide( true );
	}
}

//=========================================================
//=========================================================
bool CSpawnPoint::IsValid( void ) const
{
	// spawngroups must be >= 0
	if( m_iSpawnGroup < 0 )
		return false;

	// either a playteam or mixed
	if( m_iTeam != 0 && !IsPlayTeam( m_iTeam ) )
		return false;

	// either a squad or mixed
	if( m_iSquad != INVALID_SQUAD && !CINSSquad::IsValidSquad( m_iSquad ) )
		return false;

	return true;
}

//=========================================================
//=========================================================
void UTIL_CleanSpawnPoints( void )
{
	CViewPoint::CleanPoints( );
	CSpawnPoint::CleanSpawns( );
}

//=========================================================
//=========================================================
CSpawnManager::CSpawnManager( )
{
	m_iLastSpawnID = 0;
}

//=========================================================
//=========================================================
bool CSpawnManager::IsValid( void ) const
{
	return ( Count( ) != 0 );
}

//=========================================================
//=========================================================
CSpawnPoint *CSpawnManager::FindSpawn( CINSPlayer *pPlayer ) 
{
	if( !IsValid( ) )
	{
		Assert( false );
		return NULL;
	}

	m_iLastSpawnID++;

	if( m_iLastSpawnID >= Count( ) )
		m_iLastSpawnID = 0;

	int iCurrentID = m_iLastSpawnID;
	bool bStarted = false;

	while( true )
	{
		if( iCurrentID >= Count( ) )
			iCurrentID = 0;

		if( bStarted && m_iLastSpawnID == iCurrentID )
			break;

		CSpawnPoint *pSpawn = GetSpawn( iCurrentID );
		bStarted = true;

		if( !pSpawn )
		{
			iCurrentID++;
			continue;
		}

		m_iLastSpawnID = iCurrentID;

	#ifdef _DEBUG

		Warning( "SPAWN: Found Spawn: (%i/%i)\n", iCurrentID, Count( ) );

	#endif

		return pSpawn;
	}

	AssertMsg( false, "Not Enough Spawns" );
	return NULL;
}

//=========================================================
//=========================================================
CSpawnPoint *CSpawnGlobal::GetSpawn( int iID ) const
{
	return CSpawnPoint::GetSpawn( iID );
}

int CSpawnGlobal::Count( void ) const
{
	return CSpawnPoint::CountSpawns( );
}

//=========================================================
//=========================================================
void CSpawnGroup::AddSpawn( int iID )
{
	m_Spawns.AddToTail( iID );
}

bool CSpawnGroup::IsValid( CINSObjective *pParent, int iTeamID, int iSquadID ) const
{
	Assert( pParent );

	// skip when not needed to check it
	if( !pParent->CheckSpawnGroup( iTeamID, iSquadID ) )
		return true;

	// find the required number
	int iRequiredNum;

	if( iSquadID != INVALID_TEAM )
		iRequiredNum = MAX_SQUAD_SLOTS;
	else
		iRequiredNum = MAX_TEAM_SLOTS;

	return ( Count( ) >= iRequiredNum );
}

CSpawnPoint *CSpawnGroup::GetSpawn( int iID ) const
{
	Assert( m_Spawns.IsValidIndex( iID ) );
	return CSpawnGlobal::GetSpawn( m_Spawns[ iID ] );
}

int CSpawnGroup::Count( void ) const
{
	return m_Spawns.Count( );
}
