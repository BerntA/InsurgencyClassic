//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TDM_RULES_H
#define TDM_RULES_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_gamerules.h"

//=========================================================
//=========================================================
class CINSObjective;

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CPBRules C_PBRules

#endif

class CTDMRules : public CINSRules
{
	DECLARE_CLASS( CTDMRules, CINSRules );

public:
	int GetGameType( void ) const { return GAMETYPE_TDM; }

#ifdef GAME_DLL

private:
	bool NeedsObjectives( void ) const;

	void UpdateSpawnPoints( CINSObjective *pCapturedObj, int &iT1Spawn, int &iT2Spawn );
	bool CheckSpawnPoints( void ) const { return false; }

	bool CheckForWinner( int &iWinningTeam );
	int GetDefaultWinner( void ) const;

#endif
};

#endif // TDM_RULES_H
