//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_gamerules.h"
#include "imc_config.h"
#include "ins_player_shared.h"
#include "play_team_shared.h"
#include "ins_squad_shared.h"
#include "viewport_panel_names.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
ConVar gamecount( "ins_gamecount", "3", FCVAR_NOTIFY, "", true, 0, true, 30 );
ConVar teamswap( "ins_teamswap", "0", FCVAR_NOTIFY, "When Restarting and Enabled, Teams will Swap", true, 0, true, 1 );

//=========================================================
//=========================================================
typedef CINSPlayer *pINSPlayer_t;

int SortPlayerRoutine( const pINSPlayer_t *pLeft, const pINSPlayer_t *pRight )
{
	CINSPlayer *pLeftPlayer = *pLeft;
	CINSPlayer *pRightPlayer = *pRight;

	if( pLeftPlayer->GetMorale( ) != pRightPlayer->GetMorale( ) )
	{
		if( pLeftPlayer->GetMorale( ) < pRightPlayer->GetMorale( ) )
			return 1;
		else /*if( pLeftPlayer->GetMorale( ) > pRightPlayer->GetMorale( ) )*/
			return -1;
	}

	if( pLeftPlayer->GetTimeJoined( ) > pRightPlayer->GetTimeJoined( ) )
		return 1;
	else if( pLeftPlayer->GetTimeJoined( ) < pRightPlayer->GetTimeJoined( ) )
		return -1;

	return 0;
}

//=========================================================
//=========================================================
void CSquadPlayerOrder::Reset( void )
{
	m_Players.Purge( );
}

//=========================================================
//=========================================================
void CSquadPlayerOrder::Add( CINSPlayer *pPlayer )
{
	m_Players.AddToTail( pPlayer );
}

//=========================================================
//=========================================================
void CSquadPlayerOrder::Remove( CINSPlayer *pPlayer )
{
	m_Players.FindAndRemove( pPlayer );
}

//=========================================================
//=========================================================
int CSquadPlayerOrder::Count( void ) const
{
	return m_Players.Count( );
}

//=========================================================
//=========================================================
CINSPlayer *CSquadPlayerOrder::operator[ ]( int iID ) const
{
	return m_Players[ iID ];
}

//=========================================================
//=========================================================
CINSPlayer *CSquadPlayerOrder::Element( int iID ) const
{
	return m_Players[ iID ];
}

//=========================================================
//=========================================================
void CSquadPlayerOrder::Sort( void )
{
	m_Players.Sort( SortPlayerRoutine );
}

//=========================================================
//=========================================================
CEmptyClassPositionList::CEmptyClassPositionList( )
{
}

//=========================================================
//=========================================================
bool CEmptyClassPositionList::Extract( int iPlayerClass, SquadData_t &SquadData )
{
	// try and find the class
	int iClassIndex = m_Classes.Find( iPlayerClass );

	if( iClassIndex == m_Classes.InvalidIndex( ) )
		return false;

	// ensure it's not empty
	CPositionList::ClassPositions_t &ClassPositions = m_Classes[ iClassIndex ].Get( );

	// ensure there are positions available
	if( ClassPositions.Count( ) == 0 )
		return false;

	// copy position and remove
	SquadData = ClassPositions[ 0 ];
	ClassPositions.Remove( 0 );

	return true;
}

//=========================================================
//=========================================================
void CEmptyClassPositionList::Init( const CClassPositionList &ClassPositionList )
{
	CClassPositionList::Init( ClassPositionList );
}

//=========================================================
//=========================================================
CSquadMode::CSquadMode( )
{
}

//=========================================================
//=========================================================
int CSquadMode::Init( void )
{
	// update squad selection type
	INSRules( )->UpdateSquadOrderType( );

	// cleanup entities
	INSRules( )->CleanUpEntities( );

	// mass squad remove
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		CPlayTeam *pTeam = GetGlobalPlayTeam( i );

		if( pTeam )
			pTeam->MassSquadRemove( );
	}

	// swap teams when told
	if( teamswap.GetBool( ) )
	{
		INSRules( )->SwapPlayTeams( );

		teamswap.SetValue( false );
	}

	// if we're waiting for players, just respawn
	if( INSRules( )->ShouldBeWaitingForPlayers( ) )
	{
		for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		{
			CPlayTeam *pTeam = GetGlobalPlayTeam( i );

			// setup squads and slots
			for( int j = 0; j < pTeam->GetNumPlayers( ); j++ )
				INSRules( )->SetupPlayerSquad( pTeam->GetPlayer( j ), NULL );
		}

		return GRMODE_RUNNING;
	}

	// set when to stop
	m_flStartThreshold = gpGlobals->curtime + 1.0f;

	// remove everyone and put into 
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		Setup( i );

	return GRMODE_NONE;
}

