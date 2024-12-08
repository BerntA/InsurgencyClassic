//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_ffrules.h"

#ifdef GAME_DLL

#include "imc_config.h"
#include "ins_obj.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
REGISTER_GAMERULES_CLASS( CFFRules );
DECLARE_GAMETYPE( GAMETYPE_FIREFIGHT, CFFRules );

//=========================================================
//=========================================================
#ifdef GAME_DLL

enum FindStartSpawnType_t
{
	FSSTYPE_INVALID = 0,
	FSSTYPE_NEUTRAL,				// neutral spawn point
	FSSTYPE_OWNED					// owned spawn points
};

//=========================================================
//=========================================================
bool CFFRules::IsEnoughObjectives( void ) const
{
	return ( GetObjectiveCount( ) >= 2 );
}

//=========================================================
//=========================================================
int CFFRules::FinalSetup( int &iT1Spawn, int &iT2Spawn, int &iT1Orient )
{
	// check objective consistancy
	CINSObjective *pFirstObj = GetFirstObjective( );
	CINSObjective *pLastObj = GetLastObjective( );

	if( GetObjectiveCount( ) == 2 )
	{
		if( pFirstObj->GetCapturedTeam( ) == TEAM_NEUTRAL || 
			pLastObj->GetCapturedTeam( ) == TEAM_NEUTRAL ||
			pFirstObj->GetCapturedTeam( ) == pLastObj->GetCapturedTeam( ) )
			return GMWARN_GENERAL_INVALID_TEAM_STARTS;
	}
	else
	{
		// check that the end objectives are hidden
		if( !pFirstObj->IsHidden( ) || !pLastObj->IsHidden( ) )
			return GMWARN_FF_NO_HIDDEN_OBJS;

		// check that they're the ONLY hidden objectives
		int iFirstObj, iLastObj;
		iFirstObj = GetFirstObjectiveID( );
		iLastObj = GetLastObjectiveID( );

		for( int i = 0; i < GetObjectiveCount( ); i++ )
		{
			if( i == iFirstObj || i == iLastObj )
				continue;

			if( GetObjective( i )->IsHidden( ) )
				return GMWARN_FF_INCORRECT_HIDDEN;
		}

		// check that the end objectives are not neutral
		if( pFirstObj->GetCapturedTeam( ) == TEAM_NEUTRAL || pLastObj->GetCapturedTeam( ) == TEAM_NEUTRAL )
			return GMWARN_FF_NEUTRAL_HIDDEN_OBJS;

		// check that the hidden objectives are assigned to opposite teams
		if( pFirstObj->GetCapturedTeam( ) == pLastObj->GetCapturedTeam( ) )
			return GMWARN_FF_BAD_HIDDEN_OBJ_TEAMS;
	}

	// find and set the starts for each team
	int iStartSpawnsType = FindStartSpawns( iT1Spawn, iT2Spawn );

	if( iStartSpawnsType == FSSTYPE_INVALID )
		return GMWARN_GENERAL_INVALID_TEAM_STARTS;

	// set the orientation
	int iT2Orient;

	if( iStartSpawnsType == FSSTYPE_OWNED )
	{
		iT1Orient = iT1Spawn;
		iT2Orient = iT2Spawn;
	}
	else // FSSTYPE_NEUTRAL
	{
		int iHiddenT1, iHiddenT2;

		if( pFirstObj->GetCapturedTeam( ) == TEAM_ONE )
		{
			iHiddenT1 = GetFirstObjectiveID( );
			iHiddenT2 = GetLastObjectiveID( );
		}
		else
		{
			iHiddenT1 = GetLastObjectiveID( );
			iHiddenT2 = GetFirstObjectiveID( );
		}

		iT1Orient = iHiddenT1;
		iT2Orient = iHiddenT2;
	}

	iT1Orient = ( iT1Orient > iT2Orient ? T1ORIENTATION_HIGH : T1ORIENTATION_LOW );

	return 0;
}

//=========================================================
//=========================================================
int CFFRules::FindStartSpawns( int &iT1Spawn, int &iT2Spawn )
{
	int iFindType = FSSTYPE_OWNED;

	for( int i = 0; i < GetObjectiveCount( ); i++ )
	{
		if( iT1Spawn != INVALID_OBJECTIVE && iT2Spawn != INVALID_OBJECTIVE )
			break;

		CINSObjective *pObjective = GetObjective( i );

		if( !pObjective->IsStartSpawn( ) )
			continue;

		int iInitialTeam = pObjective->GetInitialTeam( );

		if( iInitialTeam == TEAM_NEUTRAL )
		{
			iFindType = FSSTYPE_NEUTRAL;
			iT1Spawn = iT2Spawn = i;
			break;
		}

		if( iT1Spawn == INVALID_OBJECTIVE && iInitialTeam == TEAM_ONE )
		{
			iT1Spawn = i;
			continue;
		}

		if( iT2Spawn == INVALID_OBJECTIVE && iInitialTeam == TEAM_TWO )
		{
			iT2Spawn = i;
			continue;
		}
	}

	if( iT1Spawn == INVALID_OBJECTIVE || iT2Spawn == INVALID_OBJECTIVE )
		return FSSTYPE_INVALID;

	return iFindType;
}

