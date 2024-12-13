//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef VIEW_TEAM_SHARED_H
#define VIEW_TEAM_SHARED_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CViewTeam C_ViewTeam

#include "c_view_team.h"

#else

#include "view_team.h"

#endif

//=========================================================
//=========================================================
inline bool IsViewTeam( int iTeamID )
{
	return ( iTeamID == TEAM_UNASSIGNED || iTeamID == TEAM_SPECTATOR );
}

//=========================================================
//=========================================================
inline int TeamToViewTeam( int iTeamID )
{
	Assert( IsViewTeam( iTeamID ) );
	return ( ( iTeamID == TEAM_UNASSIGNED ) ? 0 : 1 );
}

//=========================================================
//=========================================================
extern CViewTeam *GetGlobalViewTeam( int iTeamID );

#endif // VIEW_TEAM_SHARED_H