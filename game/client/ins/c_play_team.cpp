//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#include "cbase.h"
#include "imc_format.h"
#include "play_team_shared.h"
#include "team_lookup.h"
#include "ins_squad_shared.h"
#include "igameresources.h"
#include "c_playerresource.h"
#include "imc_config.h"
#include "inshud.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#if defined(CPlayTeam)
#undef CPlayTeam	
#endif

//=========================================================
//=========================================================
void RecvProxy_TeamLookup( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_PlayTeam *pTeam = ( C_PlayTeam* )pStruct;
	pTeam->SetTeamLookup( pData->m_Value.m_Int );
}

void RecvProxy_TeamColor( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_PlayTeam *pTeam = ( C_PlayTeam* )pStruct;
	pTeam->SetColor( pData->m_Value.m_Int );
}

void RecvProxyArrayLength_SquadArray( void *pStruct, int objectID, int currentArrayLength )
{
	C_PlayTeam *pTeam = ( C_PlayTeam* )pStruct;
	
	if( pTeam->m_Squads.Count() != currentArrayLength )
		pTeam->m_Squads.SetCount( currentArrayLength );
}

void RecvProxy_SquadList( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_PlayTeam *pTeam = ( C_PlayTeam* )pStruct;
	pTeam->m_Squads[ pData->m_iElement ] = pData->m_Value.m_Int;
}

void RecvProxy_NumWaves( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_PlayTeam *pTeam = ( C_PlayTeam* )pStruct;
	pTeam->SetNumWaves( pData->m_Value.m_Int - 1 );
}

void RecvProxy_TimeWave( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_PlayTeam *pTeam = ( C_PlayTeam* )pStruct;
	pTeam->SetTimeWave( pData->m_Value.m_Int );
}

//=========================================================
//=========================================================
/*BEGIN_NETWORK_TABLE_NOBASE(C_PlayTeam, DT_MedicResource)
	

END_RECV_TABLE()*/

