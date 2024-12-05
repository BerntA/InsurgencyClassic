//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_gamerules.h"

#include "play_team_shared.h"

#include <cl_dll/iviewport.h>
#include "endgame.h"
#include "hud_macros.h"

#include "script_check_shared.h"

#include "c_basetempentity.h"
#include "c_te_legacytempents.h"

#include "pain_helper.h"

#include "clientsideeffects.h"
#include "insvgui.h"

#include "imc_config.h"
#include "clipdef.h"
#include "ammodef.h"
#include "weapondef.h"

#include "voicemgr.h"

#include "igameresources.h"
#include "play_team_shared.h"
#include "ins_squad_shared.h"

#include "basic_colors.h"

#include "gameuipanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
char *g_pszGameDescriptions[ MAX_GAMETYPES ] = { 
	{ "Battle requires the player to capture the objectives in an order." },	// GAMETYPE_BATTLE
	{ "Objectives can be captured in any order." },								// GAMETYPE_FIREFIGHT
	{ "One team attacks while the other defends." },							// GAMETYPE_PUSH
	{ "One team strikes another objectives while the other defends." },			// GAMETYPE_STRIKE
	{ "Team Death Match." },														// GAMETYPE_TDM
	{ "Two Goals, One Ball." }													// GAMETYPE_POWERBALL
};

//=========================================================
//=========================================================
ConVar localcolors( "cl_localcolors", "0", FCVAR_ARCHIVE, "Defines whether to use Local Team Colors" );

Color g_DefaultTeamColor = COLOR_GREY;
Color g_FriendlyTeamColor = COLOR_BLUE;
Color g_EnemyTeamColor = COLOR_RED;

//=========================================================
//=========================================================
C_INSRules::C_INSRules( )
{
	// init crappy vars
	m_vecCurrentViewpoint.Init( );
	m_angCurrentViewpoint.Init( );

	// calculate shared scripts etc
	g_ScriptCheckShared.Calculate( );
	m_iScriptsCRC32 = g_ScriptCheckShared.GetScriptCRC32( );

	// init game status
	m_iCurrentMode = GRMODE_IDLE;

	for( int i = 0; i < GRMODE_COUNT; i++ )
	{
		ModeHelper_t ModeHelper = g_ModeHelper[ i ];

		if( !ModeHelper )
		{
			AssertMsg( false, "Missing Mode Class" );
			continue;
		}

		m_pModes[ i ] = ModeHelper( );
	}

	// reset correction
	m_ColorCorrection = 0;

	// add listeners
	gameeventmanager->AddListener( this, "round_reset", false );
}

//=========================================================
//=========================================================
C_INSRules::~C_INSRules( )
{
	gameeventmanager->RemoveListener( this );
}

//=========================================================
//=========================================================
void C_INSRules::LevelInitPreEntity( void )
{
	BaseClass::LevelInitPreEntity( );

	// just in case the UI doesn't init IMC
	CIMCConfig *pIMCConfig = CIMCConfig::IMCConfig( );
	
	if( !pIMCConfig->IsInit( ) )
		pIMCConfig->Setup( );

	// init objectives
	C_INSObjective::LevelInit( );

	// init weapon def
	GetWeaponDef( );

	// init player
	C_INSPlayer::LoadData( );

	// ensure join panel is shown
	GameUIPanel( )->WaitingJoinGame( );

	// init color correction
	const char *pszColorCorrectionFilename = IMCConfig( )->GetColorCorrection( );

	if( pszColorCorrectionFilename && *pszColorCorrectionFilename )
	{
		char szColorCorrection[ MAX_COLORCORRECTION_LENGTH ];
		Q_snprintf( szColorCorrection, sizeof( szColorCorrection ), "materials/correction/maps/%s.raw", pszColorCorrectionFilename );

		m_ColorCorrection = colorcorrection->AddLookup( szColorCorrection );

		colorcorrection->LockLookup( m_ColorCorrection );
		colorcorrection->LoadLookup( m_ColorCorrection, szColorCorrection );
		colorcorrection->UnlockLookup( m_ColorCorrection );
	}
}

//=========================================================
//=========================================================
void C_INSRules::LevelShutdownPreEntity( void )
{
	IMCConfig( )->Reset( );

	m_ColorCorrection = 0;
}

