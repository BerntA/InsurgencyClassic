//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTeam class
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "c_team.h"
#include "ins_player_shared.h"
#include "c_playerresource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
void RecvProxy_PlayerList( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_Team *pTeam = ( C_Team* )pOut;
	pTeam->m_Players[ pData->m_iElement ] = pData->m_Value.m_Int;
}

//=========================================================
//=========================================================
void RecvProxyArrayLength_PlayerArray( void *pStruct, int objectID, int currentArrayLength )
{
	C_Team *pTeam = ( C_Team* )pStruct;
	
	if ( pTeam->m_Players.Size( ) != currentArrayLength )
		pTeam->m_Players.SetSize( currentArrayLength );
}

//=========================================================
//=========================================================
IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_Team, DT_Team, CTeam )

	RecvPropInt( RECVINFO( m_iTeamID ) ),
	RecvPropArray2( 
		RecvProxyArrayLength_PlayerArray,
		RecvPropInt( "player_array_element", 0, SIZEOF_IGNORE, 0, RecvProxy_PlayerList ), 
		MAX_PLAYERS, 
		0, 
		"player_array"
		)

END_RECV_TABLE( )

//=========================================================
//=========================================================
C_Team *g_pTeams[ MAX_TEAMS ];
static int g_iInitCount = 0;

//=========================================================
//=========================================================
C_Team::C_Team( )
{
	g_iInitCount = 0;
}

//=========================================================
//=========================================================
C_Team::~C_Team( )
{
	g_pTeams[ m_iTeamID ] = NULL;
}

//=========================================================
//=========================================================
void C_Team::OnDataChanged( DataUpdateType_t Type )
{
	BaseClass::OnDataChanged( Type );

	if( Type == DATA_UPDATE_CREATED )
	{
		g_pTeams[ m_iTeamID ] = this;

		g_iInitCount++;
	}
}

//=========================================================
//=========================================================
bool C_Team::ValidTeams( void )
{
	return ( g_iInitCount == MAX_TEAMS );
}

//=========================================================
//=========================================================
const char *C_Team::GetName( void ) const
{
	Assert( false );
	return NULL;
}

//=========================================================
//=========================================================
C_INSPlayer *C_Team::GetPlayer( int iID ) const
{
	Assert( m_Players.IsValidIndex( iID ) );
	return ToINSPlayer( cl_entitylist->GetEnt( m_Players[ iID ] ) );
}

//=========================================================
//=========================================================
int C_Team::GetPlayerID( int iID ) const
{
	Assert( m_Players.IsValidIndex( iID ) );
	return m_Players[ iID ];
}

//=========================================================
//=========================================================
/*bool C_Team::ContainsPlayer(int iPlayerIndex)
{
	for (int i = 0; i < m_Players.Size(); i++)
	{
		if (m_Players[i] == iPlayerIndex)
			return true;
	}

	return false;
}*/

//=========================================================
//=========================================================
C_Team *GetGlobalTeam( int iTeamID )
{
	Assert( iTeamID >= 0 && iTeamID < MAX_TEAMS );
	return g_pTeams[ iTeamID ];
}

//=========================================================
//=========================================================
C_Team *GetPlayersTeam( int iPlayerID )
{
	return GetGlobalTeam(g_PR->GetTeamID( iPlayerID ) );
}

//=========================================================
//=========================================================
C_Team *GetPlayersTeam( C_INSPlayer *pPlayer )
{
	Assert( pPlayer );
	return GetPlayersTeam( pPlayer->entindex( ) );
}