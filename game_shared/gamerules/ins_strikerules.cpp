//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_strikerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
REGISTER_GAMERULES_CLASS( CStrikeRules );
DECLARE_GAMETYPE( GAMETYPE_STRIKE, CStrikeRules );

//=========================================================
//=========================================================
#ifdef GAME_DLL

bool CStrikeRules::IsEnoughObjectives( void ) const
{
	return ( GetObjectiveCount( ) == 2 );
}

#endif