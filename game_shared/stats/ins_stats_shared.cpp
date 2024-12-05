//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_interface.h"
#include "ins_player_shared.h"
#include "ins_stats_shared.h"
#include "statsman.h"
#include "stats_protection.h"
#include "ins_gamerules.h"
#include "viewport_panel_names.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================

// PNOTE: stats are only enabled and loaded (DLL-wise)
// when this is uncommented
//#define USE_STATS

//=========================================================
//=========================================================
#ifdef GAME_DLL

#ifdef _DEBUG

	#define STATSMAN_FILE "ins_statsmand"

#else

	#define STATSMAN_FILE "ins_statsman"

#endif

#else

#ifdef _DEBUG

	#define STATSMAN_FILE "ins_cstatsmand"

#else

	#define STATSMAN_FILE "ins_cstatsman"

#endif

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

extern ConVar globalstats;
extern ConVar stats_username;
extern ConVar stats_password;

#endif

//=========================================================
//=========================================================
CINSStats::CINSStats( )
{
#ifdef GAME_DLL

	// determine wether to use stats
	m_iType = STATSTYPE_NONE;

#endif

	m_hStatsManDLL = NULL;
	m_pStatsMan = NULL;
}

//=========================================================
//=========================================================
CINSStats::~CINSStats( )
{
	delete m_pStatsMan;

	if( m_hStatsManDLL )
		Sys_UnloadModule( m_hStatsManDLL );
}

//=========================================================
//=========================================================
CINSStats *GetINSStats( void )
{
	return CINSStats::GetINSStats( );
}

CINSStats *CINSStats::GetINSStats( void )
{
	static CINSStats *pINSStats = NULL;

	if( !pINSStats )
		pINSStats = new CINSStats( );

	return pINSStats;
}

//=========================================================
//=========================================================
void CINSStats::Init( void )
{
#ifndef USE_STATS

	return;

#endif

#ifdef GAME_DLL

	m_iType = STATSTYPE_READONLY;

	const char *pszUsername, *pszPassword;
	pszUsername = pszPassword = NULL;

#ifndef STATS_PROTECTION

	// check if username or password is empty
	pszUsername = stats_username.GetString( );
	pszPassword = stats_password.GetString( );

	if( *pszUsername == '\0' || *pszPassword == '\0' )
		return;

#endif

#endif

	LoadDLLs( );

#ifdef GAME_DLL

	// init stats
	m_pStatsMan->Init( pszUsername, pszPassword, this );

	// are we valid?
	if( !m_pStatsMan->IsValid( ) )
	{

#ifdef STATS_PROTECTION

		m_iType = STATSTYPE_BLOCKED;

#else

		m_iType = STATSTYPE_NONE;

#endif

	}

#else

	// init stats
	m_pStatsMan->Init( this );

#endif
}

//=========================================================
//=========================================================
typedef CStatsMan *( *PFNCreateStatsMan )( );
typedef void ( *PFNInitStatsMan )( char* pszSearchPath );

