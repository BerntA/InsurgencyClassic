//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_tdmrules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
REGISTER_GAMERULES_CLASS( CTDMRules );
DECLARE_GAMETYPE( GAMETYPE_TDM, CTDMRules );

//=========================================================
//=========================================================
#ifdef GAME_DLL

//=========================================================
//=========================================================
bool CTDMRules::NeedsObjectives( void ) const
{
	return false;
}

//=========================================================
//=========================================================
void CTDMRules::UpdateSpawnPoints( CINSObjective *pCapturedObj, int &iT1Spawn, int &iT2Spawn )
{
	iT1Spawn = iT2Spawn = INVALID_OBJECTIVE;
}

//=========================================================
//=========================================================
bool CTDMRules::CheckForWinner( int &iWinningTeam )
{
	return false;
}

//=========================================================
//=========================================================
int CTDMRules::GetDefaultWinner( void ) const
{
	return TEAM_NEUTRAL;
}

#endif