//=========================================================
//=========================================================
void C_INSRules::Precache( void )
{
	g_VoiceMgr.Precache( );
}

//=========================================================
//=========================================================
bool C_INSRules::ObjectiveCaptureAllowed( const C_INSObjective *pObjective, int iTeamID )
{
	return true;
}

//=========================================================
//=========================================================
void C_INSRules::SetMode( int iMode )
{
	m_iCurrentMode = iMode;

	CurrentMode( )->Init( );
}

//=========================================================
//=========================================================
C_RunningMode *CINSRules::RunningModeUpdate( void ) const
{
	return ( C_RunningMode* )m_pModes[ GRMODE_RUNNING ];
}

//=========================================================
//=========================================================
C_SquadMode *CINSRules::SquadModeUpdate( void ) const
{
	return ( C_SquadMode* )m_pModes[ GRMODE_SQUAD ];
}

//=========================================================
//=========================================================
void C_INSRules::RoundReset( void )
{
	// remove the decals
	engine->ClientCmd( "r_cleardecals\n" );

	// clear out temp entities
	C_BaseTempEntity::CheckDynamicTempEnts( );

	// clear out temp entities
	tempents->Init( );
}

//=========================================================
//=========================================================
void C_INSRules::FireGameEvent( IGameEvent *pEvent )
{
	const char *pszName = pEvent->GetName( );

	if( Q_strcmp( pszName, "round_reset" ) == 0 )
	{
		RoundReset( );
	}
}

//=========================================================
//=========================================================
void C_INSRules::SetCurrentViewpointOrigin( float flX, float flY, float flZ )
{
	m_vecCurrentViewpoint[ 0 ] = flX;
	m_vecCurrentViewpoint[ 1 ] = flY;
	m_vecCurrentViewpoint[ 2 ] = flZ;
}

//=========================================================
//=========================================================
void C_INSRules::SetCurrentViewpointAngles( float flX, float flY, float flZ )
{
	m_angCurrentViewpoint[ 0 ] = flX;
	m_angCurrentViewpoint[ 1 ] = flY;
	m_angCurrentViewpoint[ 2 ] = flZ;
}

//=========================================================
//=========================================================
const Color &C_INSRules::TeamColor( C_Team *pTeam ) const
{
	Assert( pTeam );
	return TeamColor( pTeam->GetTeamID( ) );
}

//=========================================================
//=========================================================
const Color &C_INSRules::TeamColor( C_INSPlayer *pPlayer ) const
{
	Assert( pPlayer );
	return TeamColor( pPlayer->GetTeamID( ) );
}

//=========================================================
//=========================================================
const Color &C_INSRules::TeamColor( int iTeamID ) const
{
	if( !IsPlayTeam( iTeamID ) )
		return g_DefaultTeamColor;

	if( localcolors.GetBool( ) )
	{
		CINSPlayer *pLocalPlayer = CINSPlayer::GetLocalPlayer( );

		if( !pLocalPlayer )
			return g_DefaultTeamColor;
	}

	CPlayTeam *pTeam = GetGlobalPlayTeam( iTeamID );
	return ( pTeam ? pTeam->GetColor( ) : g_DefaultTeamColor );
}

//=========================================================
//=========================================================
void C_INSRules::SetScriptsCRC32( unsigned int iCRC32 )
{
	if( m_iScriptsCRC32 == 0 )
		return;

	char szInvalidScripts[ 64 ];
	Q_snprintf( szInvalidScripts, sizeof( szInvalidScripts ), "%s %u", GRCMD_INVALIDSCRIPTS, iCRC32);

	if( iCRC32 != m_iScriptsCRC32 )
		engine->ServerCmd( szInvalidScripts, true );
}

//=========================================================
//=========================================================
void C_SquadMode::Init( void )
{
	// hide the end-game panel (saves a message)
	IViewPortPanel *pEndGame = gViewPortInterface->FindPanelByName( PANEL_ENDGAME );

	if( pEndGame )
		pEndGame->ShowPanel( false );
}

//=========================================================
//=========================================================
bool C_SquadMode::IsValidData( int iTeamID ) const
{
	return ( GetLocalTeamID( ) == iTeamID );
}

