//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_gamerules.h"
#include "ins_obj.h"
#include "ins_objmarker.h"
#include "imc_config.h"
#include "ins_squad_shared.h"
#include "play_team_shared.h"
#include "ins_weaponcache.h"
#include "ins_utils.h"

#ifdef _DEBUG
#include "ndebugoverlay.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
enum ObjSpawnDebug_t
{
	DEBUGSPAWN_NONE = 0,
	DEBUGSPAWN_SHOWALL,
	DEBUGSPAWN_SHOWSPAWNS,
	DEBUGSPAWN_SHOW
};

//=========================================================
//=========================================================
ConVar debug_spawns( "ins_debug_spawns", "0", FCVAR_CHEAT, "Spawnpoint Debugging State", true, DEBUGSPAWN_NONE, true, DEBUGSPAWN_SHOW );
ConVar debug_spawnobj( "ins_debug_spawnobj", "0", FCVAR_CHEAT, "Debug Specific Objective", true, 0, true, MAX_OBJECTIVES );

//=========================================================
//=========================================================
CINSObjective *CINSRules::GetObjective( int iID ) const
{
	if( !IsValidObjective( iID ) )
	{
		Assert( false );
		return NULL;
	}

	return m_Objectives[ iID ];
}

//=========================================================
//=========================================================
bool CINSRules::IsValidObjective( int iID ) const
{
	return m_Objectives.IsValidIndex( iID );
}

//=========================================================
//=========================================================
int CINSRules::GetObjectiveCount( void ) const
{
	return m_Objectives.Count( );
}

//=========================================================
//=========================================================
CINSObjective *CINSRules::GetUnorderedObjective( int iID ) const
{
	if( iID < 0 || iID >= MAX_OBJECTIVES )
	{
		Assert( false );
		return NULL;
	}

	return m_pUnsortedObjectives[ iID ];
}

//=========================================================
//=========================================================
bool CINSRules::IsUsingObjectives( void ) const
{
	return ( NeedsObjectives( ) && m_bUseObjs );
}

//=========================================================
//=========================================================
bool CINSRules::ObjectiveCaptureAllowed( const CINSObjective *pObjective, int iTeamID )
{
	return ObjectiveCaptureAllowed( );
}

//=========================================================
//=========================================================
bool CINSRules::ObjectiveCaptureAllowed( void )
{
	return ( IsUsingObjectives( ) && IsModeRunning( ) && RunningMode( )->ObjectivesEnabled( ) && RunningMode( )->CanWinRound( ) );
}

//=========================================================
//=========================================================
void CINSRules::ObjectiveCaptured( CINSObjective *pCapturedObj )
{
	if( !IsModeRunning( ) )
	{
		Assert( false );
		return;
	}

	int iCapturedTeamID = pCapturedObj->GetCapturedTeam( ) ;

	if( !IsPlayTeam( iCapturedTeamID ) )
	{
		Assert( false );
		return;
	}

	// mark it
	CPlayTeam::ObjectiveCaptured( );

	// if the round is extended, end the game
	if( RunningMode( )->IsRoundExtended( ) )
	{
		RunningMode( )->RoundWon( iCapturedTeamID, ENDGAME_WINCONDITION_EXTENDED );
		return;
	}

	int iWinningTeam;

	// check that all the objectives have been captured by a team
	 if( CheckForWinner( iWinningTeam ) )
	 {
		RunningMode( )->RoundWon( iWinningTeam, ENDGAME_WINCONDITION_OBJ, pCapturedObj );
		return;
	}

	// update spawn objectives
	if( IMCConfig( )->IsMovingSpawns( ) )
	{
		int iT1Spawn, iT2Spawn;
		iT1Spawn = iT2Spawn = INVALID_OBJECTIVE;

		UpdateSpawnPoints( pCapturedObj, iT1Spawn, iT2Spawn );

		RunningMode( )->SetCurrentSpawns( iT1Spawn, iT2Spawn );

		// ... and have at least one spawning zone for each team
		if( HasSpawns( iWinningTeam ) )
		{
			RunningMode( )->RoundWon( iWinningTeam, ENDGAME_WINCONDITION_OBJ, pCapturedObj );
			return;
		}
	}

	// tell the team who captured
	CPlayTeam *pTeam = GetGlobalPlayTeam( iCapturedTeamID );
	Assert( pTeam );

	if( pTeam )
		pTeam->CapturedObjective( );

	// update the capturing objectives
	ObjectiveCapturingUpdate( );

	// check the current orders
	DefaultOrderRollout( false );
}

