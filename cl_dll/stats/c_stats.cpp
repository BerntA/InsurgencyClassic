//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "ins_stats_shared.h"
#include "statsman.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
extern void DecodeROT13( char *pszBuffer );

//=========================================================
//=========================================================
void CStatsHelper::SendLoginCallback( int iMemberID, int iType )
{
	for( int i = 0; i < m_Helpers.Count( ); i++ )
		m_Helpers[ i ]->LoginCallback( iMemberID, iType );
}

//=========================================================
//=========================================================
CUtlVector< CStatsHelper* > CStatsHelper::m_Helpers;

CStatsHelper::CStatsHelper( )
{
	m_Helpers.AddToTail( this );
}

//=========================================================
//=========================================================
CStatsHelper::~CStatsHelper( )
{
	m_Helpers.FindAndRemove( this );
}

//=========================================================
//=========================================================
void CINSStats::SetupPlayer( const char *pszUsername, const char *pszPassword )
{
	// if both are NULL, assign ConVar's
	if( pszUsername && pszPassword )
	{
		// set username
		pszUsername = cl_stats_username.GetString( );

		// decode and set password
		char szPassword[ 128 ];
		Q_strncpy( szPassword, cl_stats_password.GetString( ), sizeof( szPassword ) );
		DecodeROT13( szPassword );

		pszPassword = szPassword;
	}

#ifdef TESTING

	Msg( "Stats: CINSStats::SetupPlayer - Username %s, Password %s\n", pszUsername, pszPassword );

#endif

	// don't bother if we don't have a valid configuration
	if( !pszUsername || !pszPassword || pszUsername[ 0 ] == '\0' || pszPassword[ 0 ] == '\0' )
		return;

	// if we do, try and login
	char szPlayerLogin[ 256 ];
	Q_snprintf( szPlayerLogin, sizeof( szPlayerLogin ), "playerlogin %s %s", pszUsername, pszPassword );

	engine->ServerCmd( szPlayerLogin );
}
