//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_GAMERULES_IDLE_H
#define INS_GAMERULES_IDLE_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class CIdleMode : public CModeBase
{
public:
	int Init( void );
	void Shutdown( void ) { }

	bool ShouldThink( void ) const { return true; }
	int Think( void ) { return GRMODE_NONE; }

	bool AllowBotAdd( void ) const { return false; }

	void HandlePlayerCount( void );
	bool PlayerViewpointSpawn( void ) const { return true; }
	void PlayerSpawn( CINSPlayer *pPlayer ) { }
	void PlayerJoinedTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam ) { }
	void PlayerLeftTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam ) { }
	void PlayerJoinedSquad( CINSPlayer *pPlayer, bool bFirstSquadChange ) { }

	void RagdollSpawn( CINSRagdoll *pPlayer ) { }

	void EntitiesCleaned( void ) { }

	bool AllowGameEnd( void ) const { return true; }
	bool ShouldGameEnd( void ) const { return false; }
};

#endif // INS_GAMERULES_IDLE_H
