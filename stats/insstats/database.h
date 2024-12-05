//========= Copyright © 2006 - 2008, James Mansfield, All rights reserved. ============//
//
// $NoKeywords: $
//=============================================================================//

#ifndef DATABASEMGR_H
#define DATABASEMGR_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
struct MYSQL;
struct MYSQL_RES;

//=========================================================
//=========================================================
class CDatabaseManager
{
public:
	CDatabaseManager( );

	bool Init( void );

	int ServerAuth( const char *pszUsername, const char *pszPassword, bool &bValid );

	int PlayerAuth( const char *pszUsername, const char *pszPassword, const char *pszServerName, int &iPlayerType, int &iMemberID );
	bool CanPlayerUpdate( int iMemberID, const char *pszrServerName );
	bool UpdatePlayerStats( int iMemberID, const char *pszrServerName, const CPlayerStats &PlayerStats );

private:
	bool CreatePlayerStats( int iMemberID, const char *pszDatabase );

	bool WritePlayerStats( int iMemberID, const char *pszDatabase, const CPlayerStats &PlayerStats );

	void ReadPlayerStats( int iMemberID, const char *pszDatabase, CPlayerStats &PlayerStats );
	void ReadStats( MYSQL_RES *pResource, CBaseStats &BaseStats );

	void CreateDatabaseName( const string &strDatabase );

private:
	MYSQL *m_pMySQLConnection;
};

//=========================================================
//=========================================================
extern CDatabaseManager *DatabaseManager( void );

#endif // DATABASEMGR_H