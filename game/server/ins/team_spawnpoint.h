//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team spawnpoint entity
//
// $NoKeywords: $
//=============================================================================//

#ifndef TEAM_SPAWNPOINT_H
#define TEAM_SPAWNPOINT_H
#pragma once

#include "baseentity.h"

//=========================================================
//=========================================================
class CINSPlayer;
class CINSObjective;

//=========================================================
//=========================================================
class CViewPoint : public CPointEntity
{
	DECLARE_CLASS( CViewPoint, CPointEntity );

public:
	static CViewPoint *GetPoint( int iID );
	static int CountPoints( void );
	static void CleanPoints( void );

	void Activate( void );

private:
	static CUtlVector< CViewPoint* > m_Points;
};

//=========================================================
//=========================================================
class CSpawnPoint : public CPointEntity
{
	DECLARE_CLASS( CSpawnPoint, CPointEntity );
	DECLARE_DATADESC( );

public:
	CSpawnPoint( );

	static CSpawnPoint *GetFirstSpawn( void );
	static CSpawnPoint *GetSpawn( int iID );
	static int CountSpawns( void );
	static void CleanSpawns( void );
	static void TrimSpawn( CSpawnPoint *pSpawnPoint );

	bool KeyValue( const char *szKeyName, const char *szValue );

	void Activate( void );

	int GetParentID( void ) const { return m_iParentObjective; }
	CINSObjective *GetParent( void ) const;
	int GetSpawnGroup( void ) const { return m_iSpawnGroup; }
	int GetTeamID( void ) const { return m_iTeam; }
	int GetSquadID( void ) const { return m_iSquad; }
	bool IsReinforcement( void ) const { return m_bReinforcement; }
	Stance_t GetStance( void ) const { return m_iStance; }

	void KillColliding( CINSPlayer *pPlayer ) const;

private:
	bool IsValid( void ) const;

protected:
	int m_iParentObjective;
	int m_iTeam;

private:
	static CSpawnPoint *m_pFirstPoint;
	static CUtlVector< CSpawnPoint* > m_Points;

	int m_iSpawnGroup;
	int m_iSquad;
	bool m_bReinforcement;
	Stance_t m_iStance;
};

//=========================================================
//=========================================================
class CSpawnManager
{
public:
	CSpawnManager( );

	bool IsValid( void ) const;
	CSpawnPoint *FindSpawn( CINSPlayer *pPlayer );

	virtual CSpawnPoint *GetSpawn( int iID ) const = 0;
	virtual int Count( void ) const = 0;

private:
	int m_iLastSpawnID;
};

class CSpawnGlobal : public CSpawnManager
{
public:
	virtual CSpawnPoint *GetSpawn( int iID ) const;
	virtual int Count( void ) const;
};

class CSpawnGroup : public CSpawnGlobal
{
public:
	void AddSpawn( int iID );

	bool IsValid( CINSObjective *pParent, int iTeamID, int iSquadID ) const;

	CSpawnPoint *GetSpawn( int iID ) const;
	int Count( void ) const;

private:
	CUtlVector< int > m_Spawns;
};

//=========================================================
//=========================================================
void UTIL_CleanSpawnPoints( void );

#endif // TEAM_SPAWNPOINT_H