//=========================================================
//=========================================================
void C_SquadMode::SetOrderLength( int iTeamID, int iSize )
{
	if( IsValidData( iTeamID ) && m_PlayerOrder.Size( ) != iSize )
		m_PlayerOrder.SetSize( iSize );
}

//=========================================================
//=========================================================
void C_SquadMode::SetOrder( int iTeamID, int iElement, int iValue )
{
	if( IsValidData( iTeamID ) )
		m_PlayerOrder[ iElement ] = iValue;
}

//=========================================================
//=========================================================
int C_SquadMode::GetPlayerCount( void ) const
{
	return m_PlayerOrder.Count( );
}

//=========================================================
//=========================================================
int C_SquadMode::GetPlayer( int iPlayerID ) const
{
	C_PlayTeam *pTeam = GetLocalPlayTeam( );
	return ( pTeam ? pTeam->GetPlayerID( m_PlayerOrder[ iPlayerID ] ) : 0 );
}

//=========================================================
//=========================================================
void C_RunningMode::SetStatus( int iStatus )
{
	m_iStatus = iStatus;
}

//=========================================================
//=========================================================
void C_RunningMode::SetRoundStartTime( float flRoundStartTime )
{
	m_flRoundStartTime = flRoundStartTime;
}

//=========================================================
//=========================================================
void C_RunningMode::SetRoundLength( int iRoundLength )
{
	m_iRoundLength = iRoundLength;
}

//=========================================================
//=========================================================
void C_RunningMode::SetRoundExtended( bool bState )
{
	m_bRoundExtended = bState;
}

//=========================================================
//=========================================================
void C_RunningMode::SetFrozenTimeLength(int iLength)
{
	m_iFrozenTimeLength = iLength;
}

//=========================================================
//=========================================================
void C_RunningMode::SetDeathInfoFull( bool bState )
{
	m_bDeathInfoFull = bState;
}

//=========================================================
//=========================================================
bool C_RunningMode::GetStatus( int iStatus ) const
{
	return ( ( m_iStatus & iStatus ) != 0 );
}

//=========================================================
//=========================================================
int C_RunningMode::GetRoundLength( void ) const
{
	return m_iRoundLength;
}

//=========================================================
//=========================================================
float C_RunningMode::GetRoundStartTime( void ) const
{
	return m_flRoundStartTime;
}

//=========================================================
//=========================================================
bool C_RunningMode::IsRoundExtended( void ) const
{
	return m_bRoundExtended;
}

//=========================================================
//=========================================================
int C_RunningMode::GetFrozenTimeLength( void ) const
{
	return m_iFrozenTimeLength;
}

//=========================================================
//=========================================================
bool C_RunningMode::IsDeathInfoFull( void ) const
{
	return m_bDeathInfoFull;
}

//=========================================================
//=========================================================
void CC_CaptureScoreboard( void )
{
	if( !INSRules( ) )
		return;

	IGameResources *gr = GameResources();

	if( !gr )
		return;

	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		C_PlayTeam *pTeam = GetGlobalPlayTeam( i );

		if( !pTeam )
			continue;

		Msg( "-----------------------------------\n" );
		Msg( "Team: %s - %i\n", pTeam->GetName( ), pTeam->GetScore( ) );
		Msg( "-----------------------------------\n" );

		for( int i = 0; i < pTeam->GetSquadCount( ); i++ )
		{
			C_INSSquad *pSquad = pTeam->GetSquad( i );

			if( !pSquad )
				continue;
		
			for( int j = 0; j < pTeam->GetSquadCount( ); j++ )
			{
				C_INSSquad *pSquad = pTeam->GetSquad( j );

				if( !pSquad )
					continue;

				Msg( "Squad: %s\n", pSquad->GetName( ) );

				Msg( "---------------------\n" );

				for( int k = 0; k < MAX_SQUAD_SLOTS; k++ )
				{
					if( pSquad->IsSlotEmpty( k ) )
						continue;

					int iPlayerID = pSquad->GetPlayerID( k );

					Msg( "%s - %i\n", gr->GetPlayerName( iPlayerID ), gr->GetPlayerScore( iPlayerID ) );
				}
			}
		}

		Msg( "\n" );
	}
}

static ConCommand capturescoreboard("capturescoreboard", CC_CaptureScoreboard, "Captures the Scoreboard");