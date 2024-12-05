//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "eventlog.h"
#include "keyvalues.h"
#include "team_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// PNOTE: keeps the same results as the Source version but
// working and better layout

//=========================================================
//=========================================================
CEventLog::CEventLog( )
{
}

//=========================================================
//=========================================================
CEventLog::~CEventLog( )
{
}

//=========================================================
//=========================================================
bool CEventLog::Init( void )
{
	gameeventmanager->AddListener( this, "player_changename", true );
	gameeventmanager->AddListener( this, "player_activate", true );
	gameeventmanager->AddListener( this, "player_death", true );
	gameeventmanager->AddListener( this, "player_team", true );
	gameeventmanager->AddListener( this, "player_disconnect", true );
	gameeventmanager->AddListener( this, "player_connect", true );

	return true;
}

//=========================================================
//=========================================================
void CEventLog::Shutdown( void )
{
	gameeventmanager->RemoveListener( this );
}

//=========================================================
//=========================================================
void CEventLog::FireGameEvent( IGameEvent *pEvent )
{
	PrintEvent( pEvent );
}

//=========================================================
//=========================================================
bool CEventLog::PrintEvent( IGameEvent *pEvent )
{
	const char *pszName = pEvent->GetName( );

	if( Q_strncmp( pszName, "server_", strlen( "server_" ) ) == 0 )
	{
		// we don't care about server events (engine does)
		return true;
	}
	else if( Q_strncmp( pszName, "player_", strlen( "player_" ) ) == 0 )
	{
		return PrintPlayerEvent( pEvent );
	}

	return false;
}

//=========================================================
//=========================================================
bool CEventLog::PrintPlayerEvent( IGameEvent *pEvent )
{
	const char *pszEventName = pEvent->GetName( );
	const int iUserID = pEvent->GetInt( "userid" );

	if ( !Q_strncmp( pszEventName, "player_connect", Q_strlen( "player_connect" ) ) )
	{
		// player connect is before the CBasePlayer pointer is setup
		const char *pszName = pEvent->GetString( "name" );
		const char *pszAddress = pEvent->GetString( "address" );
		const char *pszNetworkID = pEvent->GetString("networkid" );

		UTIL_LogPrintf( "\"%s<%i><%s><>\" connected, address \"%s\"\n",
			pszName, iUserID,
			pszNetworkID, pszAddress );

		return true;
	}
	else if ( !Q_strncmp( pszEventName, "player_disconnect", Q_strlen( "player_disconnect" ) ) )
	{
		const char *pszReason = pEvent->GetString( "reason" );
		const char *pszName = pEvent->GetString( "name" );
		const char *pszNetworkID = pEvent->GetString( "networkid" );

		CTeam *pTeam = NULL;
		CBasePlayer *pPlayer = UTIL_PlayerByUserId( iUserID );

		if( pPlayer )
			pTeam = GetGlobalTeam( pPlayer->GetTeamID( ) );

		UTIL_LogPrintf( "\"%s<%i><%s><%s>\" disconnected (reason \"%s\")\n",
			pszName, iUserID, pszNetworkID,
			pTeam ? pTeam->GetName( ) : "", pszReason );

		return true;
	}

	CBasePlayer *pPlayer = UTIL_PlayerByUserId( iUserID );

	if( !pPlayer )
	{
		DevMsg( "CEventLog::PrintPlayerEvent - failed to find player (userid: %i, event: %s)\n", iUserID, pszEventName );
		return false;
	}

	if( !Q_strncmp( pszEventName, "player_team", Q_strlen( "player_team" ) ) )
	{
		const bool bDisconnecting = pEvent->GetBool( "disconnect" );

		if ( !bDisconnecting )
		{
			const int iNewTeam = pEvent->GetInt( "team" );
			const int iOldTeam = pEvent->GetInt( "oldteam" );

			CTeam *pTeam = GetGlobalTeam( iNewTeam );
			CTeam *pOldTeam = GetGlobalTeam( iOldTeam );
			
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" joined team \"%s\"\n",
				pPlayer->GetPlayerName( ), pPlayer->GetUserID( ),
				pPlayer->GetNetworkIDString( ), pOldTeam ? pOldTeam->GetName( ) : "",
				pTeam ? pTeam->GetName( ) : "" );
		}

		return true;
	}
	else if( !Q_strncmp( pszEventName, "player_death", Q_strlen( "player_death" ) ) )
	{
		const int iAttackerID = pEvent->GetInt("attacker" );
		const char *pszWeapon = pEvent->GetString( "weapon" );
		
		CBasePlayer *pAttacker = UTIL_PlayerByUserId( iAttackerID );

		CTeam *pTeam = GetGlobalTeam( pPlayer->GetTeamID( ) );

		if ( pPlayer == pAttacker && pPlayer )
		{  
			// suicide
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" committed suicide with \"%s\"\n",
				pPlayer->GetPlayerName( ), iUserID, pPlayer->GetNetworkIDString( ),
				pTeam ? pTeam->GetName( ) : "", pszWeapon );
		}
		else if( pAttacker )
		{
			// normal kill
			CTeam *pAttackerTeam = GetGlobalTeam( pAttacker->GetTeamID( ) );

			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" killed \"%s<%i><%s><%s>\" with \"%s\"\n",
				pAttacker->GetPlayerName( ), iAttackerID, pAttacker->GetNetworkIDString( ),
				pAttackerTeam ? pAttackerTeam->GetName( ) : "",
				pPlayer->GetPlayerName( ), iUserID, pPlayer->GetNetworkIDString( ),
				pTeam ? pTeam->GetName( ) : "", pszWeapon );
		}
		else
		{  
			// killed by the world
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" committed suicide with \"world\"\n",
				pPlayer->GetPlayerName( ), iUserID, pPlayer->GetNetworkIDString( ),
				pTeam ? pTeam->GetName( ) : "" );
		}

		return true;
	}
	else if ( !Q_strncmp( pszEventName, "player_activate", Q_strlen( "player_activate" ) ) )
	{
		UTIL_LogPrintf( "\"%s<%i><%s><>\" entered the game\n",
			pPlayer->GetPlayerName( ), iUserID, pPlayer->GetNetworkIDString( ) );

		return true;
	}
	else if ( !Q_strncmp( pszEventName, "player_changename", Q_strlen( "player_changename" ) ) )
	{
		const char *pszNewName = pEvent->GetString( "newname" );
		const char *pszOldName = pEvent->GetString( "oldname" );

		CTeam *pTeam = GetGlobalTeam( pPlayer->GetTeamID( ) );

		UTIL_LogPrintf( "\"%s<%i><%s><%s>\" changed name to \"%s\"\n",
			pszOldName, iUserID, pPlayer->GetNetworkIDString( ),
			pTeam ? pTeam->GetName( ) : "", pszNewName );

		return true;
	}

	return false;
}