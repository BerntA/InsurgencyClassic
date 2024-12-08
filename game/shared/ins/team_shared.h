//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TEAM_SHARED_H
#define TEAM_SHARED_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class CINSPlayer;

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CTeam C_Team

#include "c_team.h"

#else

#include "team.h"

#endif

//=========================================================
//=========================================================
extern bool IsValidTeam( int iTeamID );

//=========================================================
//=========================================================
extern CTeam *GetGlobalTeam( int iTeamID );

//=========================================================
//=========================================================
extern bool OnSameTeam( CINSPlayer *pP1, CINSPlayer *pP2 );
extern bool OnSameTeam( CINSPlayer *pP1, int iTeamID );

extern bool OnSameSquad( CINSPlayer *pP1, CINSPlayer *pP2 );
extern bool OnSameSquad( CINSPlayer *pP1, int iSquadID );

#endif // TEAM_SHARED_H
