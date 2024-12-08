//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_obj_shared.h"
#include "ins_battlerules.h"
#include "play_team_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
REGISTER_GAMERULES_CLASS( CBattleRules );
DECLARE_GAMETYPE( GAMETYPE_BATTLE, CBattleRules );

//=========================================================
//=========================================================
#ifdef GAME_DLL

int GetAllowedObjective( CBattleRules *pBattleRules, int iTeamID )
{
	int iObjID = INVALID_OBJECTIVE;

	if( INSRules( )->ObjectiveCaptureAllowed( ) )
	{
		CINSObjective *pObjective = pBattleRules->GetAllowedObj( iTeamID );

		if( pObjective )
			iObjID = pObjective->GetOrderID( );
	}

	return iObjID + 1;
}

void SendProxy_AObjT2( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CBattleRules *pBattleRules = ( CBattleRules* )pStruct;
	pOut->m_Int = GetAllowedObjective( pBattleRules, TEAM_ONE );
}

void SendProxy_AObjT1( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CBattleRules *pBattleRules = ( CBattleRules* )pStruct;
	pOut->m_Int = GetAllowedObjective( pBattleRules, TEAM_TWO );
}

#else

void SetAllowedObjective( CBattleRules *pBattleRules, int iTeam, int iObjID )
{
	CINSObjective *pObj = C_INSObjective::GetObjective( iObjID );
	pBattleRules->SetCaptureObj( iTeam, pObj );
}

void RecvProxy_AObjT1(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	CBattleRules *pBattleRules = ( CBattleRules* )pStruct;
	SetAllowedObjective( pBattleRules, TEAM_ONE, pData->m_Value.m_Int - 1 );
}

void RecvProxy_AObjT2(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	CBattleRules *pBattleRules = ( CBattleRules* )pStruct;
	SetAllowedObjective( pBattleRules, TEAM_TWO, pData->m_Value.m_Int - 1 );
}

#endif

BEGIN_NETWORK_TABLE_NOBASE( CBattleRules, DT_BattleRules )

#ifdef GAME_DLL

	SendPropInt( "aobj_t1", 0, SIZEOF_IGNORE, MAX_OBJECTIVES_BITS, 0, SendProxy_AObjT1 ),
	SendPropInt( "aobj_t2", 0, SIZEOF_IGNORE, MAX_OBJECTIVES_BITS, 0, SendProxy_AObjT2 ),

#else

	RecvPropInt( "aobj_t1", 0, SIZEOF_IGNORE, 0, RecvProxy_AObjT1 ),
	RecvPropInt( "aobj_t2", 0, SIZEOF_IGNORE, 0, RecvProxy_AObjT2 ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
bool CBattleRules::ObjectiveCaptureAllowed( const CINSObjective *pObjective, int iTeamID )
{
	if( !BaseClass::ObjectiveCaptureAllowed( pObjective, iTeamID ) )
		return false;

	CINSObjective *pCaptureObj = GetAllowedObj( iTeamID );
	return ( pCaptureObj && pObjective == pCaptureObj );
}

//=========================================================
//=========================================================
void CBattleRules::SetCaptureObj( int iTeamID, CINSObjective *pObjective )
{
	m_pCaptureObjs[ TeamToPlayTeam( iTeamID ) ] = pObjective;
}

//=========================================================
//=========================================================
CINSObjective *CBattleRules::GetAllowedObj( int iTeamID ) const
{
	return m_pCaptureObjs[ TeamToPlayTeam( iTeamID ) ];
}

#ifdef GAME_DLL

//=========================================================
//=========================================================
CBattleRules::CBattleRules( )
{
	ResetAllowedCaptureObjs( );
}

//=========================================================
//=========================================================
void CBattleRules::CreateProxy( void )
{
	CreateEntityByName( "battlerules" );
}

//=========================================================
//=========================================================
bool CBattleRules::IsEnoughObjectives( void ) const
{
	return ( GetObjectiveCount( ) >= 3 );
}

//=========================================================
//=========================================================
void CBattleRules::RoundReset( void )
{
	BaseClass::RoundReset( );

	CalculateCaptureObjs( );
}

//=========================================================
//=========================================================
void CBattleRules::ResetAllowedCaptureObjs( void )
{
	memset( m_pCaptureObjs, NULL, sizeof( m_pCaptureObjs ) );
}

//=========================================================
//=========================================================
void CBattleRules::CalculateCaptureObjs( void )
{
	ResetAllowedCaptureObjs( );

	for( int iTeamID = TEAM_ONE; iTeamID <= TEAM_TWO; iTeamID++ )
	{
		CObjectiveIterator ObjectiveIterator( iTeamID );

		for( CINSObjective *pObjective = NULL; !ObjectiveIterator.End( ); ObjectiveIterator++ )
		{
			pObjective = ObjectiveIterator.Current( );

			if( pObjective->GetCapturedTeam( ) == iTeamID )
				continue;

			SetCaptureObj( iTeamID, pObjective );
			break;
		}
	}

	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CBattleRules::ObjectiveCaptured( CINSObjective *pObjective )
{
	CalculateCaptureObjs( );

	BaseClass::ObjectiveCaptured( pObjective );
}

//=========================================================
//=========================================================
bool CBattleRules::IsCutupsAllowed( void ) const
{
	return false;
}

//=========================================================
//=========================================================
void CBattleRules::CalculateDefaultOrders( int iTeamID, CObjOrder &ObjOrder )
{
	ObjOrder.Init( GetAllowedObj( iTeamID ) );
}

#endif

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( battlerules, CBattleRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( BattleRulesProxy, DT_BattleRulesProxy )

#ifdef GAME_DLL

void *SendProxy_BattleGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->SetAllRecipients( );
	return ( void* )INSRules( );
}

#else

void RecvProxy_BattleGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
{
	*pOut = ( void* )INSRules( );
}

#endif

BEGIN_NETWORK_TABLE( CBattleRulesProxy, DT_BattleRulesProxy )

#ifdef GAME_DLL

	SendPropDataTable( "battlerules_data", 0, &REFERENCE_SEND_TABLE( DT_BattleRules ), SendProxy_BattleGameRules )

#else

	RecvPropDataTable( "battlerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_BattleRules ), RecvProxy_BattleGameRules )

#endif

END_NETWORK_TABLE( )