//=========================================================
//=========================================================
void CINSRules::DefaultOrderRollout( bool bReportInvalid )
{
	if( !IsUsingObjectives( ) )
		return;

	// replace invalid orders with default ones
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++)
	{
		CPlayTeam *pTeam = GetGlobalPlayTeam( i );

		if( !pTeam )
			continue;

		CObjOrder ObjOrders;
		CalculateDefaultOrders( i, ObjOrders );

		for( int j = 0; j < pTeam->GetSquadCount( ); j++ )
		{
			CINSSquad *pSquad = pTeam->GetSquad( j );

			if( !pSquad )
				continue;

			const CObjOrder &CurrentObjOrders = pSquad->GetObjOrders( );

			if( !IsValidObjOrder( i, CurrentObjOrders.Objective( ) ) )
			{
				pSquad->AssignObjOrders( ObjOrders.Objective( ) );

				if( bReportInvalid )
					pSquad->NotifyInvalidObjOrders( );
			}
		}
	}
}

//=========================================================
//=========================================================
bool CINSRules::IsValidObjOrder( int iTeamID, CINSObjective *pObjective )
{
	if( !IsPlayTeam( iTeamID ) )
		return false;
	
	if( !pObjective )
		return false;

	if( pObjective->IsHidden( ) )
		return false;

	if( !pObjective->IsOrdersAllowed( ) )
		return false;

	return true;
}

//=========================================================
//=========================================================
void CINSRules::UpdateSpawnPoints( CINSObjective *pCapturedObj, int &iT1Spawn, int &iT2Spawn )
{
	iT1Spawn = iT2Spawn = INVALID_OBJECTIVE;
	AssertMsg( NeedsObjectives( ), "CINSRules::UpdateSpawnPoints, not Overloaded" );
}

//=========================================================
//=========================================================
void CINSRules::GetStartSpawns( int &iT1Spawn, int &iT2Spawn )
{
	iT1Spawn = m_iStartObjs[ 0 ];
	iT2Spawn = m_iStartObjs[ 1 ];
}

//=========================================================
//=========================================================
void CINSRules::CalculateDefaultOrders( int iTeamID, CObjOrder &ObjOrders )
{
	ObjOrders.Reset( );
	AssertMsg( NeedsObjectives( ), "CINSRules::CalculateDefaultOrders, not Overloaded" );
}

//=========================================================
//=========================================================
bool CINSRules::HasSpawns( int &iWinningTeam )
{
	int iT1CurrentSpawn, iT2CurrentSpawn;
	RunningMode( )->GetCurrentSpawns( iT1CurrentSpawn, iT2CurrentSpawn );

	// no-one has won, just continue
	if( iT1CurrentSpawn != INVALID_OBJECTIVE && iT2CurrentSpawn != INVALID_OBJECTIVE )
		return false;

	// when they're both invalid, force a draw
	if( iT1CurrentSpawn == INVALID_OBJECTIVE && iT2CurrentSpawn == INVALID_OBJECTIVE )
	{
		iWinningTeam = TEAM_NEUTRAL;
		return true;
	}

	// set winning team
	iWinningTeam = ( ( iT1CurrentSpawn == INVALID_OBJECTIVE ) ? TEAM_TWO : TEAM_ONE );
	return true;
}

