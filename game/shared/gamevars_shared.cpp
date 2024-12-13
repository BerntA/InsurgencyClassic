//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "gamevars_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef GAME_DLL
void MPForceCameraCallback( IConVar *var, const char *pOldString, float flOldValue )
{
	if ( mp_forcecamera.GetInt() < OBS_ALLOW_ALL || mp_forcecamera.GetInt() >= OBS_ALLOW_NUM_MODES )
	{
		mp_forcecamera.SetValue( OBS_ALLOW_TEAM );
	}
}
#endif 

// some shared cvars used by game rules
ConVar mp_forcecamera( 
	"mp_forcecamera", 
#ifdef CSTRIKE
	"0", 
#else
	"1",
#endif
	FCVAR_REPLICATED,
	"Restricts spectator modes for dead players"
#ifdef GAME_DLL
	, MPForceCameraCallback 
#endif
	);