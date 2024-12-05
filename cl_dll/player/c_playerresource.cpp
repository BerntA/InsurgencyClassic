//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "c_playerresource.h"
#include "play_team_shared.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
IMPLEMENT_CLIENTCLASS_DT_NOBASE(C_PlayerResource, DT_PlayerResource, CPlayerResource)

	RecvPropArray3( RECVINFO_ARRAY( m_bConnected ), RecvPropBool( RECVINFO( m_bConnected[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iPing ), RecvPropInt( RECVINFO( m_iPing[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iDeveloper ), RecvPropInt( RECVINFO( m_iDeveloper[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iTeamID ), RecvPropInt( RECVINFO( m_iTeamID[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_bAlive ), RecvPropBool( RECVINFO( m_bAlive[ 0 ] ) ) ),

	RecvPropArray3( RECVINFO_ARRAY( m_iMorale ), RecvPropInt( RECVINFO( m_iMorale[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iKills ), RecvPropInt( RECVINFO( m_iKills[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iDeaths ), RecvPropInt( RECVINFO( m_iDeaths[ 0 ] ) ) ),

END_RECV_TABLE( )

//=========================================================
//=========================================================
C_PlayerResource *g_PR;

bool ValidPlayerResource( void )
{
	return ( g_PR != NULL );
}

C_PlayerResource *PlayerResource( void )
{
	Assert( ValidPlayerResource( ) );
	return g_PR;
}

IGameResources *GameResources( void )
{
	return PlayerResource( );
}

//=========================================================
//=========================================================
C_PlayerResource::C_PlayerResource( )
{
	memset( m_szName, 0, sizeof( m_szName ) );

	memset( m_bConnected, 0, sizeof( m_bConnected ) );
	memset( m_iPing, 0, sizeof( m_iPing ) );
	memset( m_iDeveloper, 0, sizeof( m_iDeveloper ) );
	memset( m_iTeamID, 0, sizeof( m_iTeamID ) );
	memset( m_bAlive, 0, sizeof( m_bAlive ) );

	memset( m_iMorale, 0, sizeof( m_iMorale ) );
	memset( m_iKills, 0, sizeof( m_iKills ) );
	memset( m_iDeaths, 0, sizeof( m_iDeaths ) );
	
	g_PR = this;
}

//=========================================================
//=========================================================
C_PlayerResource::~C_PlayerResource( )
{
	g_PR = NULL;
}

//=========================================================
//=========================================================
const char *C_PlayerResource::GetTeamName( int iID )
{
	C_Team *pTeam = GetGlobalTeam( iID );
	return ( pTeam ? pTeam->GetName( ) : "unknown" );
}

//=========================================================
//=========================================================
int	C_PlayerResource::GetTeamScore( int iID )
{
	if( !IsPlayTeam( iID ) )
		return 0;

	C_PlayTeam *pTeam = GetGlobalPlayTeam( iID );
	return ( pTeam ? pTeam->GetTotalPlayerScore( ) : 0 );
}

//=========================================================
//=========================================================
const Color &C_PlayerResource::GetTeamColor( int iID )
{
	return INSRules( )->TeamColor( iID );
}

//=========================================================
//=========================================================
const char *C_PlayerResource::GetPlayerName( int iID )
{
	if( !IsValidPlayerID( iID ) )
		return "ERRORNAME";

	if( !IsConnected( iID ) )
		return "unconnected";

	// make sure it's up to date
	player_info_t PlayerInfo;

	if( !engine->GetPlayerInfo( iID, &PlayerInfo ) )
		return "unconnected";

	Q_strncpy( m_szName[ iID ], PlayerInfo.name, MAX_PLAYER_NAME_LENGTH );
	
	return m_szName[ iID ];
}

//=========================================================
//=========================================================
int C_PlayerResource::GetFrags( int iID )
{
	if( !IsConnected( iID ) )
		return 0;

	return ( IsValidPlayerID( iID ) ? m_iKills[ iID ] : 0 );
}

//=========================================================
//=========================================================
int C_PlayerResource::GetDeaths( int iID )
{
	if( !IsConnected( iID ) )
		return 0;

	return ( IsValidPlayerID( iID ) ? m_iDeaths[ iID ] : 0 );
}

//=========================================================
//=========================================================
int C_PlayerResource::GetTeam( int iID )
{
	return ( IsValidPlayerID( iID ) ? m_iTeamID[ iID ] : 0 );
}

//=========================================================
//=========================================================
bool C_PlayerResource::IsAlive( int iID )
{
	return ( IsValidPlayerID( iID ) ? m_bAlive[ iID ] : false );
}

//=========================================================
//=========================================================
bool C_PlayerResource::IsLocalPlayer( int iID )
{
	C_BasePlayer *pPlayer =	C_BasePlayer::GetLocalPlayer();
	return ( ( pPlayer && pPlayer->entindex( ) == iID ) ? true : false );
}

//=========================================================
//=========================================================
bool C_PlayerResource::IsHLTV( int iID )
{
	if( !IsConnected( iID ) )
		return false;

	player_info_t PlayerInfo;
	return ( engine->GetPlayerInfo( index, &PlayerInfo ) ? PlayerInfo.ishltv : false );
}

//=========================================================
//=========================================================
bool C_PlayerResource::IsFakePlayer( int iID )
{
	if( !IsConnected( iID ) )
		return false;

	// make sure it's up to date
	player_info_t PlayerInfo;
	return ( engine->GetPlayerInfo( iID, &PlayerInfo ) ? PlayerInfo.fakeplayer : false );
}

//=========================================================
//=========================================================
int	C_PlayerResource::GetPing( int iID )
{
	if( !IsConnected( iID ) )
		return 0;

	return m_iPing[ iID ];
}

//=========================================================
//=========================================================
bool C_PlayerResource::IsConnected( int iID )
{
	return ( IsValidPlayerID( iID ) ? m_bConnected[ iID ] : false );
}

//=========================================================
//=========================================================
int C_PlayerResource::GetDeveloper( int iID )
{
	if( !IsConnected( iID ) )
		return DEVMODE_OFF;

	return ( IsValidPlayerID( iID ) ? m_iDeveloper[ iID ] : DEVMODE_OFF );
}

//=========================================================
//=========================================================
int	C_PlayerResource::GetMorale( int iID )
{
	if( !IsConnected( iID ) )
		return 0;

	return m_iMorale[ iID ];
}

//=========================================================
//=========================================================
bool C_PlayerResource::GetSquadData( int iID, SquadData_t &SquadData )
{
	int iTeamID = GetTeamID( iID );

	if( !IsPlayTeam( iTeamID ) )
		return false;

	C_PlayTeam *pTeam = GetGlobalPlayTeam( iTeamID );
	
	if( !pTeam )
		return false;

	return pTeam->GetSquadData( iID, SquadData );
}

//=========================================================
//=========================================================
int C_PlayerResource::GetSquadID( int iID )
{
	SquadData_t SquadData;

	if( !GetSquadData( iID, SquadData ) )
		return INVALID_SQUAD;

	return SquadData.GetSquadID( );
}

//=========================================================
//=========================================================
int C_PlayerResource::GetSlotID( int iID )
{
	SquadData_t SquadData;

	if( !GetSquadData( iID, SquadData ) )
		return INVALID_SQUAD;

	return SquadData.GetSlotID( );
}

//=========================================================
//=========================================================
bool C_PlayerResource::IsValidPlayerID( int iID )
{
	if( iID < 0 || iID > MAX_PLAYERS )
	{
		Assert( false );
		return false;
	}

	return true;
}