//=========================================================
//=========================================================
CObjectiveIterator::CObjectiveIterator( int iTeamID )
{
	m_iTeamID = iTeamID;

	bool bT1LowOrientation = ( INSRules( )->GetT1ObjOrientation( ) == T1ORIENTATION_LOW );
	int iLastObjID = INSRules( )->GetObjectiveCount( ) - 1;

	if( m_iTeamID == TEAM_ONE )
	{
		m_iCurrentObjective = bT1LowOrientation ? 0 : iLastObjID;
		m_iMod = bT1LowOrientation ? 1 : -1;
	}
	else
	{
		m_iCurrentObjective = bT1LowOrientation ? iLastObjID : 0;
		m_iMod = bT1LowOrientation ? -1 : 1;
	}
}

//=========================================================
//=========================================================
bool CObjectiveIterator::End( void ) const
{
	return ( m_iCurrentObjective < 0 || m_iCurrentObjective >= INSRules( )->GetObjectiveCount( ) );
}

//=========================================================
//=========================================================
void CObjectiveIterator::operator++( int )
{
	m_iCurrentObjective += m_iMod;
}

//=========================================================
//=========================================================
CINSObjective *CObjectiveIterator::Current( void ) const
{
	return INSRules( )->GetObjective( m_iCurrentObjective );
}

//=========================================================
//=========================================================
int CINSRules::SetupObjectives( void )
{
	// check that the gamerules want to use them
	if( !NeedsObjectives( ) )
		return 0;

	// find objectives
	AddObjectives( "ins_objective" );		// INS Objective

	if( IMCConfig( )->AdditionalMapSupportAllowed( ) )
	{
		AddObjectives( "func_buyzone" );		// CS Objective
		AddObjectives( "dod_capture_area" );	// DoD Objective
	}

	// ensure the correct number of objectives
	if( !IsEnoughObjectives( ) )
		return GMWARN_GENERAL_NOTENOUGHOBJS;

	// sort objectives
	if( !SortObjectives( ) )
		return GMWARN_GENERAL_INVALIDOBJS;

	// check the consitancy of the objectives
	int iT1Spawn, iT2Spawn, iT1Orient;
	iT1Spawn = iT2Spawn = INVALID_OBJECTIVE;
	iT1Orient = T1ORIENTATION_INVALID;

	int iConsistRet = FinalSetup( iT1Spawn, iT2Spawn, iT1Orient );

	if( iConsistRet != 0 )
		return iConsistRet;

	// ensure sane values
	if( iT1Spawn == INVALID_OBJECTIVE || iT2Spawn == INVALID_OBJECTIVE || iT1Orient == T1ORIENTATION_INVALID )
	{
		AssertMsg( false, "CINSRules::SetupObjectives, FinalSetup has lied about Executing Successfully" );
		return GMWARN_GENERAL_FAILEDSETUP;
	}

	// set values
	m_iStartObjs[ 0 ] = iT1Spawn;
	m_iStartObjs[ 1 ] = iT2Spawn;
	m_iT1ObjOrientation = iT1Orient;

	// trim spawnpoints
	RemoveCollidingSpawns( );

	// load spawn-points
	CINSObjective::FindSpawnPoints( );

	// find objective markers
	CINSObjective::SetupObjectiveMarkers( );

	// join the weapon caches to the objectives
	int iCacheRet = SetupWeaponCaches( );

	if( iCacheRet != 0 )
		return iCacheRet;

	// allow objective usage
	m_bUseObjs = true;

	// enable touch on objectives and update
	for( int i = 0; i < m_Objectives.Count( ); i++ )
		m_Objectives[ i ]->FinalSetup( );

	return 0;
}

//=========================================================
//=========================================================
void CINSRules::AddObjectives( const char *pszObjectiveClass )
{
	CBaseEntity *pObjFind = NULL;

	while( ( pObjFind = gEntList.FindEntityByClassname( pObjFind, pszObjectiveClass ) ) != NULL )
	{
		CINSObjective *pObjective = ( CINSObjective* )pObjFind;
		pObjective->Init( );

		if( pObjective->HasParent( ) )
			m_Objectives.AddToTail( pObjective );
	}
}

