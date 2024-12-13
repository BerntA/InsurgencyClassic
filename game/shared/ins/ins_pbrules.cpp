//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_pbrules.h"

#ifdef GAME_DLL

#include "imc_config.h"
#include "ins_player.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef GAME_DLL

class CPowerBall : public CBaseAnimating
{
	DECLARE_CLASS( CPowerBall, CBaseAnimating );
	DECLARE_DATADESC( );

public:
	CPowerBall( );

	void CreateBall( void );
	void RemoveBall( void );

private:
	void Spawn( void );

	void PlayerTouch( CBaseEntity *pOther );
};

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_powerball, CPowerBall )

//=========================================================
//=========================================================
BEGIN_DATADESC( CPowerBall )

	DEFINE_FUNCTION( PlayerTouch )

END_DATADESC( )

//=========================================================
//=========================================================
CPowerBall::CPowerBall( )
{
}

//=========================================================
//=========================================================
void CPowerBall::CreateBall( void )
{
	SetTouch( &CPowerBall::PlayerTouch );
	RemoveEffects( EF_NODRAW );
}

//=========================================================
//=========================================================
void CPowerBall::RemoveBall( void )
{
	AddEffects( EF_NODRAW );
	SetTouch( NULL );
}

//=========================================================
//=========================================================
void CPowerBall::Spawn( void )
{
	SetModel( POWERBALL_MODEL );

	AddEffects( EF_NOSHADOW );

	RemoveBall( );
}

//=========================================================
//=========================================================
void CPowerBall::PlayerTouch( CBaseEntity *pOther )
{
	if( !pOther || !pOther->IsPlayer( ) )
		return;

	CINSPlayer *pPlayer = ToINSPlayer( pOther );
	pPlayer->GivePowerball( );

	RemoveBall( );
}

#endif

//=========================================================
//=========================================================
REGISTER_GAMERULES_CLASS( CPBRules );
DECLARE_GAMETYPE( GAMETYPE_POWERBALL, CPBRules );

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CPBRules::Precache( void )
{
	BaseClass::Precache( );

	CBaseEntity::PrecacheModel( POWERBALL_MODEL );
}

//=========================================================
//=========================================================
bool CPBRules::IsEnoughObjectives( void ) const
{
	return ( GetObjectiveCount( ) == 2 );
}

//=========================================================
//=========================================================
int CPBRules::FinalSetup( int &iT1Spawn, int &iT2Spawn, int &iT1Orient )
{
	// can't have moving spawns
	if( IMCConfig( )->IsMovingSpawns( ) )
		return GMWARN_PB_MOVINGSPAWNS;

	CINSObjective *pFirstObj = GetFirstObjective( );
	CINSObjective *pLastObj = GetLastObjective( );

	// can't be hidden
	if( pFirstObj->IsHidden( ) || pLastObj->IsHidden( ) )
		return GMWARN_PB_HIDDENOBJS;

	// can't be neutral
	if( pFirstObj->GetCapturedTeam( ) == TEAM_NEUTRAL || pLastObj->GetCapturedTeam( ) == TEAM_NEUTRAL )
		return GMWARN_PB_TEAMNEUTRAL;

	// need to be different
	if( pFirstObj->GetCapturedTeam( ) == pLastObj->GetCapturedTeam( ) )
		return GMWARN_PB_TEAMSAME;

	// find powerball
	m_pPowerBall = ( CPowerBall* )gEntList.FindEntityByClassname( NULL, "ins_powerball" );

	if( !m_pPowerBall )
		return GMWARN_PB_NOPOWERBALL;

	// set data
	if( pFirstObj->GetCapturedTeam( ) == TEAM_ONE )
	{
		iT1Spawn = GetFirstObjectiveID( );
		iT2Spawn = GetLastObjectiveID( );
		iT1Orient = T1ORIENTATION_LOW;
	}
	else
	{
		iT1Spawn = GetLastObjectiveID( );
		iT2Spawn = GetFirstObjectiveID( );
		iT1Orient = T1ORIENTATION_HIGH;
	}

	return 0;
}

//=========================================================
//=========================================================
void CPBRules::UpdateSpawnPoints( CINSObjective *pCapturedObj, int &iT1Spawn, int &iT2Spawn )
{
	Assert( false );
}

//=========================================================
//=========================================================
void CPBRules::CalculateDefaultOrders( int iTeamID, CObjOrder &ObjOrder )
{
	// just attack the other team's current spawn
	int iT1CurrentSpawn, iT2CurrentSpawn, iObjID;
	RunningMode( )->GetCurrentSpawns( iT1CurrentSpawn, iT2CurrentSpawn );

	iObjID = ( ( iTeamID == TEAM_ONE ) ? iT2CurrentSpawn : iT1CurrentSpawn );

	if( IsValidObjective( iObjID ) )
		ObjOrder.Init( GetObjective( iObjID ) );
}

//=========================================================
//=========================================================
int CPBRules::ObjectiveCapturePoints( bool bFollowingOrders ) const
{
	return 10;
}

//=========================================================
//=========================================================
int CPBRules::ObjectiveLeaderPoints( void ) const
{
	return 0;
}

//=========================================================
//=========================================================
bool CPBRules::CheckForWinner( int &iWinningTeam )
{
	CINSObjective *pFirstObj = GetFirstObjective( );
	CINSObjective *pLastObj = GetLastObjective( );

	if( pFirstObj->GetCapturedTeam( ) == pLastObj->GetCapturedTeam( ) )
	{
		iWinningTeam = pFirstObj->GetCapturedTeam( );
		return true;
	}

	return false;
}

//=========================================================
//=========================================================
int CPBRules::GetDefaultWinner( void ) const
{
	return TEAM_NEUTRAL;
}

//=========================================================
//=========================================================
void CPBRules::PlayerAddWeapons( CINSPlayer *pPlayer )
{
	// pPlayer->GiveWeapon( WEAPON_POWERBALL );
}

#endif
