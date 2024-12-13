//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef VIEW_TEAM_H
#define VIEW_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "team.h"

//=========================================================
//=========================================================
#define VIEW_TEAM_PLAYER_MODEL "models/player/us_marine.mdl"

//=========================================================
//=========================================================
class CViewTeam : public CTeam
{
public:
	DECLARE_CLASS( CViewTeam, CTeam );
	DECLARE_SERVERCLASS( );

public:
	static void Create( int iTeamID );

	const char *GetName( void ) const;
	const char *GetModel( void ) const { return VIEW_TEAM_PLAYER_MODEL; }

	int PlayerSpawnType( void ) const;
};

#endif // VIEW_TEAM_H