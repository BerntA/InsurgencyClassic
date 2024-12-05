//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef INS_STATS_SHARED_H
#define INS_STATS_SHARED_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#include "playerstats.h"
#include "stats_protection.h"

//=========================================================
//=========================================================
class CINSPlayer;
class CStatsMan;
class CSysModule;
class CStatsMan;

//=========================================================
//=========================================================
enum StatsType_t
{
	STATSTYPE_NONE = 0,			// no stats
	STATSTYPE_READONLY,			// read the stats
	STATSTYPE_ENABLED,			// stats enabled server!
	STATSTYPE_BLOCKED			// the entire server is blocked
};

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

class CStatsHelper
{
public:
	CStatsHelper( );
	~CStatsHelper( );

	static void SendLoginCallback( int iMemberID, int iType );

	virtual void LoginCallback( int iMemberID, int iType ) = 0;

private:
	static CUtlVector< CStatsHelper* > m_Helpers;
};

#endif

//=========================================================
//=========================================================
class CINSStats
{
public:
	static CINSStats *GetINSStats( void );
	~CINSStats( );

	void Init( void );

	bool IsValid( void ) const;

#ifdef CLIENT_DLL

	static bool IsBlocked( C_INSPlayer *pPlayer );

#endif

#ifdef GAME_DLL

	int GetType( void ) const { return m_iType; }

#endif

	void LoginPlayer( CINSPlayer *pPlayer, const char *pszUsername, const char *pszPassword );
	void LoginCallback( CINSPlayer *pPlayer, int iType, int iMemberID );

#ifdef GAME_DLL

	void LogoutPlayer( CINSPlayer *pPlayer );
	void LogoutPlayers( void );

#else

	void SetupPlayer( const char *pszUsername = NULL, const char *pszPassword = NULL );

#endif

private:
	CINSStats( );

	void LoadDLLs( void );

private:

#ifdef GAME_DLL

	int m_iType;

#endif

	CStatsMan *m_pStatsMan;
	CSysModule *m_hStatsManDLL;

#ifdef GAME_DLL

	CUtlVector< int > m_CurrentMemberIDs;

#endif
};

//=========================================================
//=========================================================
extern CINSStats *GetINSStats( void );

#endif // INS_STATS_SHARED_H
