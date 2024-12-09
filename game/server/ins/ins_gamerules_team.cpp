//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_gamerules.h"
#include "viewport_panel_names.h"
#include "basecombatweapon.h"
#include "game.h"
#include "voice_gamemgr.h"
#include "team_lookup.h"
#include "imc_config.h"
#include "team.h"
#include "team_spawnpoint.h"
#include "view_team.h"
#include "play_team_shared.h"
#include "ins_obj.h"
#include "ins_objmarker.h"
#include "ins_player.h"
#include "basic_colors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
ConVar squadorder( "ins_squadorder", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Defines whether Squad Selection Occurs", true, SQUADORDER_NONE, true, SQUADORDER_ALWAYS );

//=========================================================
//=========================================================
int CINSRules::CalculateAutoAssign( void ) const
{
	CPlayTeam *pT1, *pT2;
	pT1 = GetGlobalPlayTeam( TEAM_ONE );
	pT2 = GetGlobalPlayTeam( TEAM_TWO );

	int iDesiredTeam;

	if( pT1->GetNumPlayers( ) != pT2->GetNumPlayers( ) )
		iDesiredTeam = ( pT1->GetNumPlayers( ) < pT2->GetNumPlayers( ) ) ? TEAM_ONE : TEAM_TWO;
	else
		iDesiredTeam = random->RandomInt( TEAM_ONE, TEAM_TWO );
	
	CPlayTeam *pDesiredTeam = pDesiredTeam = GetGlobalPlayTeam( iDesiredTeam );

	if( !pDesiredTeam->IsFull( ) )
		return iDesiredTeam;

	pDesiredTeam = GetGlobalPlayTeam( FlipPlayTeam( iDesiredTeam ) );

	if( !pDesiredTeam->IsFull( ) )
		return iDesiredTeam;

	return TEAM_SPECTATOR;
}

//=========================================================
//=========================================================
bool CINSRules::ShouldBeWaitingForPlayers( void ) const
{
	return ( GetGlobalPlayTeam( TEAM_ONE )->GetNumPlayers( ) == 0 || GetGlobalPlayTeam( TEAM_TWO )->GetNumPlayers( ) == 0 );
}

//=========================================================
//=========================================================
int CINSRules::GetTotalTeamScore( void ) const
{
	int iTotalTeamScore = 0;

	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		iTotalTeamScore += GetGlobalPlayTeam( i )->GetScore( );

	return iTotalTeamScore;
}

//=========================================================
//=========================================================
Color CINSRules::CalculateTeamColor( int iType, bool bBackup )
{
	if( iType == TEAMTYPE_CONVENTIONAL )
		return bBackup ? COLOR_YELLOW : COLOR_BLUE;
	else
		return bBackup ? COLOR_GREEN : COLOR_RED;
}

//=========================================================
//=========================================================
void CINSRules::EchoNoScoringMessage( void ) const
{
	
}

//=========================================================
//=========================================================
#define PLAYERWAIT_IGNORE_TIME 15

void CINSRules::HandlePlayerCount( void )
{
	// when there aren't any players ... set the game to idle
	if( CPlayTeam::GetTotalPlayerCount( ) == 0 )
	{
		SetMode( GRMODE_IDLE );
		return;
	}	

	CurrentMode( )->HandlePlayerCount( );
}

//=========================================================
//=========================================================
void CINSRules::SetupPlayerTeam( CINSPlayer *pPlayer, int iNewTeam )
{
	if( !pPlayer || !CanChangeTeam( pPlayer ) )
		return;

	int iDesiredTeam;

	// correct the new Team		
	switch( iNewTeam )
	{
		case TEAMSELECT_ONE:
			iDesiredTeam = TEAM_ONE;
			break;

		case TEAMSELECT_TWO:
			iDesiredTeam = TEAM_TWO;
			break;

		case TEAMSELECT_AUTOASSIGN:
			iDesiredTeam = CalculateAutoAssign( );
			break;

		case TEAMSELECT_SPECTATOR:
			iDesiredTeam = TEAM_SPECTATOR;
			break;

		default:
			return;
	}

	// abort if original team
	if( pPlayer->GetTeamID( ) == iDesiredTeam )
		return;

	// if they want to be spectator, execute now
	bool bJoiningPlayTeam = ( iDesiredTeam != TEAM_SPECTATOR );

	if( bJoiningPlayTeam )
	{
		if( !ValidJoinTeam( iDesiredTeam ) )
		{
			pPlayer->ShowViewPortPanel( PANEL_CHANGETEAM, true );
			return;
		}
	}

	// update player config
	bool bSlain = PlayerConfigUpdate( pPlayer );

	// change team
	pPlayer->ChangeTeam( iDesiredTeam );

	// tell the running mode
	if( IsModeRunning( ) )
		RunningMode( )->PlayerJoinedPlayTeam( pPlayer, bSlain );

	// ensure a good player count
	if( bJoiningPlayTeam )
		HandlePlayerCount( );
}

//=========================================================
//=========================================================
bool CINSRules::ValidJoinTeam( int iTeamID )
{
	CPlayTeam *pTeam = GetGlobalPlayTeam( iTeamID );

	if( !pTeam || pTeam->IsFull( ) )
		return false;

	if( autoteambalance.GetBool( ) )
	{
		CPlayTeam *pOtherTeam = GetGlobalPlayTeam( FlipPlayTeam( iTeamID ) );

		if( !pOtherTeam || ( pTeam->GetNumPlayers( ) - pOtherTeam->GetNumPlayers( ) ) > limitteams.GetInt( ) )
			return false;
	}

	return true;
}

//=========================================================
//=========================================================
void CINSRules::PlayerJoinedTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam )
{
	CurrentMode( )->PlayerJoinedTeam( pPlayer, pTeam );

	CINSObjective::UpdateAllRequiredPlayers( );
}

