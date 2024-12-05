//========= Copyright © 2005, James "Pongles" Mansfield, All Rights Reserved. ============//
//
// Purpose: setup a Daemon that auth's users and manages updating a of user data
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>

#include "insstats.h"
#include "database.h"
#include "network.h"

#include "iniparser.h"
#include "stringtokenizer.h"

//=========================================================
//=========================================================
CStatsServer *CStatsServer::StatsServer( void )
{
	static CStatsServer *pStatsServer = NULL;

	if( !pStatsServer )
		pStatsServer = new CStatsServer;

	return pStatsServer;
}

//=========================================================
//=========================================================
CStatsServer *StatsServer( )
{
	return CStatsServer::StatsServer( );
}

//=========================================================
//=========================================================
bool CStatsServer::Init( const char *pszFilename )
{
	// load the config
	if( !LoadConfig( pszFilename ) )
	{
		printf( "Error when Loading Config\n" );
		return false;
	}

	// load the database
	CDatabaseManager *pDatabase = DatabaseManager( );

	if( !pDatabase->Init( ) )
	{
		printf( "Error Initializing Database\n" );
		return false;
	}

	// load the network
	CNetworkManager *pNetwork = NetworkManager( );

	if( !pNetwork->Init( ) )
	{
		printf( "Error Initializing Network\n" );
		return false;
	}

	return true;
}

//=========================================================
//=========================================================
bool CStatsServer::LoadConfig( const char *pszFilename )
{
	// sanity check filename
	if( !pszFilename || *pszFilename == '\0' )
		return false;

	// load ini
	dictionary *pINI = iniparser_load( pszFilename );

	if( !pINI )
		return false;

	// load mysql
	m_ServerConfig.m_pszMySQLHostname = iniparser_getstr( pINI, "mysql:hostname" );
	m_ServerConfig.m_iPort = iniparser_getint( pINI, "mysql:port", 0 );

	m_ServerConfig.m_pszMySQLUser = iniparser_getstr( pINI, "mysql:username" );
	m_ServerConfig.m_pszMySQLPass = iniparser_getstr( pINI, "mysql:password" );

	static const char *pszUserGroups[ USERGRP_COUNT ] = {
		"banned",		// USERGRP_BANNED
		"dev"			// USERGRP_DEVELOPER
	};

	// load user groups
	char szKey[ 64 ];

	for( int i = 0; i < USERGRP_COUNT; i++ )
	{
		sprintf( szKey, "ugroups:%s", pszUserGroups[ i ] );
		ParseUserGroup( iniparser_getstr( pINI, szKey ), m_ServerConfig.m_UserGroups[ i ] );
	}

	return true;
}

//=========================================================
//=========================================================
void CStatsServer::ParseUserGroup( char *pszText,UserGroup_t &Group )
{
	// do sanity check
    if( !pszText || *pszText == '\0' )
		return;

	// tokenize!
	StringTokenizer GroupTokens( pszText, "," );
	int iGroupID;

	while( GroupTokens.HasMoreTokens( ) )
	{
		const char *pszNextToken = GroupTokens.NextToken( );

		if( pszNextToken )
		{
			iGroupID = atoi( pszNextToken );

			if( iGroupID >= 0 )
				Group.push_back( iGroupID );
		}
	}
}

//=========================================================
//=========================================================
void CStatsServer::Execute( void )
{
	NetworkManager( )->Execute( );
}

//=========================================================
//=========================================================
int main( int argc, char **argv )
{
	if( argc != 2 )
	{
		printf( "useage: insstatsd [configfile]\n" );
		return false;
	}

	CStatsServer *pStatsServer = StatsServer( );

	if( !pStatsServer )
		return -1;

	if( !pStatsServer->Init( argv[ 1 ] ) )
		return -1;

#if defined( DEBUG ) && !defined( _WIN32 )

	// make a child process, make the parent process quit
	if( fork( ) != 0 )
		return 0;

#endif

	pStatsServer->Execute( );

	return 0;
}