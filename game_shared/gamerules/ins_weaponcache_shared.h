//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef INS_WEAPONCACHE_SHARED_H
#define INS_WEAPONCACHE_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_inventory_shared.h"

#ifdef GAME_DLL

#include "ins_weaponcache.h"

#else

#include "c_ins_weaponcache.h"

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CINSWeaponCache C_INSWeaponCache

#endif

//=========================================================
//=========================================================
enum WeaponCreateState_t
{
	WCACHESTATE_INVALID = 0,
	WCACHESTATE_HIDDEN,
	WCACHESTATE_ACTIVE
};

#define WCACHESTATE_BITS 4

#endif // INS_WEAPONCACHE_SHARED_H