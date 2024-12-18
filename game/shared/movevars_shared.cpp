//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "movevars_shared.h"
#include "ins_shared_global.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// some cvars used by player movement system

float GetCurrentGravity(void)
{
	return sv_gravity.GetFloat();
}

ConVar	sv_gravity("sv_gravity", INS_DEFAULT_GRAVITY, FCVAR_NOTIFY | FCVAR_REPLICATED, "World gravity.");
ConVar	sv_stopspeed("sv_stopspeed", "100", FCVAR_NOTIFY | FCVAR_REPLICATED, "Minimum stopping speed when on ground.");
ConVar	sv_stopspeed_prone("sv_stopspeed_prone", "45", FCVAR_NOTIFY | FCVAR_REPLICATED, "Minimum stopping speed when on ground and prone.");
ConVar	sv_noclipaccelerate("sv_noclipaccelerate", "5", FCVAR_NOTIFY | FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar	sv_noclipspeed("sv_noclipspeed", "5", FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_specaccelerate("sv_specaccelerate", "5", FCVAR_NOTIFY | FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar	sv_specspeed("sv_specspeed", "3", FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_specnoclip("sv_specnoclip", "1", FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_maxspeed("sv_maxspeed", "320", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_accelerate("sv_accelerate", "5", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_accelerate_supported("sv_accelerate_supported", "250", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_airaccelerate("sv_airaccelerate", "10", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_wateraccelerate("sv_wateraccelerate", "10", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_waterfriction("sv_waterfriction", "1", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_footsteps("sv_footsteps", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Play footstep sound for players");
ConVar	sv_friction("sv_friction", "4", FCVAR_NOTIFY | FCVAR_REPLICATED, "World friction.");
ConVar	sv_bounce("sv_bounce", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Bounce multiplier for when physically simulated objects collide with other objects.");
ConVar	sv_maxvelocity("sv_maxvelocity", "3500", FCVAR_REPLICATED, "Maximum speed any ballistically moving object is allowed to attain per axis.");
ConVar	sv_stepsize("sv_stepsize", "18", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_backspeed("sv_backspeed", "0.6", FCVAR_ARCHIVE | FCVAR_REPLICATED, "How much to slow down backwards motion");
ConVar  sv_waterdist("sv_waterdist", "12", FCVAR_REPLICATED, "Vertical view fixup when eyes are near water plane.");
ConVar	sv_skyname("sv_skyname", "sky_urb01", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Current name of the skybox texture");
