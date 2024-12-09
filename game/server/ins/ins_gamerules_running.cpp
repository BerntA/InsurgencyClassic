//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_gamerules.h"

#ifdef GAME_DLL

#include "imc_config.h"
#include "play_team_shared.h"
#include "ins_obj.h"
#include "team_lookup.h"
#include "team_spawnpoint.h"
#include "viewport_panel_names.h"
#include "player_resource.h"
#include "ins_squad_shared.h"
#include "ins_player.h"
#include "weapon_ballistic_base.h" // remove when hack is removed

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define DEFAULTORDERS_FROZENFRAC 0.8f

#define RAGDOLLFADE_MIN 2.0f
#define RAGDOLLFADE_MAX 60.0f

//=========================================================
//=========================================================
enum WarmupTypes_t
{
	WARMUP_NONE = 0,		// no warmup
	WARMUP_TIMED,			// timed warmup
	WARMUP_TIMED_START,		// timed warmup, only on RESET_GAME
	WARMUP_FULLROUND_START	// full-round warmup, only on RESET_GAME
};

//=========================================================
//=========================================================
enum ResetRoundTypes_t
{
	RESET_NONE = 0,
	RESET_GAME,				// reset scores and makes everybody choose squads again
	RESET_ROUND,			// make everyone choose squads again (well, depends on squad type etc)
	RESET_RESPAWN,			// just respawn everybody
};

//=========================================================
//=========================================================
enum ResetRoundReturnCodes_t
{
	RESETCODE_NONE = 0,		// do nothing (relates to RESET_RESTART)
	RESETCODE_SQUADORDER,	// reset the round normally, back to squad selection
	RESETCODE_CHANGELEVEL	// change the level
};

//=========================================================
//=========================================================
#define WARMUP_RESTARTTIME 5.0f

#define WARMUPTIME_MIN WARMUP_RESTARTTIME * 2

#define WINLIMIT_MAX 128

