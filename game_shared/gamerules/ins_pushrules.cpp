//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_obj_shared.h"
#include "ins_pushrules.h"

#ifdef GAME_DLL

#include "play_team_shared.h"
#include "imc_config.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
REGISTER_GAMERULES_CLASS( CPushRules );
DECLARE_GAMETYPE( GAMETYPE_PUSH, CPushRules );

//=========================================================
//=========================================================
#ifdef GAME_DLL

void SendProxy_AttackableObj( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CPushRules *pPushRules = ( CPushRules* )pStruct;

	int iObjID = 0;

	if( INSRules( )->ObjectiveCaptureAllowed( ) )
	{
		CINSObjective *pNextPossibleObjective = NULL;

		if( ( pNextPossibleObjective = pPushRules->GetNextPossibleObjective( ) ) != NULL )
			iObjID = pNextPossibleObjective->GetOrderID( );
	}

	pOut->m_Int = iObjID;
}

#else

void RecvProxy_AttackableObj( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CPushRules *pPushRules = ( CPushRules* )pStruct;
	pPushRules->SetAttackableObjID( pData->m_Value.m_Int );
}

#endif

BEGIN_NETWORK_TABLE_NOBASE( CPushRules, DT_PushRules )

#ifdef GAME_DLL

	SendPropIntWithMinusOneFlag( "aobj", 0, SIZEOF_IGNORE, MAX_OBJECTIVES_BITS, SendProxy_AttackableObj ),