BEGIN_NETWORK_TABLE_NOBASE( C_PlayTeam, DT_TeamResource )

	RecvPropArray2( 
		RecvProxyArrayLength_SquadArray,
		RecvPropInt( "squad_element", 0, SIZEOF_IGNORE, 0, RecvProxy_SquadList ),
		MAX_SQUADS, 
		0, 
		"squad_array"
		),

	RecvPropArray3( RECVINFO_ARRAY( m_iSquadData ), RecvPropInt( RECVINFO( m_iSquadData[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iHealthType ), RecvPropInt( RECVINFO( m_iHealthType[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iRank ), RecvPropIntWithMinusOneFlag( RECVINFO( m_iRank[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iOrder ), RecvPropIntWithMinusOneFlag( RECVINFO( m_iOrder[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iStatusType ), RecvPropIntWithMinusOneFlag( RECVINFO( m_iStatusType[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iStatusID ), RecvPropIntWithMinusOneFlag( RECVINFO( m_iStatusID[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_bNeedsHelp ), RecvPropBool( RECVINFO( m_bNeedsHelp[ 0 ] ) ) ),

	RecvPropInt( "numwaves", 0, SIZEOF_IGNORE, 0, RecvProxy_NumWaves ),
	RecvPropInt( "timewave", 0, SIZEOF_IGNORE, 0, RecvProxy_TimeWave ),

	RecvPropArray3( RECVINFO_ARRAY( m_iReinforcementsLeft ), RecvPropInt( RECVINFO( m_iReinforcementsLeft[ 0 ] ) ) ),
	RecvPropFloat( RECVINFO( m_flStartReinforcementTime ) ),
	RecvPropBool( RECVINFO( m_bEmergencyReinforcement ) ),

	RecvPropArray3( RECVINFO_ARRAY( m_iEmergencyReinforcementsLeft ), RecvPropInt( RECVINFO( m_iEmergencyReinforcementsLeft[ 0 ] ) ) ),

END_RECV_TABLE()

IMPLEMENT_CLIENTCLASS_DT( C_PlayTeam, DT_PlayTeam, CPlayTeam )

	RecvPropInt( "team_lookupid", 0, SIZEOF_IGNORE, 0, RecvProxy_TeamLookup ),

	RecvPropInt( "color", 0, SIZEOF_IGNORE, SPROP_UNSIGNED, RecvProxy_TeamColor ),

	RecvPropInt( RECVINFO( m_iScore ) ),

	RecvPropDataTable( "TeamResource", 0, 0, &REFERENCE_RECV_TABLE( DT_TeamResource ) ),
	//RecvPropDataTable( "MedicResource", 0, 0, &REFERENCE_RECV_TABLE( DT_MedicResource ) ),

END_RECV_TABLE()

//=========================================================
//=========================================================
C_PlayTeam::C_PlayTeam( )
{
	m_pIMCTeamConfig = NULL;

	m_iPing = 0;

	memset( m_iSquadData, 0, sizeof( m_iSquadData ) );
	memset( m_iHealthType, 0, sizeof( m_iHealthType ) );
	memset( m_iRank, 0, sizeof( m_iRank ) );
	memset( m_bNeedsHelp, false, sizeof( m_bNeedsHelp ) );
}

//=========================================================
//=========================================================
void C_PlayTeam::Setup( void )
{
	m_pIMCTeamConfig = IMCConfig( )->GetIMCTeam( m_iTeamID );
	Assert( m_pIMCTeamConfig );
}

//=========================================================
//=========================================================
void C_PlayTeam::OnDataChanged( DataUpdateType_t Type )
{
	BaseClass::OnDataChanged( Type );

	if( Type == DATA_UPDATE_CREATED )
	{
		Setup( );
	}
	else if( Type == DATA_UPDATE_DATATABLE_CHANGED )
	{
		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

		if( pPlayer && pPlayer->OnPlayTeam( ) && pPlayer->OnTeam( this ) )
			GetINSHUDHelper( )->SendTeamUpdate( this );
	}
}

//=========================================================
//=========================================================
void C_PlayTeam::SetTeamLookup( int iTeamLookup )
{
	m_iTeamLookup = iTeamLookup;
	m_pTeamLookup = g_pTeamLookup[ iTeamLookup ];
	Assert( m_pTeamLookup );
}

//=========================================================
//=========================================================
CTeamLookup *C_PlayTeam::GetTeamLookup( void ) const
{
	Assert( m_pTeamLookup );
	return m_pTeamLookup;
}

//=========================================================
//=========================================================
const char *C_PlayTeam::GetName( void ) const
{
	return GetTeamLookup( )->GetName( );
}

//=========================================================
//=========================================================
const char *C_PlayTeam::GetNameFromID( int iTeamID )
{
	return ( ( iTeamID == TEAM_ONE ) ? "one" : "two" );
}

//=========================================================
//=========================================================
C_INSSquad *C_PlayTeam::GetSquad( int iID ) const
{
	if( !m_Squads.IsValidIndex( iID ) )
		return NULL;

	C_INSSquad *pSquad = ( C_INSSquad* )cl_entitylist->GetEnt( m_Squads[ iID ] );
	Assert( pSquad );

	return pSquad;
}

//=========================================================
//=========================================================
int C_PlayTeam::CountTotalSlots( void ) const
{
	return ( GetSquadCount() * MAX_SQUAD_SLOTS );
}

//=========================================================
//=========================================================
int C_PlayTeam::CountTotalFreeSlots( void ) const
{
	int iTotalCount = 0;

	for( int i = 0; i < GetSquadCount(); i++ )
		iTotalCount += GetSquad( i )->CountFreeSlots( );
	
	return iTotalCount;
}

//=========================================================
//=========================================================
void C_PlayTeam::UpdatePing( void )
{
	m_iPing = 0;

	int iNumberPlayers = GetNumPlayers( );
	IGameResources *gr = GameResources( );

	if( !gr )
		return;

	for( int i = 0; i < iNumberPlayers; i++ )
	{
		int iPlayerIndex = GetPlayerID( i );

		if( gr->IsConnected( iPlayerIndex) )
			m_iPing += gr->GetPing( iPlayerIndex );
	}

	if( m_iPing != 0 )
		m_iPing /= iNumberPlayers;
}

//=========================================================
//=========================================================
bool C_PlayTeam::GetSquadData( int iIndex, SquadData_t &SquadData )
{
	if( iIndex < 0 || iIndex > MAX_PLAYERS )
	{
		Assert( false );
		return false;
	}
	else
	{
		SquadData.ParseEncodedInfo( m_iSquadData[ iIndex ] );
		return true;
	}
}

//=========================================================
//=========================================================
int C_PlayTeam::GetHealthType( int iIndex )
{
	if( iIndex < 0 || iIndex > MAX_PLAYERS )
	{
		Assert( false );
		return 0;
	}
	else
	{
		return m_iHealthType[ iIndex ];
	}
}

//=========================================================
//=========================================================
int C_PlayTeam::GetRank( int iIndex )
{
	if( iIndex < 0 || iIndex > MAX_PLAYERS )
	{
		Assert( false );
		return 0;
	}
	else
	{
		return m_iRank[ iIndex ];
	}
}

//=========================================================
//=========================================================
int C_PlayTeam::GetOrder( int iIndex )
{
	if( iIndex < 0 || iIndex > MAX_PLAYERS )
	{
		Assert( false );
		return 0;
	}
	else
	{
		return m_iOrder[ iIndex ];
	}
}

//=========================================================
//=========================================================
int C_PlayTeam::GetStatusType( int iIndex )
{
	if( iIndex < 0 || iIndex > MAX_PLAYERS )
	{
		Assert( false );
		return 0;
	}
	else
	{
		return m_iStatusType[ iIndex ];
	}
}

//=========================================================
//=========================================================
int C_PlayTeam::GetStatusID( int iIndex )
{
	if( iIndex < 0 || iIndex > MAX_PLAYERS )
	{
		Assert( false );
		return 0;
	}
	else
	{
		return m_iStatusID[ iIndex ];
	}
}

//=========================================================
//=========================================================
int C_PlayTeam::NeedsHelp( int iIndex )
{
	if( iIndex < 0 || iIndex > MAX_PLAYERS )
	{
		Assert( false );
		return false;
	}
	else 
	{
		return m_bNeedsHelp[ iIndex ];
	}
}

//=========================================================
//=========================================================
int C_PlayTeam::GetType( void )
{
	return GetTeamLookup( )->GetType( );
}

//=========================================================
//=========================================================
C_PlayTeam *GetGlobalPlayTeam( int iTeamID )
{
	Assert( IsPlayTeam( iTeamID ) );
	return ( C_PlayTeam* )GetGlobalTeam( iTeamID );
}

//=========================================================
//=========================================================
C_PlayTeam *GetPlayersPlayTeam( int iPlayerIndex )
{
	int iTeamID = g_PR->GetTeamID( iPlayerIndex );

	if( !IsPlayTeam( iTeamID ) )
		return NULL;

	return GetGlobalPlayTeam( iTeamID );
}

//=========================================================
//=========================================================
C_PlayTeam *GetPlayersPlayTeam(C_INSPlayer *pPlayer)
{
	return GetPlayersPlayTeam(pPlayer->entindex());
}

//=========================================================
//=========================================================
void C_PlayTeam::SetColor( int iColor )
{
	m_Color.SetRawColor( iColor );
}

//=========================================================
//=========================================================
int C_PlayTeam::GetReinforcementsLeft( int iSquadID )
{
	return m_iReinforcementsLeft[ iSquadID ];
}

//=========================================================
//=========================================================
int C_PlayTeam::GetEmergencyReinforcementsLeft( int iSquadID )
{
	return m_iEmergencyReinforcementsLeft[ iSquadID ];
}