//=========================================================
//=========================================================
ConVar roundlimit( "mp_roundlimit", "10", FCVAR_NOTIFY | FCVAR_REPLICATED, "Defines how Many Rounds Played until the Map Rotates", true, 0, true, WINLIMIT_MAX );
ConVar winlimit( "mp_winlimit", "10", FCVAR_NOTIFY | FCVAR_REPLICATED, "Defines how Many Rounds Played by One Team before Map Rotates", true, 0, true, WINLIMIT_MAX );
ConVar freezetime( "mp_freezetime", "4.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "How Long to Freeze the Player Before the Game Starts", true, 3.0f, true, 10.0f );

ConVar warmuptype( "ins_warmuptype", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Defines Warm-Up Type", true, WARMUP_NONE, true, WARMUP_FULLROUND_START );
ConVar warmuptime( "ins_warmuptime", "15", FCVAR_NOTIFY | FCVAR_REPLICATED, "Warm-Up Time before a Game (in Seconds)", true, WARMUPTIME_MIN, true, MAX_ROUNDLENGTH );

ConVar roundtimer( "ins_roundtimer", "300", FCVAR_REPLICATED | FCVAR_NOTIFY, "Length of Time (in Seconds) of each Round", true, MIN_ROUNDLENGTH, true, MAX_ROUNDLENGTH );
ConVar timertype( "ins_timertype", "2", FCVAR_REPLICATED | FCVAR_NOTIFY, "Defines which Timer to Use", true, ROUNDTIMER_NONE, true, ROUNDTIMER_IMC);
ConVar endgametime( "ins_endgametime", "3", FCVAR_REPLICATED | FCVAR_NOTIFY, "Change how Long the Winning Players get to Wander Around", true, 0, true, 10 );

ConVar randomlayout( "ins_randomlayout", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Determines wether or not to use Random Layouts", true, 0, true, 1 );

ConVar objdisable( "ins_objdisable", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Ignore Objective States" );

ConVar deadinfofull( "ins_deadinfofull", "1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Determines whether or not to use Full Death Information", true, 0, true, 1 );

ConVar scorefreeze( "ins_scorefrozen", "0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Determines whether or not the Score is Frozen", true, 0, true, 1 );
ConVar maskotherteam( "ins_maskotherteam", "0", FCVAR_NOTIFY, "When Enabled the Other Teams Score will not be Updated until the End of the Round", true, 0, true, 1 );

ConVar deadcam_modes( "ins_deadcam_modes", "0", FCVAR_REPLICATED, "Restricts Spectator Modes", true, OBS_ALLOWMODE_ALL, true, OBS_ALLOWMODE_NONE );
ConVar deadcam_targets( "ins_deadcam_targets", "1", FCVAR_REPLICATED, "Restricts Spectator Targets", true, OBS_ALLOWTARGETS_ALL, true, OBS_ALLOWTARGETS_SQUAD );

ConVar ragdollkeep( "ins_ragdoll_keep", "0", FCVAR_NOTIFY, "Defines whether Ragdolls will Remain until a Restart", true, 0, true, 1 );
ConVar ragdollmin( "ins_ragdoll_min", "24.0", FCVAR_NOTIFY, "The Minimum Time a Ragdoll will Remain Before Disappearing", true, RAGDOLLFADE_MIN, true, RAGDOLLFADE_MAX );
ConVar ragdollmax( "ins_ragdoll_max", "60.0", FCVAR_NOTIFY, "The Maximum Time a Ragdoll will Remain Before Disappearing", true, RAGDOLLFADE_MIN, true, RAGDOLLFADE_MAX );

//=========================================================
//=========================================================
void UTIL_ExecuteRestart(int iType, const CCommand& args)
{
	if( !UTIL_IsCommandIssuedByServerAdmin( ) )
		return;

	if (args.ArgC() < 2)
		return;

	CINSRules *pRules = INSRules( );
	if( pRules && pRules->IsModeRunning( ) )
		pRules->RunningMode()->RequestReset(iType, atoi(args[1]), true);
}

//=========================================================
//=========================================================
CON_COMMAND( ins_restart_game, "Restart the Game" )
{
	UTIL_ExecuteRestart( RESET_GAME, args );
}

//=========================================================
//=========================================================
CON_COMMAND( ins_restart_round, "Restart the Round" )
{
	UTIL_ExecuteRestart( RESET_ROUND, args );
}

//=========================================================
//=========================================================
int CRunningMode::Init( void )
{
	m_bObjectivesEnabled = false;

	m_iStatus = 0;

	m_iT1CurrentSpawn = m_iT2CurrentSpawn = INVALID_OBJECTIVE;

	m_iWarmupType = warmuptype.GetInt( );
	m_iWarmupLength = ( m_iWarmupType != WARMUP_NONE ) ? warmuptime.GetInt( ) : 0;
	m_flWarmupStartTime = 0.0f;

	m_iFreezeTimeLength = 0;
	m_flFreezeTime = 0.0f;

	m_bWantEndGame = false;
	m_flEndGameTime = 0.0f;
	m_pEndGameValues = NULL;

	UpdateConsoleValues( );

	m_bMaskOtherTeam = false;
	m_bScoreFrozen = false;
	m_bDeathInfoFull = false;

	m_bFirstRestart = true;
	m_bRoundExtended = false;
	RequestReset( RESET_RESPAWN, 0, false );

	HandlePlayerCount( );

	m_flDefaultOrderRollout = 0.0f;

	m_flRagdollFadeTime = RAGDOLLFADE_MAX;

	return GRMODE_NONE;
}

//=========================================================
//=========================================================
void CRunningMode::Shutdown( void )
{
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		CPlayTeam *pTeam = GetGlobalPlayTeam( i );
		Assert( pTeam );
		
		for( int j = 0; j < pTeam->GetSquadCount( ); j++ )
		{
			CINSSquad *pSquad = pTeam->GetSquad( j );
			Assert( pSquad );

			pSquad->ResetOrders( );
		}
	}
}

//=========================================================
//=========================================================
int CRunningMode::Think( void )
{
	// draw badspawns
	INSRules( )->DrawBadSpawns( );

	// check for ragdoll removal
	if( !ragdollkeep.GetBool( ) )
	{
		for( int i = 0; i < m_RagdollFadeList.Count( ); i++ )
		{
			CBaseEntity *pRagdollEntity = m_RagdollFadeList[ i ];
			CINSRagdoll *pRagdoll = ( CINSRagdoll* )pRagdollEntity;

			if( pRagdoll && gpGlobals->curtime > ( pRagdoll->m_flStartTime + m_flRagdollFadeTime ) )
				pRagdoll->Vanish( );
		}
	}

	// check for the end-game
	if( m_flEndGameTime != 0.0f && gpGlobals->curtime >= m_flEndGameTime )
	{
		EndGameFinished( );

		// don't do it again
		m_flEndGameTime = 0.0f;
	}

	// check for a roundreset request
	int iRoundReset = CheckRoundReset( );

	if( iRoundReset != GRMODE_NONE )
		return iRoundReset;

	// check for the end of warmup
	if( IsWarmingup( ) && !IsRestarting( ) && 
		( m_iWarmupType == WARMUP_TIMED || m_iWarmupType == WARMUP_TIMED_START ) && 
		( ( gpGlobals->curtime - m_flWarmupStartTime ) >= m_iWarmupLength ) )
	{
		RequestReset( RESET_ROUND, WARMUP_RESTARTTIME, true );
	}

	// check if the players can start the round
	if( m_flFreezeTime != 0.0f )
	{
		if( gpGlobals->curtime > m_flDefaultOrderRollout )
		{
			INSRules( )->DefaultOrderRollout( true );

			m_flDefaultOrderRollout = 0.0f;
		}

		if( gpGlobals->curtime >= m_flFreezeTime )
		{
			SetFrozenState( false );

			m_flFreezeTime = 0.0f;
		}
	}

	// handle objective draw
	INSRules( )->HandleObjectiveDebug( );

	// the reset 
	if( IsRestarting( ) )
		return GRMODE_NONE;

	// check round timer
	if( !m_bRoundExtended && m_iRoundTimerType != ROUNDTIMER_NONE &&		
		( !IsWarmingup( ) || ( IsWarmingup( ) && m_iWarmupType == WARMUP_FULLROUND_START ) ) &&
		( gpGlobals->curtime - m_flRoundStartTime ) >= m_iRoundLength )
	{
		int iDefaultWinner = INSRules( )->GetDefaultWinner( );

		if( IsWarmingup( ) || iDefaultWinner != TEAM_UNASSIGNED )
		{
			RoundWon( iDefaultWinner, ENDGAME_WINCONDITION_TIMER );

			return GRMODE_NONE;
		}

		RoundExtended( );
	}

	// check reinforcements clock
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		GetGlobalPlayTeam( i )->CheckReinforcements( );

	return GRMODE_NONE;
}

//=========================================================
//=========================================================
void CRunningMode::PlayerSpawn( CINSPlayer *pPlayer )
{
	// don't bother doing anything if they're not on a play team or in a valid squad
	if( !pPlayer->InSquad( ) )
		return;

	// send off data
	CINSSquad *pSquad = pPlayer->GetSquad( );

	if( !pSquad )
	{
		AssertMsg( false, "CRunningMode::PlayerSpawn, Player has Invalid Squad" );
		return;
	}

	pSquad->UpdateOrders( pPlayer );

	// find player class
	CPlayerClass *pPlayerClass = pPlayer->GetClass( );
	Assert( pPlayerClass );
	
	if( !pPlayerClass )
	{
		AssertMsg( false, "CRunningMode::PlayerSpawn, Player Spawned with an Invalid Class" );
		return;
	}

	// disable autoswitch
	pPlayer->DisableAutoSwitch( true );

	// send all the valid items for copying
	const CPlayerInventoryBlueprint &Blueprint = pPlayerClass->GetBlueprint( );
	Blueprint.Write( pPlayer );

	// add the melee weapon
	CTeamLookup *pTeamLookup = pPlayer->GetTeamLookup( );
	
	if( pTeamLookup )
		pPlayer->AddWeapon( pTeamLookup->GetDefaultWeapon( ), 0, 0 );

	// load if it's a second wave
	CPlayTeam *pTeam = pPlayer->GetPlayTeam( );

	if( pTeam && pTeam->HadDeployment( ) )
	{
		CWeaponINSBase *pPrimaryWeapon = ToINSWeapon( pPlayer->GetPrimaryWeapon( ) );

		if( pPrimaryWeapon )
			pPrimaryWeapon->ForceReady( );
	}

	// always load the secondary weapons
	CWeaponINSBase *pSecondaryWeapon = ToINSWeapon( pPlayer->GetSecondaryWeapon( ) );

	if( pSecondaryWeapon )
		pSecondaryWeapon->ForceReady( );

	// tell the gamerules we're adding weapons
	INSRules( )->PlayerAddWeapons( pPlayer );

	// switch to best weapon
	pPlayer->SwitchToNextBestWeapon( NULL );

	// renable autoswitch
	pPlayer->DisableAutoSwitch( false );
}

//=========================================================
//=========================================================
int CRunningMode::GetDeadCamMode( void ) const
{
	return IsWaitingForPlayers( ) ? OBS_ALLOWMODE_ALL : m_iDeadCamModes;
}

//=========================================================
//=========================================================
int CRunningMode::GetDeadCamTargets( void ) const
{
	return IsWaitingForPlayers( ) ? OBS_ALLOWTARGETS_ALL : m_iDeadCamTargets;
}

//=========================================================
//=========================================================
void CRunningMode::SetCurrentSpawns( int iT1Spawn, int iT2Spawn )
{
	m_iT1CurrentSpawn = iT1Spawn;
	m_iT2CurrentSpawn = iT2Spawn;
}

//=========================================================
//=========================================================
void CRunningMode::GetCurrentSpawns( int &iT1Spawn, int &iT2Spawn )
{
	iT1Spawn = m_iT1CurrentSpawn;
	iT2Spawn = m_iT2CurrentSpawn;
}

//=========================================================
//=========================================================
int CRunningMode::ExecuteRoundReset( void )
{
	Assert( m_iRestartType != RESET_NONE && GetStatus( GAMERUNNING_RESTARTING ) );

	int iRestartType = m_iRestartType;

	bool bNonRespawnReset = ( iRestartType != RESET_RESPAWN );
	bool bGameReset = ( iRestartType == RESET_GAME );

	if (bGameReset)
		INSRules()->m_bAwardRoundPoints=false;

	// rotate the map when one team exceed winlimit
	int iWinLimit = winlimit.GetInt( );

	if( iWinLimit != 0 )
	{
		for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		{
			if( GetGlobalPlayTeam( i )->GetScore( ) >= iWinLimit )
				return RESETCODE_CHANGELEVEL;
		}
	}

	// rotate the map when max-rounds has been passed
	int iRoundLimit = roundlimit.GetInt( );

	if( iRoundLimit != 0 && INSRules( )->GetTotalTeamScore( ) >= iRoundLimit )
		return RESETCODE_CHANGELEVEL;

	// check for a complete reset
	bool bChooseSquads = false;

	if( bNonRespawnReset )
	{
		// change to squad choice when need
		// ... we've had a game reset and the the last wasn't squad
		// ... if we always want to use squad selection
		int iSquadSelectionType = INSRules( )->GetSquadOrderType( );

		if( bGameReset || iSquadSelectionType == SQUADORDER_ALWAYS )
		{
			bChooseSquads = true;
		}
		else
		{
			// update vars
			m_bFirstRestart = bGameReset;

			if( bGameReset )
			{
				INSRules( )->UpdateClanMode( );

				if( INSRules( )->IsClanMode( ) )
					m_bReadyWaiting = true;
			}
		}
	}

	// send off message
	IGameEvent *pEvent = gameeventmanager->CreateEvent( "round_reset", true );
	
	if( pEvent )
		gameeventmanager->FireEvent( pEvent );

	// tell the gamerules
	INSRules( )->RoundReset( );

	// reset scores
	if( bGameReset )
	{
		for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
			GetGlobalPlayTeam( i )->ResetScores( );
	}

	// exit now if doing a game restart (will be respawned fully when player enters etc)
	if( bChooseSquads )
		return RESETCODE_SQUADORDER;

	// set the spectators as roaming and looking in the viewpoint
	CTeam *pSpectatorTeam = GetGlobalTeam( TEAM_SPECTATOR );

	for( int i = 0; i < pSpectatorTeam->GetNumPlayers( ); i++ )
	{
		CINSPlayer *pPlayer = pSpectatorTeam->GetPlayer( i );

		if( pPlayer )
		{
			pPlayer->SetObserverMode( OBS_MODE_ROAMING );

			pPlayer->SetAbsOrigin( INSRules( )->GetCurrentViewpointOrigin( ) );
			pPlayer->SnapEyeAngles( INSRules( )->GetCurrentViewpointAngle( ) );
		}
	}

	// unmask stats
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		CTeam *pTeam = GetGlobalTeam( i );

		for( int j = 0; j < pTeam->GetNumPlayers( ); j++ )
		{
			CINSPlayer *pPlayer = pTeam->GetPlayer( j );

			if( pPlayer )
				pPlayer->UnmaskStats( );
		}
	}

	// reset current objs
	INSRules( )->ResetSpawnPoints( );
	
	// handle warmup
	/*bool bWarmingup = false;

	if( ( m_iWarmupType != WARMUP_NONE || m_bReadyWaiting ) )
	{
		// only allow a warmup when we are waiting for 
		// ... clans to be ready
		// ... when start at timed at fullround - when its the first round
		// ... when its a timer, only do it when we weren't warming up before
		bWarmingup = ( m_bReadyWaiting || 
			( ( m_iWarmupType != WARMUP_TIMED_START && m_iWarmupType != WARMUP_FULLROUND_START ) && m_bFirstRestart ) ||
			( m_iWarmupType == WARMUP_TIMED && !IsWarmingup( ) ) );

		if( bWarmingup )
		{
			// TODO: tell the player?

			AddStatus( GAMERUNNING_WARMINGUP );

			m_flWarmupStartTime = gpGlobals->curtime;
		}
		else
		{
			RemoveStatus( GAMERUNNING_WARMINGUP );
		}
	}*/

	// set detonations allowed
	m_bDetonationsAllowed = true;

	// update round info
	m_flRoundStartTime = gpGlobals->curtime;
	m_bRoundExtended = false;

	// handle teams
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		GetGlobalPlayTeam( i )->RoundReset( );

	// only freeze the player when not a warmup
	// and the game and clans are ready
	if( iRestartType == RESET_RESPAWN && !IsWaitingForPlayers( ) )
		SetFrozenState( true );

	// Shift commanders
	for( int i = TEAM_ONE; i <= TEAM_TWO; ++i )
	{
		for(int j = 0; j < MAX_SQUADS; ++j )
		{
			CINSSquad* squad= GetGlobalPlayTeam( i )->GetSquad( j );
			squad->SelectNewCommander();
		}
	}

	// setup misc
	SetupRagdollFadeTime( );

	// update
	INSRules( )->NetworkStateChanged( );

	return RESETCODE_NONE;
}

//=========================================================
//=========================================================
int CRunningMode::CheckRoundReset( void )
{
	if( !IsRestarting( ) )
		return GRMODE_NONE;

	if( gpGlobals->curtime < m_flRestartTimeMarker )
		return GRMODE_NONE;

	// allow game end for a moment
	m_bAllowEndGame = true;

	// check if the map should rotate first
	int iReturnCode = GRMODE_NONE;

	if( INSRules( )->ShouldMapRotate( ) )
	{
		return GRMODE_NONE;
	}
	else
	{
		if( !IsWarmingup( ) )
			HideEndDialog( );

		switch( ExecuteRoundReset( ) )
		{
			case RESETCODE_SQUADORDER:
				iReturnCode = GRMODE_SQUAD;
				break;

			case RESETCODE_CHANGELEVEL:
				m_bWantEndGame = true;
				break;
		}

		m_bAllowEndGame = m_bWantEndGame;
	}

	// reset
	m_iRestartType = RESET_NONE;
	m_flRestartTimeMarker = 0.0f;

	RemoveStatus( GAMERUNNING_RESTARTING | GAMERUNNING_ENDGAME );

	return iReturnCode;
}

//=========================================================
//=========================================================
bool CRunningMode::RequestReset( int iType, float flResetTime, bool bRequested )
{
	if( flResetTime < 0.0f )
		return false;

	if( bRequested && IsRestarting( ) )
		return false;

	AddStatus( GAMERUNNING_RESTARTING );

	m_iRestartType = iType;
	m_flRestartTimeMarker = gpGlobals->curtime + flResetTime;

	if( bRequested )
		EchoGameReset( NULL, RoundFloatToInt( flResetTime ) );

	return true;
}

//=========================================================
//=========================================================
void CRunningMode::EchoGameReset( CINSPlayer *pPlayer, int iTime )
{
	char szResetText[ 128 ];
	Q_snprintf( szResetText, sizeof( szResetText ), "Game will Reset in %i Seconds", iTime );

	if( pPlayer )
		ClientPrint( pPlayer, HUD_PRINTCENTER, szResetText );
	else
		UTIL_ClientPrintAll( HUD_PRINTCENTER, szResetText );
}

//=========================================================
//=========================================================
void CRunningMode::RepeatGameReset( CINSPlayer *pPlayer )
{
	int iTimeLeft = RoundFloatToInt( gpGlobals->curtime - m_flRestartTimeMarker );

	if( ( gpGlobals->curtime - iTimeLeft ) > 3 )
		EchoGameReset( pPlayer, iTimeLeft );
}


//=========================================================
//=========================================================
bool CRunningMode::CanWinRound( void )
{
	return ( !IsRestarting( ) && !IsEnding( ) );
}

//=========================================================
//=========================================================
void CRunningMode::RoundWon( int iTeamID, int iWinType, CBaseEntity *pWinningObject, bool bVerbose )
{
	if( !IsPlayTeam( iTeamID ) )
	{
		if( bVerbose )
			Msg( "round win failed - invalid team\n" );

		return;
	}

	if( !IsValidWinType( iWinType ) )
	{
		if( bVerbose )
			Msg( "round win failed - invalid wintype\n" );

		return;
	}

	if( !CanWinRound( ) )
	{
		if( bVerbose )
			Msg( "round win failed - cannot win at this time\n" );

		return;
	}

	// send off event
	IGameEvent *pEvent = gameeventmanager->CreateEvent( "round_end", true );

	if( pEvent )
	{
		pEvent->SetInt( "winner", iTeamID );
		gameeventmanager->FireEvent( pEvent );
	}

	// update the score
	CPlayTeam *pTeam = NULL;

	if( iTeamID != TEAM_NEUTRAL )
	{
		pTeam = GetGlobalPlayTeam( iTeamID );

		if( IsScoringAllowed( ) )
			pTeam->IncrementScore( );
	}

	// setup the ending game
	AddStatus( GAMERUNNING_ENDGAME );

	INSRules()->m_bAwardRoundPoints=true;

	// setup player scores
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		CTeam *pTeam = GetGlobalTeam( i );

		for( int j = 0; j < pTeam->GetNumPlayers( ); j++ )
		{
			CINSPlayer *pPlayer = pTeam->GetPlayer( j );

			if( !pPlayer )
				continue;

			pPlayer->IncrementStat( PLAYERSTATS_GAMEPTS, MORALE_VICTORY );
			pPlayer->SendStatNotice( MORALE_VICTORY, "Victory" );

			// give the winning players big cheesy smiles
			/*if( iTeamID == i )
			{
				int iSmileID = pPlayer->FindFlexController( "smile" );

				if( iSmileID != -1 )
					pPlayer->SetFlexWeight( iSmileID, 1.0f );
			}*/

			// now take the dead players and point them at the winning obj/player
			if( pWinningObject && pPlayer->IsObserver( ) )
				pPlayer->SetupEndgameCamera( pWinningObject );

			// unmask all stats
			pPlayer->UnmaskStats( );
		}
	}

	// update player data right away
	g_pPlayerResource->UpdatePlayerData();

	// point the spectators at the winning object
	// NOTE: this doesn't work
	/*CTeam *pSpectatorTeam = GetGlobalTeam(TEAM_SPECTATOR);

	for(int i = 0; i < pSpectatorTeam->GetNumPlayers(); i++)
	{
		CBasePlayer *pPlayer = pTeam->GetPlayer(i);

		if(!pPlayer)
			continue;

		pPlayer->SetupEndgameCamera(pWinningObject);
	}*/

	// caculate end game values
	if( !IsWarmingup( ) && IsScoringAllowed( ) )
		CalculateEndValues( iTeamID, iWinType );

	// either hang around a bit or end immediately
	if( iWinType == ENDGAME_WINCONDITION_OBJ || iWinType == ENDGAME_WINCONDITION_DEATH )
	{
		Assert( pTeam );

		// send message
		char szWinningTeam[ 128 ];
		Q_snprintf( szWinningTeam, sizeof( szWinningTeam ), "%s Win", pTeam->GetName( ) );
		UTIL_CenterPrintAll( szWinningTeam );

		// end innabit
		m_flEndGameTime = gpGlobals->curtime + endgametime.GetFloat( );
	}
	else
	{
		EndGameFinished( );
	}

	// update the capturing objectives
	INSRules( )->ObjectiveCapturingUpdate( );
}

//=========================================================
//=========================================================
#define ENDGAME_TIME 5.0f

void CRunningMode::EndGameFinished( void )
{
	// clean the grenades early
	m_bDetonationsAllowed = false;

	// now, show the end-dialogs
	ShowEndDialog( );

	// reset the round innabit
	float flResetTime = ENDGAME_TIME;

	if( IsScoringAllowed( ) )
		flResetTime += chattime.GetFloat( );

	// reset round
	RequestReset( RESET_ROUND, flResetTime, false );

	// reset timer
	m_flEndGameTime = 0.0f;
}

//=========================================================
//=========================================================
bool CRunningMode::IsValidWinType( int iWinType )
{
	return ( iWinType >= 0 && iWinType < ENDGAME_WINCONDITION_EXTENDED );
}

//=========================================================
//=========================================================
void CRunningMode::CheckDeathRoundWin( CPlayTeam *pTeam, CBasePlayer *pAttacker )
{
	if( pTeam->AreAllPlayersDead( ) && !pTeam->HasReinforcementLeft( ) )
		RoundWon( FlipPlayTeam( pTeam->GetTeamID( ) ), ENDGAME_WINCONDITION_DEATH, pAttacker );
}

//=========================================================
//=========================================================
void CRunningMode::AddStatus( int iStatus )
{
	m_iStatus |= iStatus;

	INSRules( )->NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CRunningMode::RemoveStatus(int iStatus)
{
	m_iStatus &= ~iStatus;

	INSRules( )->NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CRunningMode::RoundExtended( void )
{
	UTIL_ClientPrintAll( HUD_PRINTCENTER, "Round has Entered Sudden Death" );

	m_bRoundExtended = true;

	INSRules( )->NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CRunningMode::CalculateEndValues( int iWinningTeam, int iWinType )
{
	m_pEndGameValues = new KeyValues( "data" );

	// set basic data
	m_pEndGameValues->SetInt( "team", iWinningTeam );
	m_pEndGameValues->SetInt( "type", iWinType );

	// only calculate extra stuff when a playteam wins
	if( iWinningTeam == TEAM_NEUTRAL )
		return;

	// work out teams
	CPlayTeam *pWinningTeam = GetGlobalPlayTeam( iWinningTeam );
	CPlayTeam *pLosingTeam = GetGlobalPlayTeam( FlipPlayTeam( iWinningTeam ) );

	// calculate the length of the game
	int iLength = ENDGAME_ROUNDLENGTH_MEDIUM;

	if( !IsRoundExtended( ) )
	{
		if( INSRules( )->ForceTimerType( ) )
		{
			float flTotalRoundLength = gpGlobals->curtime - m_flRoundStartTime;

			if( flTotalRoundLength <= ( m_iRoundLength / 4.0f ) )
				iLength = ENDGAME_ROUNDLENGTH_QUICK;
			else if( flTotalRoundLength <= ( m_iRoundLength / 2.5f ) )
				iLength = ENDGAME_ROUNDLENGTH_MEDIUM;
			else
				iLength = ENDGAME_ROUNDLENGTH_LONG;
		}
		else
		{
			int iCapturableObjectives = 0;

			for( int i = 0; i < INSRules( )->GetObjectiveCount( ); i++ )
			{
				CINSObjective *pObjective = INSRules( )->GetObjective( i );

				if( pObjective && pObjective->IsCapturable( ) )
					iCapturableObjectives++;
			}

			int iObjectiveCaptures = pWinningTeam->GetObjectiveCaptures( );

			if( iObjectiveCaptures <= iCapturableObjectives )
			{
				iLength = ENDGAME_ROUNDLENGTH_QUICK;
			}
			else
			{
				float flCaptureFraction = ( float )iObjectiveCaptures / ( float )iCapturableObjectives;

				if( flCaptureFraction <= 0.5f )
					iLength = ENDGAME_ROUNDLENGTH_LONG;
				else if( flCaptureFraction <= 0.75f )
					iLength = ENDGAME_ROUNDLENGTH_MEDIUM;
				else
					iLength = ENDGAME_ROUNDLENGTH_QUICK;
			}
		}
	}
	else
	{
		iLength = ENDGAME_ROUNDLENGTH_LONG;
	}

	// set the length of the game
	m_pEndGameValues->SetInt( "length", iLength );

	// calculate the balance
	int iCurrentBalance = ENDGAME_BALANCE_BALANCED;
	int iBalancePoolSize = 0;

	// .. reinforcements used
	if( !pWinningTeam->IsUnlimitedWaves( ) && !pLosingTeam->IsUnlimitedWaves( ) )
	{
		int iWinningWavesLeft, iLosingWavesLeft;
		iWinningWavesLeft = pWinningTeam->AverageReinforcementsLeft( );
		iLosingWavesLeft = pLosingTeam->AverageReinforcementsLeft( );
	
		if( iWinningWavesLeft > 0 && iWinningWavesLeft > iLosingWavesLeft )
		{
			float flWaveFraction = ( float )iLosingWavesLeft / ( float )iWinningWavesLeft;

			if( flWaveFraction <= 0.5f )
			{
				if( flWaveFraction >= 0.35f )
					iCurrentBalance += ENDGAME_BALANCE_SUPERIOR;
				else
					iCurrentBalance += ENDGAME_BALANCE_OWNED;
			}

			iBalancePoolSize++;
		}
	}

	// .. objective captures
	int iWinningTeamNumObjectives, iLosingTeamNumObjectives;
	iWinningTeamNumObjectives = iLosingTeamNumObjectives = 0;

	for( int i = 0; i < INSRules( )->GetObjectiveCount( ); i++ )
	{
		CINSObjective *pObjective = INSRules( )->GetObjective( i );

		int iCapturedTeam = pObjective->GetCapturedTeam( );

		if( pObjective->IsCapturable( ) || iCapturedTeam == TEAM_NEUTRAL )
			continue;

		int &iTeamNumObjectives = ( (iCapturedTeam == TEAM_ONE ) ? iWinningTeamNumObjectives : iLosingTeamNumObjectives );
		iTeamNumObjectives++;
	}

	if( iWinningTeamNumObjectives > iLosingTeamNumObjectives )
	{
		float flObjectiveFraction = ( float )iLosingTeamNumObjectives / ( float )iWinningTeamNumObjectives;

		if( flObjectiveFraction <= 0.5f )
		{
			if( flObjectiveFraction >= 0.35f )
				iCurrentBalance += ENDGAME_BALANCE_SUPERIOR;
			else
				iCurrentBalance += ENDGAME_BALANCE_OWNED;

			iBalancePoolSize++;
		}
	}

	// ... round length
	if( m_iRoundTimerType != ROUNDTIMER_NONE )
	{
		if( iLength == ENDGAME_ROUNDLENGTH_MEDIUM )
			iCurrentBalance += ENDGAME_BALANCE_SUPERIOR;
		else if( iLength == ENDGAME_ROUNDLENGTH_QUICK )
			iCurrentBalance += ENDGAME_BALANCE_OWNED;

		iBalancePoolSize++;
	}

	if( iBalancePoolSize > 0 )
		iCurrentBalance /= iBalancePoolSize;

	// set balance of the game
	m_pEndGameValues->SetInt( "balance", iCurrentBalance );
}

//=========================================================
//=========================================================
#define FADE_BLACK_TIME 3.0f

void CRunningMode::ShowEndDialog(void)
{
	for( int i = TEAM_ONE; i <= TEAM_SPECTATOR; i++ )
	{
		CTeam *pTeam = GetGlobalTeam( i );

		for( int j = 0; j < pTeam->GetNumPlayers( ); j++ )
		{
			CINSPlayer *pPlayer = pTeam->GetPlayer( j );

			// fade to black
			pPlayer->SetMoveType( MOVETYPE_NONE );
			pPlayer->EnableControl( false );

			//pPlayer->FadeToBlack( FADE_BLACK_TIME );

			pPlayer->ShowViewPortPanel( PANEL_SCOREBOARD, true, m_pEndGameValues );

			// reset buttons
			pPlayer->m_nButtons = 0;
		}
	}
}

//=========================================================
//=========================================================
void CRunningMode::HideEndDialog(void)
{
	if( m_pEndGameValues )
	{
		m_pEndGameValues->deleteThis( );
		m_pEndGameValues = NULL;
	}

	for( int i = TEAM_ONE; i <= TEAM_SPECTATOR; i++ )
	{
		CTeam *pTeam = GetGlobalTeam( i );

		for( int j = 0; j < pTeam->GetNumPlayers(); j++ )
		{
			CBasePlayer	*pPlayer = pTeam->GetPlayer( j );
			pPlayer->ShowViewPortPanel( PANEL_SCOREBOARD, false );
		}
	}
}

//=========================================================
//=========================================================
#define RESET_CLAN_TIME 5

void CRunningMode::UpdateClanStatus( CPlayTeam *pTeam )
{
	if( !m_bReadyWaiting )
		return;

	// update team
	pTeam->SetClanReady( true );

	// ensure that both teams are ready for reset
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		CPlayTeam *pTeam = GetGlobalPlayTeam( i );

		if( !pTeam || !pTeam->IsClanReady( ) )
			return;
	}

	// no longer waiting
	m_bReadyWaiting = false;

	// reset
	RequestReset( RESET_ROUND, RESET_CLAN_TIME, false );

	char szResetText[ 128 ];
	Q_snprintf( szResetText, sizeof( szResetText ), "Game is Live in %i Seconds", RESET_CLAN_TIME );
	UTIL_ClientPrintAll( HUD_PRINTCENTER, szResetText );
}

//=========================================================
//=========================================================
#define COMMENCING_TIME 2.5

void CRunningMode::HandlePlayerCount( void )
{
	if( INSRules( )->ShouldBeWaitingForPlayers( ) )
	{
		if( !IsWaitingForPlayers( ) )
			AddStatus( GAMERUNNING_PLAYERWAIT );

		if( !IsRestarting( ) && !IsWarmingup( ) )
			UTIL_ClientPrintAll( HUD_PRINTCENTER, "Scoring will not Begin until Both Teams have Players" );

	}
	else if( IsWaitingForPlayers( ) && !IsRestarting( ) )
	{
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "Game Commencing!" );
		RequestReset( RESET_GAME, COMMENCING_TIME, false );
	}
}

//=========================================================
//=========================================================
void CRunningMode::UpdateConsoleValues( void )
{
	m_bObjectivesEnabled = !objdisable.GetBool( );

	// but check the gamerules to see if we want to force any values
	int iTimerType = timertype.GetInt( );

	if( INSRules( )->ForceTimerType( ) && iTimerType == ROUNDTIMER_NONE )
		iTimerType = ROUNDTIMER_IMC;

	m_iRoundTimerType = iTimerType;

	// update timer length
	switch( iTimerType )
	{
		case ROUNDTIMER_NONE:
			m_iRoundLength = ROUNDTIMER_INVALID;
			break;

		case ROUNDTIMER_VALUE:
			m_iRoundLength = roundtimer.GetInt( );
			break;

		case ROUNDTIMER_IMC:
			m_iRoundLength = IMCConfig( )->GetRoundLength( );
			break;
	}

	m_iDeadCamModes = deadcam_modes.GetInt( );
	m_iDeadCamTargets = deadcam_targets.GetInt( );
	m_iFreezeTimeLength = freezetime.GetInt( );
	m_bMaskOtherTeam = maskotherteam.GetBool( );
	m_bScoreFrozen = scorefreeze.GetBool( );
	m_bDeathInfoFull = deadinfofull.GetBool( );

	INSRules( )->NetworkStateChanged( );
}

//=========================================================
//=========================================================
bool CRunningMode::ObjectivesEnabled( void ) const
{
	return m_bObjectivesEnabled;
}

//=========================================================
//=========================================================
void CRunningMode::PlayerJoinedPlayTeam( CINSPlayer *pPlayer, bool bSlain )
{
	// if the game is running, open up the squad change menu or repeat how long its
	// going to take until the game starts
	if( IsRestarting( ) && 
		( m_iRestartType == RESET_GAME || ( m_iRestartType == RESET_ROUND && INSRules( )->GetSquadOrderType( ) == SQUADORDER_ALWAYS ) ) )
	{
		RepeatGameReset( pPlayer );
	}
	else
	{
		if( bSlain )
			pPlayer->ChangedTeam( );
		else
			pPlayer->ShowViewPortPanel( PANEL_SQUADSELECT, true );
	}
}

//=========================================================
//=========================================================
void CRunningMode::PlayerJoinedSquad( CINSPlayer *pPlayer, bool bFirstSquadChange )
{
	// never spawn right away, start reinforcements etc
	if( !IsRestarting( ) )
	{
		// respawn when in a viewpoint
		if( pPlayer->InViewpoint( ) )
			pPlayer->Spawn( );

		// start reinforcements when they haven't been started
		CPlayTeam *pTeam = pPlayer->GetPlayTeam( );

		if( pTeam )
			pTeam->StartReinforcement( pPlayer->GetSquadID( ) );

		// send them a message when not in a viewpoint
		if( pPlayer->IsObserver( ) )
			ClientPrint( pPlayer, HUD_PRINTTALK, "You will deploy as your new class" );
	}

	// mark them as having changed
	pPlayer->ChangedSquad( );
}

//=========================================================
//=========================================================
void CRunningMode::SetFrozenState( bool bState )
{
	// update state etc
	if( bState )
	{
		AddStatus( GAMERUNNING_FREEZE );

		m_flFreezeTime = gpGlobals->curtime + m_iFreezeTimeLength;

		m_flDefaultOrderRollout = gpGlobals->curtime + ( m_iFreezeTimeLength * DEFAULTORDERS_FROZENFRAC );
	}
	else
	{
		RemoveStatus( GAMERUNNING_FREEZE );

		m_flFreezeTime = 0.0f;

		// tell everyone about it
		INSRules( )->RoundUnfrozen( );

		for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		{
			CPlayTeam *pTeam = GetGlobalPlayTeam( i );

			if( pTeam )
				pTeam->RoundUnfrozen( );
		}
	}
}

//=========================================================
//=========================================================
void CRunningMode::SetupRagdollFadeTime( void )
{
	int iTotalTimeWave = 0;

	// find total timewave
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		iTotalTimeWave += GetGlobalPlayTeam( i )->GetIMCTeamConfig( )->GetTimeWave( );

	// calculate fadetime and clamp value
	float flMinFadeTime, flMaxFadeTime, flFadeTime;
	flMinFadeTime = ragdollmin.GetFloat( );
	flMaxFadeTime = max( flMinFadeTime, ragdollmax.GetFloat( ) );
	flFadeTime = iTotalTimeWave * 0.5f;

	m_flRagdollFadeTime = clamp( flFadeTime, flMinFadeTime, flMaxFadeTime );
}

//=========================================================
//=========================================================
void CRunningMode::RagdollSpawn( CINSRagdoll *pRagdoll )
{
	if( ragdollkeep.GetBool( ) )
		return;

	m_RagdollFadeList.AddToTail( pRagdoll );
}

//=========================================================
//=========================================================
void CRunningMode::EntitiesCleaned( void )
{
	m_RagdollFadeList.Purge( );
}

//=========================================================
//=========================================================
bool CRunningMode::AllowGameEnd( void ) const
{
	return m_bAllowEndGame;
}

//=========================================================
//=========================================================
bool CRunningMode::ShouldGameEnd( void ) const
{
	return m_bWantEndGame;
}
