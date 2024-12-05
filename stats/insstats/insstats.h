//========= Copyright © 2006 - 2008, James Mansfield, All rights reserved. ============//
//
// $NoKeywords: $
//=============================================================================//

#ifndef INSSTATS_H
#define INSSTATS_H
#ifdef _WIN32
#pragma once
#endif

#include <string>
#include <vector>

#include "playerstats.h"

#include "stats_protocal.h"
#include "stats_shared.h"
#include "stats_global.h"

using namespace std;

//=========================================================
//=========================================================
enum ForumUserGroups_t
{
	USERGRP_BANNED = 0,
	USERGRP_DEVELOPER,
	USERGRP_COUNT
};

typedef vector< int > UserGroup_t;

//=========================================================
//=========================================================
struct ServerConfig_t
{
	const char *m_pszMySQLHostname;
	unsigned int m_iPort;

	const char *m_pszMySQLUser, *m_pszMySQLPass;

	UserGroup_t m_UserGroups[ USERGRP_COUNT ];
};

//=========================================================
//=========================================================
class CStatsServer
{
public:
	static CStatsServer *StatsServer( void );

	bool Init( const char *pszFilename );

	void Execute( void );

	const ServerConfig_t &ServerConfig( void ) const;

private:
	CStatsServer( );

	bool LoadConfig( const char *pszFilename );
	void ParseUserGroup( char *pszText, UserGroup_t &UserGroup );

private:
	ServerConfig_t m_ServerConfig;
};

//=========================================================
//=========================================================
extern CStatsServer *StatsServer( );

#endif // INSSTATS_H