//=========================================================
//=========================================================
bool CINSRules::SortObjectives( void )
{
	// now create a new	vector of objectives and copy it to m_Objectives
	CUtlVector< CINSObjective* > Objectives;
	CINSObjective *pUnsortedObjectives[ MAX_OBJECTIVES ];

	memset( pUnsortedObjectives, NULL, sizeof( pUnsortedObjectives ) );

	for( int i = 0; i < IMCConfig( )->GetNumObjectives( ); i++ )
	{
		CIMCObjective *pIMCObj = IMCConfig( )->GetObj( i );

		if( !pIMCObj )
		{
			Assert( false );
			continue;
		}

		bool bFound = false;

		for( int j = 0; j < m_Objectives.Count( ); j++ )
		{
			CINSObjective *pObjective = m_Objectives[ j ];

			if( pIMCObj->GetID( ) != pObjective->GetID( ) )
				continue;

			if( pObjective->GetOrderID( ) != INVALID_OBJECTIVE )
			{
				AssertMsg( false, "CINSRules::SortObjectives, Duplicate Objective ID" );
				return false;
			}

			// set the orderid
			pObjective->SetOrderID( Objectives.AddToTail( pObjective ) );

			// put in the unsorted array
			pUnsortedObjectives[ pIMCObj->GetID( ) ] = pObjective;

			// it's been found
			bFound = true;

			break;
		}

		if( !bFound )
			return false;
	}

	// copy it
	m_Objectives = Objectives;
	memcpy( m_pUnsortedObjectives, pUnsortedObjectives, sizeof( m_pUnsortedObjectives ) );

#ifdef TESTING

	Msg( "Ordered Objective\n" );

	// print out objective details
	for( int i = 0; i < m_Objectives.Count( ); i++ )
	{
		CINSObjective *pCurrentObj = m_Objectives[ i ];
		Msg( "Objective %i (%i): %s : %s\n", pCurrentObj->GetOrderID( ), pCurrentObj->GetID( ), pCurrentObj->GetName( ), ( pCurrentObj->IsStartSpawn( ) ? "true" : "false" ) );
	}

#endif

	return true;
}

//=========================================================
//=========================================================
void CINSRules::RemoveCollidingSpawns( void )
{
	if( IMCConfig( )->IsCertifiedIMC( ) )
		return;

	CBaseEntity *pEntity = NULL;

	// look through spawns
	for( int i = 0; i < CSpawnPoint::CountSpawns( ); i++ )
	{
		CSpawnPoint *pSpawnPoint = CSpawnPoint::GetSpawn( i );

		if( !pSpawnPoint )
			continue;

		// ensure the origin isn't in the world
		if( UTIL_PointContents( UTIL_SpawnPositionOffset( pSpawnPoint ) ) & MASK_SOLID )
		{
			EchoError( "spawn", "colliding with the world" );

			m_BadSpawns.AddToTail( pSpawnPoint );

			continue;
		}

		// ensure there isn't anything nearby
		for( CEntitySphereQuery sphere( pSpawnPoint->GetAbsOrigin( ), SPAWNCOLLIDE_DISTANCE ); ( pEntity = sphere.GetCurrentEntity( ) ) != NULL; sphere.NextEntity( ) )
		{
			if( !pEntity )
				continue;

			if( pEntity == pSpawnPoint )
				continue;

			CSpawnPoint *pCollidingSpawn = dynamic_cast< CSpawnPoint* >( pEntity );

			if( !pCollidingSpawn )
				continue;

			if( pSpawnPoint->GetParentID( ) != pCollidingSpawn->GetParentID( ) )
				continue;

			if( m_BadSpawns.Find( pSpawnPoint ) != m_BadSpawns.InvalidIndex( ) )
				continue;

			EchoError( "spawn", "colliding with other spawns" );

			m_BadSpawns.AddToTail( pSpawnPoint );
		}
	}
	
	// remove badspawns
	for( int i = 0; i < m_BadSpawns.Count( ); i++ )
		CSpawnPoint::TrimSpawn( m_BadSpawns[ i ] );
}