#else

	RecvPropIntWithMinusOneFlag( "aobj", 0, SIZEOF_IGNORE, RecvProxy_AttackableObj ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
CPushRules::CPushRules( )
{
#ifdef GAME_DLL

	m_iAttackingTeamID = TEAM_NEUTRAL;

	m_bUseInitialAttackingObj = false;
	m_pInitialAttackingObjID = NULL;

#else

	m_iAttackableObjID = INVALID_OBJECTIVE;

#endif
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CPushRules::CreateProxy( void )
{
	CreateEntityByName( "pushrules" );
}

//=========================================================
//=========================================================
bool CPushRules::IsEnoughObjectives( void ) const
{
	return ( GetObjectiveCount( ) >= 3 );
}

//=========================================================
//=========================================================
int CPushRules::FinalSetup( int &iT1Spawn, int &iT2Spawn, int &iT1Orient )
{
	CINSObjective *pFirstObj = GetFirstObjective( );
	CINSObjective *pLastObj = GetLastObjective( );

	// check that one of the end objectives are hidden
	if( !pFirstObj->IsHidden( ) && !pLastObj->IsHidden( ) )
		return GMWARN_PUSH_NO_HIDDEN_OBJ;

	// check that both of them aren't hidden
	if( pFirstObj->IsHidden( ) && pLastObj->IsHidden( ) )
		return GMWARN_PUSH_BOTH_HIDDEN;

	// check that none of the objectives are neutral
	for( int i = 0; i < GetObjectiveCount( ); i++ )
	{
		if( GetObjective( i )->GetCapturedTeam( ) == TEAM_NEUTRAL )
			return GMWARN_PUSH_NEUTRAL_OBJS;
	}

	// check that the end objectives are assigned to opposite teams
	if( pFirstObj->GetCapturedTeam( ) == pLastObj->GetCapturedTeam( ) )
		return GMWARN_PUSH_BAD_HIDDEN_OBJ_TEAMS;

	// set the hidden objective
	int iHiddenObjID = ( pFirstObj->IsHidden( ) ) ? GetFirstObjectiveID( ) : GetLastObjectiveID( );
	CINSObjective *pHiddenObj = GetObjective( iHiddenObjID );

	// set the orientation
	if( pFirstObj->GetCapturedTeam( ) == TEAM_ONE )
		iT1Orient = T1ORIENTATION_LOW;
	else
		iT1Orient = T1ORIENTATION_HIGH;

	// check that all the Objectives (expect the hidden objective) are not hidden and of the correct team
	int iCheckTeam = TEAM_ONE;

	if( pHiddenObj->GetCapturedTeam( ) == TEAM_ONE )
		iCheckTeam = TEAM_TWO;

	for( int i = 0; i < GetObjectiveCount( ); i++ )
	{
		if( iHiddenObjID == i )
			continue;

		CINSObjective *pObjective = GetObjective( i );

		if( pObjective->IsHidden(  ) )
			return GMWARN_PUSH_MISPLACED_HIDDEN_OBJ;

		if( pObjective->GetCapturedTeam( ) != iCheckTeam )
			return GMWARN_PUSH_BAD_DEFENDER_TEAM;
	}

	// set the attacking team
	m_iAttackingTeamID = pHiddenObj->GetCapturedTeam( );
	int iDefendingTeamID = FlipPlayTeam( m_iAttackingTeamID );

	// find the initial attacking obj
	int iInitialDefendingObjID = iHiddenObjID;
	
	if( pHiddenObj->GetCapturedTeam( ) == TEAM_ONE )
	{
		if( iT1Orient == T1ORIENTATION_LOW )
			iInitialDefendingObjID++;
		else
			iInitialDefendingObjID--;
	}
	else
	{
		if( iT1Orient == T1ORIENTATION_LOW )
			iInitialDefendingObjID--;
		else
			iInitialDefendingObjID++;
	}

	// set the inital attacking obj
	m_pInitialAttackingObjID = GetObjective( iInitialDefendingObjID );

	// try and overload the defending spawn
	for( int i = 0; i < GetObjectiveCount( ); i++ )
	{
		CINSObjective *pObjective = GetObjective( i );

		if( !pObjective->IsStartSpawn( ) || pObjective->GetCaptureTeam( ) != iDefendingTeamID )
			continue;

		iInitialDefendingObjID = i;
		break;
	}

	// set the initial spawns
	iT1Spawn = ( m_iAttackingTeamID == TEAM_ONE ) ? iHiddenObjID : iInitialDefendingObjID;
	iT2Spawn = ( m_iAttackingTeamID == TEAM_ONE ) ? iInitialDefendingObjID : iHiddenObjID;

	return 0;
}

//=========================================================
//=========================================================
bool CPushRules::ForceTimerType( void )
{
	if( !IsUsingObjectives( ) )
		return false;

	CPlayTeam *pAttackingTeam = GetGlobalPlayTeam( m_iAttackingTeamID );
	return ( pAttackingTeam && pAttackingTeam->GetIMCTeamConfig( )->GetNumWaves( ) == UNLIMITED_SUPPLIES );
}

//=========================================================
//=========================================================
void CPushRules::ResetSpawnPoints( void )
{
	BaseClass::ResetSpawnPoints( );

	m_bUseInitialAttackingObj = true;
}

//=========================================================
//=========================================================
void CPushRules::UpdateSpawnPoints( CINSObjective *pCapturedObj, int &iT1Spawn, int &iT2Spawn )
{
	int iAttackObjID, iDefendObjID;
	iAttackObjID = iDefendObjID = INVALID_OBJECTIVE;

	CObjectiveIterator ObjectiveIterator( m_iAttackingTeamID );

	for( CINSObjective *pObjective = NULL; !ObjectiveIterator.End( ); ObjectiveIterator++ )
	{
		pObjective = ObjectiveIterator.Current( );
		iDefendObjID = ObjectiveIterator.CurrentID( );

		if( pObjective->GetCapturedTeam( ) != m_iAttackingTeamID )
			break;

		iAttackObjID = iDefendObjID;
	}

	if( iAttackObjID == INVALID_OBJECTIVE || iDefendObjID == INVALID_OBJECTIVE )
	{
		AssertMsg( false, "CPushRules::UpdateSpawnPoints, Failed" );
		return;
	}
	
	if( m_iAttackingTeamID == TEAM_ONE )
	{
		iT1Spawn = iAttackObjID;
		iT2Spawn = iDefendObjID;
	}
	else
	{
		iT1Spawn = iDefendObjID;
		iT2Spawn = iAttackObjID;
	}

	m_bUseInitialAttackingObj = false;

	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CPushRules::CalculateDefaultOrders( int iTeamID, CObjOrder &ObjOrder )
{
	CINSObjective *pObjective = GetNextPossibleObjective( );

	if( pObjective )
		ObjOrder.Init( pObjective );
}

//=========================================================
//=========================================================
CINSObjective *CPushRules::GetNextPossibleObjective( void )
{
	if( m_bUseInitialAttackingObj )
		return m_pInitialAttackingObjID;

	int iT1Spawn, iT2Spawn;
	RunningMode( )->GetCurrentSpawns( iT1Spawn, iT2Spawn );

	return GetObjective( ( m_iAttackingTeamID == TEAM_ONE ) ? iT2Spawn : iT1Spawn );
}

//=========================================================
//=========================================================
int CPushRules::ObjectiveCapturePoints( bool bFollowingOrders ) const
{
	return bFollowingOrders ? MORALE_OBJECTIVE_PUSH_FOLLOWING_ORDERS : MORALE_OBJECTIVE_PUSH_LONE_WOLF;
}

//=========================================================
//=========================================================
int CPushRules::ObjectiveLeaderPoints( void ) const
{
	return MORALE_OBJECTIVE_LEADERSHIP;
}

//=========================================================
//=========================================================
bool CPushRules::ObjectiveCaptureAllowed( const CINSObjective *pObjective, int iTeamID )
{
	// check team
	if( m_iAttackingTeamID != iTeamID && iTeamID != TEAM_NEUTRAL )
		return false;

	return ( GetNextPossibleObjective( ) == pObjective );
}

//=========================================================
//=========================================================
void CPushRules::ObjectiveCaptured( CINSObjective *pObjective )
{
	BaseClass::ObjectiveCaptured( pObjective );

	if( pObjective )
		pObjective->SetOrdersAllowed( false );
}

//=========================================================
//=========================================================
bool CPushRules::CheckForWinner( int &iWinningTeamID )
{
	// only reset the round when all the objectives are owned by one team
	for( int i = 0; i < GetObjectiveCount( ); i++ )
	{
		if( GetObjective( i )->GetCapturedTeam( ) != m_iAttackingTeamID )
			return false;
	}

	iWinningTeamID = m_iAttackingTeamID;
	return true;
}

#else

//=========================================================
//=========================================================
bool C_PushRules::ObjectiveCaptureAllowed( const CINSObjective *pObjective, int iTeamID )
{
	return ( pObjective->GetOrderID( ) == m_iAttackableObjID );
}

#endif

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( pushrules, CPushRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( PushRulesProxy, DT_PushRulesProxy )

#ifdef GAME_DLL

void *SendProxy_PushGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->SetAllRecipients( );
	return ( void* )INSRules( );
}

#else

void RecvProxy_PushGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
{
	*pOut = ( void* )INSRules( );
}

#endif

BEGIN_NETWORK_TABLE( CPushRulesProxy, DT_PushRulesProxy )

#ifdef GAME_DLL

	SendPropDataTable("pushrules_data", 0, &REFERENCE_SEND_TABLE( DT_PushRules ), SendProxy_PushGameRules)

#else

	RecvPropDataTable( "pushrules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_PushRules ), RecvProxy_PushGameRules )

#endif

END_NETWORK_TABLE( )