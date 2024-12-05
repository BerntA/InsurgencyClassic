//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "play_team_shared.h"

#ifdef CLIENT_DLL
#include "c_playerresource.h"
#endif // CLIENT_DLL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CIMCTeamConfig *CPlayTeam::GetIMCTeamConfig( void ) const
{
	Assert( m_pIMCTeamConfig );
	return m_pIMCTeamConfig;
}

//=========================================================
//=========================================================
bool CPlayTeam::IsUnlimitedWaves( void ) const
{
	return ( GetNumWaves( ) == UNLIMITED_SUPPLIES );
}

int CPlayTeam::GetTotalPlayerScore( void )
{
	int iRet=0;
	for (int j=0;j<m_Players.Count();j++)
	{
#ifdef CLIENT_DLL
		iRet+=PlayerResource()->GetMorale(m_Players[j]);
#else
		iRet+=m_Players[j]->GetMorale();
#endif // CLIENT_DLL
	}

	return iRet;
}