//=========================================================
//=========================================================
void CINSRules::ResetSpawnPoints( void )
{
	RunningMode( )->SetCurrentSpawns( m_iStartObjs[ 0 ], m_iStartObjs[ 1 ] );
}

//=========================================================
//=========================================================
void CINSRules::ObjectiveCapturingUpdate( void )
{
	// find out if any objectives need to start capturing, or think if they already are
	for(int i = 0; i < m_Objectives.Count( ); i++)
	{
		CINSObjective *pObjective = m_Objectives[ i ];

		if( pObjective->IsCapturing( ) )
			pObjective->CheckTimedCapture( );
	}
}

//=========================================================
//=========================================================
typedef CINSWeaponCache *WeaponCache_t;

int SortWeaponCacheRoutine( const WeaponCache_t *pLeft, const WeaponCache_t *pRight )
{
	CINSWeaponCache *pWeaponCacheLeft = *pLeft;
	CINSWeaponCache *pWeaponCacheRight = *pRight;

	return ( pWeaponCacheLeft->GetID( ) - pWeaponCacheRight->GetID( ) );
}

int CINSRules::SetupWeaponCaches( void )
{
	// first find the weapon cache placements
	CBaseEntity *pWCFind = NULL;
	CUtlVector< CINSWeaponCache* > WeaponCaches;

	while( ( pWCFind = gEntList.FindEntityByClassname( pWCFind, "ins_weaponcache" ) ) != NULL)
		WeaponCaches.AddToTail( ( CINSWeaponCache* )pWCFind );

	WeaponCaches.Sort( SortWeaponCacheRoutine );

	// ... connect it all together
	for( int i = 0; i < IMCConfig( )->GetWeaponCacheCount( ); i++ )
	{
		CIMCWeaponCache *pIMCWeaponCache = IMCConfig( )->GetWeaponCache( i );

		if( !pIMCWeaponCache )
		{
			Assert( false );
			return GMWARN_GENERAL_INVALIDCACHE;
		}

		CINSWeaponCache *pINSWeaponCache = NULL;

		// try and find it
		int iWantedID = pIMCWeaponCache->GetID( );
			
		for( int j = 0; j < WeaponCaches.Count( ); j++ )
		{
			CINSWeaponCache *pINSFindWeaponCache = WeaponCaches[ j ];

			if( pINSFindWeaponCache->GetID( ) == iWantedID )
			{
				pINSWeaponCache = pINSFindWeaponCache;
				break;
			}
		}

		if( !pINSWeaponCache )
			return GMWARN_GENERAL_INVALIDCACHE;

		// setup the cache
		pINSWeaponCache->Setup( pIMCWeaponCache );

		// add it to its list (either with its parent obj
		if( pIMCWeaponCache->GetParentObjID( ) != WCACHE_NOPARENTOBJ )
		{
			CINSObjective *pObjective = m_pUnsortedObjectives[ pIMCWeaponCache->GetParentObjID( ) ];

			if( pObjective )
			{
				pObjective->AddWeaponCache( pINSWeaponCache );
			}
			else
			{
				AssertMsg( false, "CINSRules::SetupWeaponCaches, Obj Parent should be Valid" );
				UTIL_Remove( pINSWeaponCache );
			}
		}
		else
		{
			m_WeaponCaches.AddToTail( pINSWeaponCache );
		}

		// if its stock is deployed, add to a quick lookup list
		if( pIMCWeaponCache->GetFlags( ) & WCACHE_FLAG_DEPLOYSTOCK )
			m_WeaponCachesDeploy.AddToTail( pINSWeaponCache );
	}

	return 0;
}

//=========================================================
//=========================================================
void CINSRules::CreateWeaponCaches( void )
{
	CINSObjective::CreateWeaponCaches( NULL, m_WeaponCaches, &IMCConfig( )->GetWeaponCacheRandomData( ) );
}

