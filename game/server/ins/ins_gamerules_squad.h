//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_GAMERULES_SQUAD_H
#define INS_GAMERULES_SQUAD_H
#ifdef _WIN32
#pragma once
#endif

#include "play_team_shared.h"

//=========================================================
//=========================================================
class CINSPlayer;

//=========================================================
//=========================================================
class CSquadPlayerOrder
{
public:
	void Reset( void );
	
	void Add( CINSPlayer *pPlayer );
	void Remove( CINSPlayer *pPlayer );

	int Count( void ) const;
	CINSPlayer *operator[ ]( int iID ) const;
	CINSPlayer *Element( int iID ) const;

	void Sort( void );

private:
	CUtlVector< CINSPlayer* > m_Players;
};

//=========================================================
//=========================================================
class CEmptyClassPositionList : public CClassPositionList
{
public:
	CEmptyClassPositionList( );

	void Init( const CClassPositionList &ClassPositionList );

	bool Extract( int iPlayerClass, SquadData_t &SquadData );
};

//=========================================================
//=========================================================
class CSquadMode : public CModeBase
{
public:
	CSquadMode( );

	int Init( void );
	void Shutdown( void ) { }

	bool ShouldThink( void ) const { return true; }
	int Think( void );

	bool AllowBotAdd( void ) const { return true; }

	void HandlePlayerCount( void ) { }
	bool PlayerViewpointSpawn( void ) const { return true; }

	void PlayerSpawn( CINSPlayer *pPlayer ) { }
	void PlayerJoinedTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam );
	void PlayerLeftTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam );
	void PlayerJoinedSquad( CINSPlayer *pPlayer, bool bFirstSquadChange ) { }

	void RagdollSpawn( CINSRagdoll *pPlayer ) { }

	void EntitiesCleaned( void ) { }

	bool AllowGameEnd( void ) const { return true; }
	bool ShouldGameEnd( void ) const { return false; }

	int GetOrderLength( int iTeamID );
	int GetOrderElement( int iTeamID, int iElement );

private:
	void Setup( int iTeamID );

	const CSquadPlayerOrder &GetPlayerOrder( int iTeamID ) const;
	CSquadPlayerOrder &GetPlayerOrder( int iTeamID );

private:
	float m_flStartThreshold;

	CSquadPlayerOrder m_PlayerOrder[ MAX_PLAY_TEAMS ];
};

#endif // INS_GAMERULES_SQUAD_H
