//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "ins_gamerules.h"
#include "ins_obj_shared.h"
#include "c_team.h"
#include "hlscolor.h"
#include "collisionutils.h"
#include "ins_utils.h"
#include "inshud.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#if defined( CINSObjective )
#undef CINSObjective	
#endif

//=========================================================
//=========================================================
void RecvProxy_ObjName( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_INSObjective *pObjective = ( C_INSObjective* )pOut;
	pObjective->SetName( pData->m_Value.m_pString );
}

void RecvProxy_ObjPhonetic( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_INSObjective *pObjective = ( C_INSObjective* )pOut;
	pObjective->SetPhonetic( pData->m_Value.m_Int );
}

void RecvProxy_ObjColor( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_INSObjective *pObjective = ( C_INSObjective* )pOut;

	Color clrColor;
	clrColor.SetRawColor( pData->m_Value.m_Int );

	pObjective->SetColor( clrColor );
}

void RecvProxy_CaptureProgress( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_INSObjective *pObjective = ( C_INSObjective* )pOut;
	pObjective->SetCaptureProgress( pData->m_Value.m_Int - 1 );
}

void RecvProxy_RequiredPlayers( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_INSObjective *pObjective = ( C_INSObjective* )pOut;
	pObjective->SetPlayersRequired( pData->m_Value.m_Int );
}

void RecvProxy_PlayersCapturing( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_INSObjective *pObjective = ( C_INSObjective* )pOut;
	pObjective->SetPlayersCapturing( pData->m_Value.m_Int );
}

//=========================================================
//=========================================================
BEGIN_NETWORK_TABLE_NOBASE( C_INSObjective, DT_CapturingObj )

	RecvPropInt( "capture_progress", 0, SIZEOF_IGNORE, SPROP_UNSIGNED, RecvProxy_CaptureProgress ),
	
	RecvPropInt( "players_required", 0, 1, SPROP_UNSIGNED, RecvProxy_RequiredPlayers ),
	RecvPropInt( "players_capturing", 0, 1, SPROP_UNSIGNED, RecvProxy_PlayersCapturing ),

END_RECV_TABLE( )

IMPLEMENT_CLIENTCLASS_DT( C_INSObjective, DT_Objective, CINSObjective )

	RecvPropVector( RECVINFO( m_vecOrigin ) ),
	RecvPropInt( RECVINFO( m_iOrderID ), SPROP_UNSIGNED ),
	RecvPropInt( RECVINFO( m_iChaseRadius ), SPROP_UNSIGNED ),
	RecvPropString( "name", 0, MAX_OBJNAME_LENGTH, 0, RecvProxy_ObjName ),
	RecvPropInt( "phonetic", 0, 1, SPROP_UNSIGNED, RecvProxy_ObjPhonetic ),
	RecvPropInt( "color", 0, 1, SPROP_UNSIGNED, RecvProxy_ObjColor ),

	RecvPropInt( RECVINFO( m_iCapturedTeam ), SPROP_UNSIGNED ),
	RecvPropInt( RECVINFO( m_iCaptureTeam ), SPROP_UNSIGNED ),

	RecvPropBool( RECVINFO( m_bOrdersAllowed ) ),

	RecvPropDataTable( "CaptureObj", 0, 0, &REFERENCE_RECV_TABLE( DT_CapturingObj ) )

END_RECV_TABLE( )

//=========================================================
//=========================================================
C_INSObjective *g_pObjectives[ MAX_OBJECTIVES ];
CUtlVector< C_INSObjective* > g_ObjectiveList;

//=========================================================
//=========================================================
C_INSObjective::C_INSObjective( )
{
	g_ObjectiveList.AddToTail( this );

	m_szName[ 0 ] = '\0';
	m_iRadius = 0;
}

//=========================================================
//=========================================================
C_INSObjective::~C_INSObjective()
{
	g_pObjectives[ GetOrderID( ) ] = NULL;

	g_ObjectiveList.FindAndRemove( this );
}

//=========================================================
//=========================================================
void C_INSObjective::LevelInit( void )
{
	memset( g_pObjectives, 0, sizeof( g_pObjectives ) );
}

//=========================================================
//=========================================================
C_INSObjective *C_INSObjective::GetObjective( int iID )
{
	if( iID < 0 || iID >= MAX_OBJECTIVES )
		return NULL;

	return g_pObjectives[ iID ];
}

//=========================================================
//=========================================================
const CUtlVector< C_INSObjective* > &C_INSObjective::GetObjectiveList( void )
{
	return g_ObjectiveList;
}

//=========================================================
//=========================================================
void C_INSObjective::OnDataChanged( DataUpdateType_t Type )
{
	BaseClass::OnDataChanged( Type );

	if( Type == DATA_UPDATE_CREATED )
	{
		// setup objective
		Assert( m_iOrderID != INVALID_OBJECTIVE );
		g_pObjectives[ m_iOrderID ] = this;

		// work out radius
		Vector ObjMaxs, ObjMins;
		GetRenderBounds( ObjMins, ObjMaxs );

		m_iRadius = RoundFloatToInt( ( ( ( ObjMaxs.x - ObjMins.x ) + ( ObjMaxs.y - ObjMins.y ) * 0.5f ) * METERS_PER_INCH ) * 0.5f );
	}
	else if( Type == DATA_UPDATE_DATATABLE_CHANGED )
	{
		// send obj update
		GetINSHUDHelper( )->SendObjUpdate( this );
	}
}

//=========================================================
//=========================================================
void C_INSObjective::SetName( const char *pszName )
{
	Q_strncpy( m_szName, pszName, MAX_OBJNAME_LENGTH );
}

//=========================================================
//=========================================================
#define OBJ_LUMINANCE_LEVEL 0.88f

void C_INSObjective::SetColor( const Color clrColor )
{
	m_clrColor = clrColor;

	HLSColor WhiteColor( clrColor );
	WhiteColor.m_flLuminance = OBJ_LUMINANCE_LEVEL;

	m_clrWhitedColor = WhiteColor.ConvertToRGB( );
}

//=========================================================
//=========================================================
const char *C_INSObjective::GetPhonetischName( void ) const
{
	return g_pszPhoneticAlphabet[ m_iPhonetic ];
}

//=========================================================
//=========================================================
char C_INSObjective::GetPhonetischLetter( void ) const
{
	const char *pszPhonetischName = GetPhonetischName( );
	return pszPhonetischName[ 0 ];
}

//=========================================================
//=========================================================
bool C_INSObjective::IsCapturing( void ) const
{
	return ( m_iCaptureProgress != OBJ_CAPTUREINVALID );
}

//=========================================================
//=========================================================
bool C_INSObjective::IsPlayerInside( C_BasePlayer *pPlayer )
{
	Vector ObjMins, ObjMaxs, PlayerMins, PlayerMaxs;

	GetRenderBoundsWorldspace( ObjMins, ObjMaxs );
	pPlayer->GetRenderBoundsWorldspace( PlayerMins, PlayerMaxs );

	return IsBoxIntersectingBox( ObjMins, ObjMaxs, PlayerMins, PlayerMaxs );
}