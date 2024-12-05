//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef STATSMAN_H
#define STATSMAN_H
#ifdef _WIN32
#pragma once
#endif

#include "stats_shared.h"
#include "stats_protocal.h"
#include "playerstats.h"

//=========================================================
//=========================================================
#define STATSMAN_USERNAME_LENGTH 32
#define STATSMAN_PASSWORD_LENGTH 32

//=========================================================
//=========================================================
class CINSStats;

class CINSPlayer;
typedef void ( CINSStats::*LoginCallback_t )( CINSPlayer *pPlayer, int iType, int iMemberID );
typedef void ( CINSStats::*PlayerStatsCallback_t )( CINSPlayer *pPlayer, CPlayerStats &PlayerStats );

//=========================================================
//=========================================================
class CStatsMan
{
public:
	CStatsMan( );

#ifdef SERVER_STATSMAN

	// setup the server stats management
	virtual void Init( const char *pszUsername, const char *pszPassword, CINSStats *pStats );

	// is this a valids stats server?
	virtual bool IsValid( void ) const;

#endif

#ifdef CLIENT_STATSMAN

	// setup the client stats management
	virtual void Init( CINSStats *pStats );

#endif

	// login a player, callback when finished
	virtual void Login( CINSPlayer *pPlayer, LoginCallback_t LoginCallback, const char *pszUsername, const char *pszPassword, bool bClientLogin );

#ifdef SERVER_STATSMAN

	// Update the Players Details
	virtual void Update( const PlayerUpdateData_t &PlayerUpdateData );
	virtual void Update( int iMemberID, const CPlayerStats *pPlayerStats );

	// get storted login Details
	virtual const char *GetServerUsername( void ) const;
	virtual const char *GetServerPassword( void ) const;

#endif

private:
	CINSStats *m_pStats;

#ifdef SERVER_STATSMAN

	bool m_bValid;

	char m_szUsername[ STATSMAN_USERNAME_LENGTH ];
	char m_szPassword[ STATSMAN_PASSWORD_LENGTH ];

#endif
};

//=========================================================
//=========================================================

// deathz0rz [
#ifdef WIN32

extern "C" __declspec(dllexport) CStatsMan *GetStatsMan(void);
extern "C" __declspec(dllexport) void InitDLL(char* pszSearchPath);

#elif _LINUX

extern "C" CStatsMan *GetStatsMan(void);

#endif
// deathz0rz ]

#endif // STATS_H