void CINSStats::LoadDLLs( void )
{
	char szStatsmanPath[ MAX_PATH ];

#ifdef GAME_DLL

	engine->GetGameDir( szStatsmanPath, MAX_PATH );

#else

	Q_strncpy( szStatsmanPath, engine->GetGameDirectory( ), MAX_PATH );

#endif

	Q_strncat( szStatsmanPath, "/bin/", MAX_PATH, MAX_PATH );
	char *pszEndPath = szStatsmanPath + Q_strlen( szStatsmanPath );
	Q_strncat( pszEndPath, STATSMAN_FILE, MAX_PATH, MAX_PATH );

	// create manager
	m_hStatsManDLL = Sys_LoadModule( szStatsmanPath );
	*pszEndPath = '\0';

	if( !m_hStatsManDLL )
	{
	#ifdef _LINUX

		Warning( "Sys_LoadModule: %s\n", dlerror( ) );

	#endif

		return;
	}

	PFNInitStatsMan pfnInitStats = ( PFNInitStatsMan )Sys_GetProcAddress( m_hStatsManDLL, "InitDLL" );

	if( pfnInitStats )
		pfnInitStats( szStatsmanPath );

	PFNCreateStatsMan pfnCreateStats = ( PFNCreateStatsMan )Sys_GetProcAddress( m_hStatsManDLL, "GetStatsMan" );
	Assert( pfnCreateStats );
	m_pStatsMan = ( pfnCreateStats )( );
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CINSStats::IsBlocked( CINSPlayer *pPlayer )
{
	return ( INSRules( )->GetStatsType( ) == STATSTYPE_BLOCKED && !pPlayer->IsDeveloper( ) );
}

#endif

//=========================================================
//=========================================================
bool CINSStats::IsValid( void ) const
{
	return ( m_pStatsMan != NULL );
}

//=========================================================
//=========================================================
void CINSStats::LoginPlayer( CINSPlayer *pPlayer, const char *pszUsername, const char *pszPassword )
{
	if( !IsValid( ) )
		return;

#if defined( STATS_PROTECTION ) && defined( GAME_DLL )

	// don't let players login when stats aren't being used at all
	if( m_iType == STATSTYPE_NONE )
		return;

#endif

#ifdef GAME_DLL

	if( !pPlayer )
		return;

#endif

#if defined( GAME_DLL ) && defined( _DEBUG )

	// bots login straight away
	if( pPlayer->IsBot( ) )
	{
		LoginCallback( pPlayer, SAC_PLAYERTYPE_VALID, 0 );
		return;
	}

#endif

	// client login if we're using stats protection and
	// the server isn't using stats at all
	bool bClientLogin;

#ifdef CLIENT_DLL

	bClientLogin = true;

#else

	bClientLogin = false;

#endif

#if defined( GAME_DLL ) && defined( STATS_PROTECTION )

	if( m_iType != STATSTYPE_ENABLED )
		bClientLogin = true;

#endif

#if defined( TESTING ) && defined( GAME_DLL )

	Msg("Stats: CINSStats::LoginPlayer - Player %s, Username %s, Password %s\n", pPlayer->GetPlayerName(), pszUsername, pszPassword);

#endif

	// try and login
	m_pStatsMan->Login( pPlayer, &CINSStats::LoginCallback, pszUsername, pszPassword, bClientLogin );
}

//=========================================================
//=========================================================
void CINSStats::LoginCallback( CINSPlayer *pPlayer, int iType, int iMemberID )
{
	if( !IsValid( ) )
	{
		Assert( false );
		return;
	}

#ifdef GAME_DLL

	if( !pPlayer )
		return;

#endif

	// check the type
	if( iType < 0 || iType > SAC_PLAYERTYPE_COUNT )
		iType = SAC_PLAYERTYPE_INVALID;

#ifdef GAME_DLL

	if( m_CurrentMemberIDs.Find( iMemberID ) != m_CurrentMemberIDs.InvalidIndex( ) )
		iType = SAC_PLAYERTYPE_INVALID;

	bool bValidPlayer = ( iType == SAC_PLAYERTYPE_VALID || iType == SAC_PLAYERTYPE_DEVELOPER );

#endif

#if defined( GAME_DLL ) && defined( TESTING )

	Msg("Stats: CINSStats::LoginCallback - Player %s\n", pPlayer->GetPlayerName( ) );

#endif

	// send a message
	if( !pPlayer || !pPlayer->IsBot( ) )
	{
	#ifdef GAME_DLL

		if( iType == SAC_PLAYERTYPE_DEVELOPER )
			pPlayer->SetDeveloper( );

		if( m_iType == STATSTYPE_ENABLED )
			pPlayer->SendPlayerLogin( iType );

	#else
		
		// register
		CStatsHelper::SendLoginCallback( iMemberID, iType );

		// setup player if in-game
		if( engine->IsInGame( ) )
			SetupPlayer( );

	#endif

	#ifdef GAME_DLL

		if( !bValidPlayer )
		{
			iMemberID = MEMBERID_INVALID;
		}

	#ifdef STATS_PROTECTION
		else if( m_iType == STATSTYPE_ENABLED || iType == SAC_PLAYERTYPE_DEVELOPER )
		{
			pPlayer->FadeOutBlack( INITIAL_FADEIN );

			pPlayer->ShowViewPortPanel( PANEL_INITIALSETUP, true );
		}
	#endif

	#endif
	}

#ifdef GAME_DLL

	if( bValidPlayer )
	{
		// login player
		pPlayer->StatsLogin( iMemberID );

		// register it with the currently logged in player
		if( m_iType == STATSTYPE_ENABLED )
			m_CurrentMemberIDs.AddToTail( iMemberID );
	}

#endif
}