//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "team_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
bool IsValidTeam( int iTeamID )
{
	return ( iTeamID >= 0 && iTeamID < MAX_TEAMS );
}

//=========================================================
//=========================================================
int CTeam::GetNumPlayers( void ) const
{
	return m_Players.Size( );
}
