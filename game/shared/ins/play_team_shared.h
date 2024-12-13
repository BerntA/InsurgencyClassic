//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef PLAY_TEAM_SHARED_H
#define PLAY_TEAM_SHARED_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CPlayTeam C_PlayTeam

#include "c_play_team.h"

#else

#include "play_team.h"

#endif

//=========================================================
//=========================================================
enum EmergencyDeployReturn_t
{
	EDRET_INVALID = 0,		// not allowed
	EDRET_DONE,				// it's been called
	EDRET_DOING,			// already been called
	EDRET_NONE,				// no more left
	EDRET_NOTATM,			// not yet
	EDRET_NONEED_PLAYERS,	// no-one to reinforce
	EDRET_NONEED_TIME,		// they're dereploying soon anyway
	EDRET_COUNT
};

//=========================================================
//=========================================================
inline bool IsPlayTeam( int iTeamID )
{
	return ( iTeamID == TEAM_ONE || iTeamID == TEAM_TWO );
}

//=========================================================
//=========================================================
inline bool IsPlayTeam2( int iTeamID )
{
	return ( iTeamID == 0 || iTeamID == 1 );
}

//=========================================================
//=========================================================
inline int TeamToPlayTeam( int iTeamID )
{
	Assert( IsPlayTeam( iTeamID ) );
	return ( ( iTeamID == TEAM_ONE ) ? 0 : 1 );
}

//=========================================================
//=========================================================
inline int PlayTeamToTeam( int iTeamID )
{
	Assert( IsPlayTeam2( iTeamID ) );
	return ( ( iTeamID == 0 ) ? TEAM_ONE : TEAM_TWO );
}

//=========================================================
//=========================================================
inline int FlipPlayTeam( int iTeamID )
{
	Assert( IsPlayTeam( iTeamID ) );
	return ( ( iTeamID == TEAM_ONE ) ? TEAM_TWO : TEAM_ONE );
}

//=========================================================
//=========================================================
extern CPlayTeam *GetGlobalPlayTeam( int iTeamID );

#endif // PLAY_TEAM_SHARED_H