//=========================================================
//=========================================================
void CFFRules::UpdateSpawnPoints( CINSObjective *pCapturedObj, int &iT1Spawn, int &iT2Spawn )
{
	// update the cut-ups
	UpdateCutups( pCapturedObj );

	// set the start spawns
	iT1Spawn = FindObjForSpawn( TEAM_ONE );
	iT2Spawn = FindObjForSpawn( TEAM_TWO );
}

//=========================================================
//=========================================================
bool CFFRules::IsCutupsAllowed( void ) const
{
	return IMCConfig( )->IsCutupsAllowed( );
}

//=========================================================
//=========================================================
void CFFRules::UpdateCutups( CINSObjective *pCapturedObj )
{
	if( !IsCutupsAllowed( ) )
		return;

	bool bT1CutupState, bT2CutupState;

	bT1CutupState = ( ( pCapturedObj->GetCapturedTeam( ) == TEAM_ONE ) ? false : true );
	bT2CutupState = !bT1CutupState;

	CObjectiveIterator ObjectiveIterator( pCapturedObj->GetCapturedTeam( ) );
	int iCapturedTeamID;

	for( CINSObjective *pObjective = NULL; !ObjectiveIterator.End( ); ObjectiveIterator++ )
	{
		pObjective = ObjectiveIterator.Current( );

		if( pObjective == pCapturedObj )
			break;

		if( pObjective->IsHidden( ) )
			continue;

		iCapturedTeamID = pObjective->GetCapturedTeam( );

		if( iCapturedTeamID == TEAM_ONE )
			pObjective->SetCutUp( bT1CutupState );
		else if( iCapturedTeamID == TEAM_TWO )
			pObjective->SetCutUp( bT2CutupState );
	}
}

//=========================================================
//=========================================================
int CFFRules::FindObjForSpawn( int iTeamID )
{
	int iStartObj = INVALID_OBJECTIVE;

	CObjectiveIterator ObjectiveIterator( iTeamID );
	int iCapturedTeamID;

	for( CINSObjective *pObjective = NULL; !ObjectiveIterator.End( ); ObjectiveIterator++ )
	{
		pObjective = ObjectiveIterator.Current( );
		iCapturedTeamID = pObjective->GetCapturedTeam( );

		if( iCapturedTeamID == TEAM_NEUTRAL )
			continue;

		if( iCapturedTeamID == iTeamID && !pObjective->IsCutUp( ) )
			iStartObj = ObjectiveIterator.CurrentID( );
	}

	return iStartObj;
}

//=========================================================
//=========================================================
int CFFRules::ObjectiveCapturePoints( bool bFollowingOrders ) const
{
	return ( bFollowingOrders ? MORALE_OBJECTIVE_FOLLOWING_ORDERS : MORALE_OBJECTIVE_LONE_WOLF );
}

//=========================================================
//=========================================================
int CFFRules::ObjectiveLeaderPoints( void ) const
{
	return MORALE_OBJECTIVE_LEADERSHIP;
}

//=========================================================
//=========================================================
void CFFRules::CalculateDefaultOrders( int iTeamID, CObjOrder &ObjOrder )
{
	// find the objective closet to the root
	CINSObjective *pClosetAttackable = NULL;
	CObjectiveIterator ObjectiveIterator( iTeamID );

	for( CINSObjective *pObjective = NULL; !ObjectiveIterator.End( ); ObjectiveIterator++ )
	{
		pObjective = ObjectiveIterator.Current( );

		if( pObjective->GetCapturedTeam( ) != iTeamID )
		{
			pClosetAttackable = pObjective;
			break;
		}
	}

	if( pClosetAttackable )
		ObjOrder.Init( pClosetAttackable );
}

//=========================================================
//=========================================================
bool CFFRules::CheckForWinner( int &iWinningTeam )
{
	// only reset the round when all the objectives are owned by one team
	int iCurrentWinner = TEAM_NEUTRAL;

	for(int i = 0; i < GetObjectiveCount( ); i++)
	{
		CINSObjective *pObjective = GetObjective( i );

		if( pObjective->IsHidden( ) )
			continue;

		if( !pObjective->IsCaptured( ) )
			return false;

		if( iCurrentWinner != TEAM_NEUTRAL )
		{
			if( iCurrentWinner != pObjective->GetCapturedTeam( ) )
				return false;
		}
		else
		{
			iCurrentWinner = pObjective->GetCapturedTeam( );
		}
	}

	iWinningTeam = iCurrentWinner;

	return true;
}

//=========================================================
//=========================================================
int CFFRules::GetDefaultWinner( void ) const
{
	// count out who has the most objs
	int iT1Ownership, iT2Ownership;
	iT1Ownership = iT2Ownership = 0;

	for( int i = 0; i < GetObjectiveCount( ); i++ )
	{
		int iCapturedTeam = GetObjective( i )->GetCapturedTeam( );

		if( iCapturedTeam == TEAM_ONE )
			iT1Ownership++;
		else if( iCapturedTeam == TEAM_TWO)
			iT2Ownership++;
	}

	if( iT1Ownership == iT2Ownership )
		return TEAM_NEUTRAL;

	return ( ( iT1Ownership > iT2Ownership ) ? TEAM_ONE : TEAM_TWO );
}

#endif