//=========================================================
//=========================================================
void CINSRules::PlayerLeftTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam )
{
	CurrentMode( )->PlayerLeftTeam( pPlayer, pTeam );

	CINSObjective::UpdateAllRequiredPlayers( );
}

//=========================================================
//=========================================================
bool CINSRules::PlayerConfigUpdate( CINSPlayer *pPlayer )
{
	if( !IsModeRunning( ) || !pPlayer->IsRunningAround( ) )
		return false;

	m_bIgnoreNextDeath = true;
	pPlayer->CommitSuicide( true );

	return true;
}

//=========================================================
//=========================================================
bool CINSRules::SetupPlayerSquad( CINSPlayer *pPlayer, bool bForce, EncodedSquadData_t *pEncodedSquadData, bool bWhenDie )
{
	// can we change?
	if( !pPlayer || ( !bForce && !CanChangeSquad( pPlayer ) ) )
		return false;

	CPlayTeam *pTeam = pPlayer->GetPlayTeam( );

	if( !pTeam )
		return false;

	// translate the slot
	SquadData_t NewSquadData;
	
	bool bRandomSquad = false;

	if( !pEncodedSquadData )
	{
		bRandomSquad = true;
	}
	else
	{
		NewSquadData.ParseEncodedInfo( *pEncodedSquadData );

		// make sure squad is valid and enabled
		if( !pTeam->IsValidSquadData( NewSquadData ) )
			bRandomSquad = true;
	}

	// select random squad if needed
	if( bRandomSquad )
	{
		if( !pTeam->GetRandomSquad( NewSquadData ) )
			return false;
	}

	// ensure its valid and he's not selected the same slot
	if( !NewSquadData.IsValid( ) || NewSquadData == pPlayer->GetSquadData( ) )
		return false;

	// ensure whendie is valid
	if( bWhenDie && !pPlayer->IsRunningAround( ) )
		bWhenDie = false;

	// change the squad
	bool bSlain = false;

	if( !bWhenDie )
		bSlain = PlayerConfigUpdate( pPlayer );

	bool bWasFirstChange = pPlayer->IsFirstSquadChange( );

	// change the squad
	if( !pPlayer->ChangeSquad( NewSquadData, bWhenDie ) )
		return false;

	// send it off to the current mode
	if( !bWhenDie )
		CurrentMode( )->PlayerJoinedSquad( pPlayer, bWasFirstChange );

	return true;
}

//=========================================================
//=========================================================
bool CINSRules::SetupPlayerSquad( CINSPlayer *pPlayer, const SquadData_t *pSquadData )
{
	CPlayTeam *pTeam = pPlayer->GetPlayTeam( );
	Assert( pTeam );

	if( !pTeam )
		return false;

	if( pSquadData )
	{
		pPlayer->ChangeSquad( *pSquadData, false );
	}
	else
	{
		SquadData_t SquadData;

		if( !pTeam->GetRandomSquad( SquadData ) )
			return false;

		pPlayer->ChangeSquad( SquadData, false );
	}

	CurrentMode( )->PlayerJoinedSquad( pPlayer, false );

	return true;
}

//=========================================================
//=========================================================
bool CINSRules::AllowOrderedSquadSelection( void ) const
{
	// don't allow ordered squad selection when random squads is enabled
	// or when we should be waiting for players
	return ( m_iSquadOrderType != SQUADORDER_NONE );
}

//=========================================================
//=========================================================
void CINSRules::UpdateSquadOrderType(void)
{
	m_iSquadOrderType = squadorder.GetInt( );
}
