//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "interface.h"
#include "ins_stats_shared.h"
#include "ins_player.h"
#include "ins_gamerules.h"
#include "stats_protection.h"
#include "statsman.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
ConVar globalstats( "ins_globalstats", "0", FCVAR_ARCHIVE, "", true, STATSTYPE_NONE, true, STATSTYPE_ENABLED );
ConVar stats_username( "ins_stats_username", "", FCVAR_ARCHIVE | FCVAR_PROTECTED, "" );
ConVar stats_password( "ins_stats_password", "", FCVAR_ARCHIVE | FCVAR_PROTECTED, "" );

//=========================================================
//=========================================================
void CINSStats::LogoutPlayers( void )
{
	if( !IsValid( ) )
		return;

	/*// collect stats data
	PlayerUpdateData_t PlayerUpdateData;
	int iElementID = 0;

	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CINSPlayer *pPlayer = ToINSPlayer( UTIL_PlayerByIndex( i ) );

		if( !pPlayer || !pPlayer->IsUsingStats( ) )
			continue;

		pPlayer->StatsLogout( );

		PlayerUpdateData[ iElementID ].m_iMemberID = pPlayer->GetStatsMemberID( );
		PlayerUpdateData[ iElementID ].m_PlayerStats = pPlayer->GetAggregateStats( );

		iElementID++;
	}

	// update
	m_pStatsMan->Update( PlayerUpdateData );

	// remove all
	m_CurrentMemberIDs.RemoveAll( );*/
}

//=========================================================
//=========================================================
void CINSStats::LogoutPlayer( CINSPlayer *pPlayer )
{
	if( !IsValid( ) )
		return;

	/*// log him out
	pPlayer->StatsLogout( );

	// update
	m_pStatsMan->Update( pPlayer->GetStatsMemberID( ), pPlayer->GetLiveStats( ) );

	// remove him
	m_CurrentMemberIDs.FindAndRemove( iMemberID );*/
}