//=========================================================
//=========================================================
void CINSRules::RestockWeaponCaches( int iTeamID )
{
	for( int i = 0; i < m_WeaponCachesDeploy.Count( ); i++ )
	{
		CINSWeaponCache *pWeaponCache = m_WeaponCachesDeploy[ i ];

		if( pWeaponCache && pWeaponCache->GetTeam( ) == iTeamID )
			pWeaponCache->Restock( );
	}
}

//=========================================================
//=========================================================
CBaseEntity *CINSRules::GetObjectiveSpawnPoint( CINSPlayer *pPlayer )
{
	Assert( IsModeRunning( ) );

	static CSpawnGlobal GlobalSpawns;

	// ensure thats there are spawn points in the game
	if( !IsModeRunning( ) || !GlobalSpawns.IsValid( ) )
	{
#ifdef _DEBUG

	Warning( "SPAWN: Spawned Badly due to Invalid Mode or no Spawns\n" );

#endif

		return GetLastSpawnOption( pPlayer );
	}

	// if we're using objectives, find the spawn group
	bool bValidSpawnManager = false;

	CSpawnManager *pSpawnManager = NULL;

	if( IsUsingObjectives( ) )
	{
		CINSObjective *pObjective = NULL;

		if( RunningMode( )->ObjectivesEnabled( ) )
		{
			// when the objectives aren't disabled, do it correctly
			int iT1CurrentSpawn, iT2CurrentSpawn;
			RunningMode( )->GetCurrentSpawns( iT1CurrentSpawn, iT2CurrentSpawn );

			int iObjID = ( pPlayer->GetTeamID( ) == TEAM_ONE ) ? iT1CurrentSpawn : iT2CurrentSpawn;

			if( IsValidObjective( iObjID ) )
				pObjective = GetObjective( iObjID );
		}

		if( pObjective )
		{
			pSpawnManager = pObjective->GetSpawnGroupPlayer( pPlayer );
			Assert( pSpawnManager );

			if( pSpawnManager && pSpawnManager->IsValid( ) )
				bValidSpawnManager = true;
		}
	}

	// if invalid, use the global manager
	if( !bValidSpawnManager )
	{
	#ifdef _DEBUG

		Warning( "SPAWN: Spawned Badly due to Invalid Spawn Manager\n" );

	#endif

		// ... only echo error when gamerules say
		if( IsUsingObjectives( ) )
			EchoSpawnError( pPlayer );

		pSpawnManager = &GlobalSpawns;
	}

	// find a valid spawn-point
	CSpawnPoint *pSpawnPoint = pSpawnManager->FindSpawn( pPlayer );

	if( !pSpawnPoint )
		return GetLastSpawnOption( pPlayer );

	// kill any players inside
	pSpawnPoint->KillColliding( pPlayer );

	return pSpawnPoint;
}

//=========================================================
//=========================================================
int CINSRules::GetFirstObjectiveID( void )
{
	return 0;
}

//=========================================================
//=========================================================
int CINSRules::GetLastObjectiveID( void )
{
	return ( INSRules( )->GetObjectiveCount( ) - 1 );
}

//=========================================================
//=========================================================
CINSObjective *CINSRules::GetFirstObjective( void )
{
	return ( INSRules( )->GetObjective( GetFirstObjectiveID( ) ) );
}

//=========================================================
//=========================================================
CINSObjective *CINSRules::GetLastObjective( void )
{
	return ( INSRules( )->GetObjective( GetLastObjectiveID( ) ) );
}

