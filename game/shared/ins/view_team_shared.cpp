//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#include "cbase.h"
#include "view_team_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
const char *g_pszViewTeam[ MAX_VIEW_TEAMS ] = {
	"Unassigned",
	"Spectator"
};

//=========================================================
//=========================================================
const char *CViewTeam::GetName( void ) const
{
	Assert( IsViewTeam( m_iTeamID ) );
	return ( g_pszViewTeam[ TeamToViewTeam( m_iTeamID ) ] );
}