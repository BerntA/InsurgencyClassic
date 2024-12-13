//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef STRIKE_RULES_H
#define STRIKE_RULES_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_pushrules.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CStrikeRules C_StrikeRules

#endif

class CStrikeRules : public CPushRules
{
	DECLARE_CLASS( CStrikeRules, CPushRules );

private:
	int GetGameType( void ) const { return GAMETYPE_STRIKE; }

#ifdef GAME_DLL

	bool IsEnoughObjectives( void ) const;

#endif
};

#endif // STRIKE_RULES_H
