//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "commander_shared.h"
#include "ins_obj_shared.h"

#ifdef CLIENT_DLL

#include "ins_player_shared.h"
#include "view.h"
#include "ins_utils.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef GAME_DLL

DEFINE_STRING_LOOKUP_CONSTANTS( int, ranks )

	ADD_LOOKUP( RANK_PRIVATE )
	ADD_LOOKUP( RANK_LCORPORAL )
	ADD_LOOKUP( RANK_CORPORAL )
	ADD_LOOKUP( RANK_SERGEANT )
	ADD_LOOKUP( RANK_LIEUTENANT )

END_STRING_LOOKUP_CONSTANTS( )

#endif

//=========================================================
//=========================================================
bool UTIL_ValidRank( int iRankID )
{
	return ( iRankID > INVALID_RANK && iRankID < RANK_COUNT );
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

const char *g_pszRankIcons[ RANK_COUNT ] = {
	"pvt",	// RANK_PRIVATE
	"lpv",	// RANK_LCORPORAL
	"cpl",	// RANK_CORPORAL
	"sgt",	// RANK_SERGEANT
	"lt"	// RANK_LIEUTENANT
};

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

const char *g_pszObjOrderTypeNames[ ORDERTYPE_OBJ_COUNT ] = 
{
	"Unassigned",	// ORDERTYPE_OBJ_NONE
	"Attack",		// ORDERTYPE_OBJ_ATTACK
	"Defend"		// ORDERTYPE_OBJ_DEFEND
};

const char *g_pszUnitOrderTypeNames[ ORDERTYPE_UNIT_COUNT ] =
{
	"Secure",		// ORDERTYPE_UNIT_SECURE
	"Move",			// ORDERTYPE_UNIT_MOVE
	"Guard",		// ORDERTYPE_UNIT_GUARD
	"Regroup",		// ORDERTYPE_UNIT_REGROUP
	"Stealth",		// ORDERTYPE_UNIT_STEALTH
};

#endif

//=========================================================
//=========================================================
CObjOrder::CObjOrder( )
{
	Reset( );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CObjOrder::Init( CINSObjective *pObjective )
{
	Assert( pObjective );
	m_pObjective = pObjective;
}

//=========================================================
//=========================================================
void CObjOrder::Init( void )
{
	Reset( );
}

#else

//=========================================================
//=========================================================
void CObjOrder::Init( bf_read &Msg )
{
	int iObjID = Msg.ReadByte( );

	if( iObjID != INVALID_OBJECTIVE )
		m_pObjective = C_INSObjective::GetObjective( iObjID );
	else
		m_pObjective = NULL;		
}

#endif

//=========================================================
//=========================================================
bool CObjOrder::IsValidOrder( int iOrderType )
{
	return ( iOrderType > ORDERTYPE_OBJ_NONE && iOrderType < ORDERTYPE_OBJ_COUNT );
}

//=========================================================
//=========================================================
void CObjOrder::Reset( void )
{
	m_pObjective = NULL;
}

//=========================================================
//=========================================================
bool CObjOrder::HasOrders( void ) const
{
	return ( m_pObjective != NULL );
}

//=========================================================
//=========================================================
bool CObjOrder::HasOrder( CINSObjective *pObjective ) const
{
	return ( m_pObjective == pObjective );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CObjOrder::Send( IRecipientFilter &filter ) const
{
	int iObjID = INVALID_OBJECTIVE;

	if( HasOrders( ) )
		iObjID = m_pObjective->GetOrderID( );

	UserMessageBegin( filter, "ObjOrder" );

		WRITE_BYTE( iObjID );

	MessageEnd( );
}

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

int CObjOrder::OrderType( void ) const
{
	// PNOTE: this could work by putting a recvproxy in C_INSObjective
	// and keeping it stored it here - experiment later!

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );
	return UTIL_CaculateObjType( pPlayer, m_pObjective );
}

#endif

//=========================================================
//=========================================================
static const char *g_pszUnitModels[ ORDERTYPE_UNIT_COUNT ] = {
	"models/HUD/waypoint.mdl",		// ORDERTYPE_UNIT_SECURE
	"models/HUD/waypoint.mdl",		// ORDERTYPE_UNIT_MOVE
	"models/HUD/waypoint.mdl",		// ORDERTYPE_UNIT_GUARD
	"models/HUD/waypoint.mdl",		// ORDERTYPE_UNIT_REGROUP
	"models/HUD/waypoint.mdl"		// ORDERTYPE_UNIT_STEALTH
};

//=========================================================
//=========================================================
CUnitOrder::CUnitOrder( )
{
	Reset( );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

//=========================================================
//=========================================================
void CUnitOrder::Precache( void )
{
#if 0

	for( int i = 0; i < ORDERTYPE_UNIT_COUNT; i++ )
		CBaseEntity::PrecacheModel( g_pszUnitModels[ i ] );

#endif

	CBaseEntity::PrecacheModel( g_pszUnitModels[ 0 ] );
}

//=========================================================
//=========================================================
void CUnitOrder::Init( int iOrderType, const Vector &vecPosition )
{
	m_iOrderType = iOrderType;
	m_vecPosition = vecPosition;
}

//=========================================================
//=========================================================
#else

void CUnitOrder::Init( bf_read &msg )
{
	m_iOrderType = msg.ReadByte( );

	m_vecPosition.x = msg.ReadFloat( );
	m_vecPosition.y = msg.ReadFloat( );
	m_vecPosition.z = msg.ReadFloat( );
}

#endif

//=========================================================
//=========================================================
bool CUnitOrder::IsValidOrder( int iOrderType )
{
	return ( iOrderType >= 0 && iOrderType < ORDERTYPE_UNIT_COUNT );
}

//=========================================================
//=========================================================
void CUnitOrder::Reset( void )
{
	// reset all data
	m_iOrderType = ORDERTYPE_UNIT_INVALID;
	m_vecPosition.Init( );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CUnitOrder::Send( IRecipientFilter &filter )
{
	UserMessageBegin( filter, "UnitOrder" );

		WRITE_BYTE( m_iOrderType );

		if( HasOrders( ) )
		{
			WRITE_FLOAT( m_vecPosition.x );
			WRITE_FLOAT( m_vecPosition.y );
			WRITE_FLOAT( m_vecPosition.z );
		}

	MessageEnd( );
}

#endif

//=========================================================
//=========================================================
bool CUnitOrder::HasOrders( void ) const
{
	return IsValidOrder( m_iOrderType );
}

//=========================================================
//=========================================================
const char *CUnitOrder::ModelName( int iID )
{
	return g_pszUnitModels[ iID ];
}

//=========================================================
//=========================================================
bool UTIL_ValidPlayerOrder( int iID )
{
	return ( iID >= 0 && iID < PORDER_COUNT );
}

//=========================================================
//=========================================================
bool UTIL_ValidPlayerOrderResponse( int iID )
{
	return ( iID >= 0 && iID < PORESPONSE_COUNT );
}
