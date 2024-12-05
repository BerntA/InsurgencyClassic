//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "team_shared.h"
#include "ins_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
void SendProxy_PlayerList( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CTeam *pTeam = ( CTeam* )pData;

	const CUtlVector< CINSPlayer* > &PlayerList = pTeam->GetPlayerList( );

	// if this assertion fails, then SendProxyArrayLength_PlayerArray failed
	Assert( iElement < PlayerList.Size( ) );

	CINSPlayer *pPlayer = PlayerList[ iElement ];
	pOut->m_Int = pPlayer->entindex( );
}

//=========================================================
//=========================================================
int SendProxyArrayLength_PlayerArray( const void *pStruct, int objectID )
{
	CTeam *pTeam = ( CTeam* )pStruct;

	return pTeam->GetPlayerList( ).Count( );
}

//=========================================================
//=========================================================
IMPLEMENT_SERVERCLASS_ST_NOBASE( CTeam, DT_Team )

	SendPropInt( SENDINFO( m_iTeamID ), 8, SPROP_UNSIGNED ),

	SendPropArray2( 
		SendProxyArrayLength_PlayerArray,
		SendPropInt( "player_array_element", 0, 4, 10, SPROP_UNSIGNED, SendProxy_PlayerList ), 
		MAX_PLAYERS, 
		0, 
		"player_array"
		),

END_SEND_TABLE()

//=========================================================
//=========================================================
CTeam *g_pTeams[ MAX_TEAMS ];

//=========================================================
//=========================================================
CTeam::CTeam( )
{
}

//=========================================================
//=========================================================
CTeam::~CTeam( )
{
	m_Players.Purge( );

	g_pTeams[ m_iTeamID ] = NULL;
}

//=========================================================
//=========================================================
int CTeam::UpdateTransmitState( void )
{
	return SetTransmitState(FL_EDICT_ALWAYS);
}

//=========================================================
//=========================================================
void CTeam::Init( int iTeamID )
{
	Assert( IsValidTeam( iTeamID ) );

	m_iTeamID = iTeamID;

	g_pTeams[ m_iTeamID ] = this;
}

//=========================================================
//=========================================================
void CTeam::AddPlayer( CINSPlayer *pPlayer )
{
	m_Players.AddToTail( pPlayer );

	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CTeam::RemovePlayer( CINSPlayer *pPlayer )
{
	m_Players.FindAndRemove( pPlayer );

	NetworkStateChanged( );
}

//=========================================================
//=========================================================
/*CINSPlayer *CTeam::FindPlayer( CINSPlayer *pPlayer )
{
	int iID = m_Players.Find( pPlayer );

	if( !m_Players.IsValidIndex( iID ) )
		return NULL;		

	return m_Players[ iID ];
}*/

//=========================================================
//=========================================================
CINSPlayer *CTeam::GetPlayer( int iID ) const
{
	Assert( iID >= 0 && iID < m_Players.Size( ) );
	return m_Players[ iID ];
}

//=========================================================
//=========================================================
CTeam *GetGlobalTeam( int iTeamID )
{
	Assert( iTeamID >= 0 && iTeamID < MAX_TEAMS );
	return g_pTeams[ iTeamID ];
}