//=========================================================
//=========================================================
void CSquadMode::Setup( int iTeamID )
{
	CSquadPlayerOrder &PlayerOrder = GetPlayerOrder( iTeamID );

	// reset order
	PlayerOrder.Reset( );

	// get team
	CPlayTeam *pTeam = GetGlobalPlayTeam( iTeamID );

	if( !pTeam )
		return;

	// create list of players and respawn
	for( int i = 0; i < pTeam->GetNumPlayers( ); i++ )
	{
		CINSPlayer *pPlayer = pTeam->GetPlayer( i );

		if( !pPlayer )
			continue;

		// add them to the player order
		PlayerOrder.Add( pPlayer );

		// strip items
		pPlayer->RemoveAllItems( );

		// respawn!
		pPlayer->Spawn( );
	}

	// sort the player list
	PlayerOrder.Sort( );

	// if debugging, print everything out
#ifdef _DEBUG

	Msg( "--------------------------\n" );
	Msg( "Squad Order for %s\n", pTeam->GetName( ) );
	Msg( "--------------------------\n" );

	for( int i = 0; i < PlayerOrder.Count( ); i++ )
	{
		CINSPlayer *pPlayer = PlayerOrder[ i ];
		Msg( "%i Player: %s\n", i, pPlayer ? pPlayer->GetPlayerName( ) : "unknown!?" );
	}

	Msg( "\n" );

#endif
}

//=========================================================
//=========================================================
const CSquadPlayerOrder &CSquadMode::GetPlayerOrder( int iTeamID ) const
{
	return m_PlayerOrder[ TeamToPlayTeam( iTeamID ) ];
}

//=========================================================
//=========================================================
CSquadPlayerOrder &CSquadMode::GetPlayerOrder( int iTeamID )
{
	return m_PlayerOrder[ TeamToPlayTeam( iTeamID ) ];
}

//=========================================================
//=========================================================
int CSquadMode::Think( void )
{
	if( gpGlobals->curtime >= m_flStartThreshold )
	{
		for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		{
			CPlayTeam *pTeam = GetGlobalPlayTeam( i );

			if( !pTeam )
				continue;

			// copy the class position list
			CEmptyClassPositionList ClassPositionList;
			ClassPositionList.Init( pTeam->GetClassPositionList( ) );

			// run through the order and try and spawn them
			CSquadPlayerOrder &PlayerOrder = GetPlayerOrder( i );

			for( int j = 0; j < PlayerOrder.Count( ); j++ )
			{
				CINSPlayer *pPlayer = PlayerOrder[ j ];

				if( !pPlayer )
					continue;

				if( INSRules( )->AllowOrderedSquadSelection( ) )
				{
					for( int k = 0; k < PLAYER_CLASSPREFERENCE_COUNT; k++ )
					{
						int iPlayerClass;

						if( j == 0 && k == 0 )
							iPlayerClass = pTeam->GetTeamLookup( )->GetCommanderClass( );
						else
							iPlayerClass = pPlayer->GetClassPreference( k );							

						if( iPlayerClass == INVALID_CLASS )
							break;

						SquadData_t SquadData;

						if( ClassPositionList.Extract( iPlayerClass, SquadData ) )
							INSRules( )->SetupPlayerSquad( pPlayer, &SquadData );
					}
				}

				if( !pPlayer->InSquad( ) )
					INSRules( )->SetupPlayerSquad( pPlayer, NULL );
			}
		}

		return GRMODE_RUNNING;
	}

	return GRMODE_NONE;
}

//=========================================================
//=========================================================
void CSquadMode::PlayerJoinedTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam )
{
	CSquadPlayerOrder &PlayerOrder = GetPlayerOrder( pTeam->GetTeamID( ) );
	PlayerOrder.Add( pPlayer );
}

//=========================================================
//=========================================================
void CSquadMode::PlayerLeftTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam )
{
	CSquadPlayerOrder &PlayerOrder = GetPlayerOrder( pTeam->GetTeamID( ) );
	PlayerOrder.Remove( pPlayer );
}

//=========================================================
//=========================================================
int CSquadMode::GetOrderLength( int iTeamID )
{
	Assert( IsPlayTeam( iTeamID ) );
	return GetPlayerOrder( iTeamID ).Count( );
}

//=========================================================
//=========================================================
int CSquadMode::GetOrderElement( int iTeamID, int iElement )
{
	Assert( IsPlayTeam( iTeamID ) );

	CINSPlayer *pPlayer = GetPlayerOrder( iTeamID ).Element( iElement );

	if( !pPlayer )
		return NULL;

	return pPlayer->entindex( );
}