//=========================================================
//=========================================================
void CINSRules::HandleObjectiveDebug( void )
{
	if( !IsModeRunning( ) )
		return;

	switch( debug_spawns.GetInt( ) )
	{
		case DEBUGSPAWN_NONE:
		{
			break;
		}
	
		case DEBUGSPAWN_SHOWSPAWNS:
		{
			int iT1CurrentSpawn, iT2CurrentSpawn;
			RunningMode( )->GetCurrentSpawns( iT1CurrentSpawn, iT2CurrentSpawn );

			Color T1Color, T2Color;
			T1Color = GetGlobalPlayTeam( TEAM_ONE )->GetColor( );
			T2Color = GetGlobalPlayTeam( TEAM_TWO )->GetColor( );

			if( iT1CurrentSpawn != INVALID_OBJECTIVE )
				DrawSpawnDebug_Obj( m_Objectives[ iT1CurrentSpawn ], &T1Color );

			if( iT2CurrentSpawn != INVALID_OBJECTIVE )
				DrawSpawnDebug_Obj( m_Objectives[ iT2CurrentSpawn ], &T2Color );

			break;
		}

		case DEBUGSPAWN_SHOWALL:
		{
			for( int i = 0; i < GetObjectiveCount( ); i++ )
				DrawSpawnDebug_Obj( GetObjective( i ) );

			break;
		}

		case DEBUGSPAWN_SHOW:
		{
			int iObjID = debug_spawnobj.GetInt( );

			if( IsValidObjective( iObjID ) )
				DrawSpawnDebug_Obj( GetObjective( iObjID ) );
		}
	}
}

//=========================================================
//=========================================================
void CINSRules::DrawSpawnDebug_Obj( CINSObjective *pObjective, const Color *pForceColor )
{
	Color ObjColor;
	
	if( !pForceColor )
		ObjColor = pObjective->GetColor( );
	else
		ObjColor = *pForceColor;

	// draw obj
	DrawSpawnDebug_Entity( ObjColor, pObjective, false );

	// draw spawns
	int iCapturedTeam = pObjective->GetCapturedTeam( );

	if( iCapturedTeam == TEAM_NEUTRAL )
		return;

	if( pObjective->UseReinforcementSpawns( ) )
		DrawSpawnDebug_ObjSpawns( pObjective, ObjColor, &pObjective->GetReinforcementSpawns( ) );
	else
		DrawSpawnDebug_ObjSpawns( pObjective, ObjColor, &pObjective->GetMainSpawns( ) );
}

//=========================================================
//=========================================================
void CINSRules::DrawSpawnDebug_ObjSpawns( CINSObjective *pObjective, const Color &ObjColor, CObjSpawns *pObjSpawns )
{
	CSpawnGroupHandler *pSpawnHandler = pObjSpawns->GetSpawnHandler( );

	if( pSpawnHandler )
		pSpawnHandler->DrawSpawnDebug( ObjColor );
}

//=========================================================
//=========================================================
void CINSRules::DrawSpawnDebug_SpawnGroup( const Color &ObjColor, CSpawnGroup *pSpawnGroup )
{
	if( !pSpawnGroup )
		return;

	for( int i = 0; i < pSpawnGroup->Count( ); i++ )
		DrawSpawnDebug_Entity( ObjColor, pSpawnGroup->GetSpawn( i ), true );
}

//=========================================================
//=========================================================
void CINSRules::DrawSpawnDebug_Entity( const Color &ObjColor, CBaseEntity *pEntity, bool bSpawn )
{
	Vector ObjMins, ObjMaxs;
	ObjMins = ObjMaxs = vec3_origin;

	pEntity->CollisionProp( )->WorldSpaceSurroundingBounds( &ObjMins, &ObjMaxs );

	if( bSpawn )
		ObjMaxs += Vector( 20, 20, 40 );

	NDebugOverlay::Box( vec3_origin, ObjMins, ObjMaxs, 
		ObjColor.r( ), ObjColor.g( ), ObjColor.b( ), 25, 0.0f );
}

//=========================================================
//=========================================================
CON_COMMAND( ins_debug_printspawns, "Print out Information on Spawns" )
{
	if( !INSRules( ) )
		return;

	for( int i = 0; i < INSRules( )->GetObjectiveCount( ); i++ )
	{
		CINSObjective *pObjective = INSRules( )->GetObjective( i );

		if( pObjective )
			pObjective->PrintSpawnStatus( );
	}
}