//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#include "cbase.h"
#include "c_ins_weaponcache.h"
#include "action_helper_types.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
int C_INSWeaponCache::ActionType( void ) const
{
	return ACTION_WCACHE;
}