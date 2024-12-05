//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_DEFINES_SHARED_H
#define WEAPON_DEFINES_SHARED_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
enum
{
	INVALID_WEAPON = -1,

	// usmc weapons
	WEAPON_M67 = 0,
	WEAPON_M18,
	WEAPON_M16A4,
	WEAPON_M16M203,
	WEAPON_M4,
	WEAPON_M9,
	WEAPON_M14,
	WEAPON_M249,
	WEAPON_M1014,
	WEAPON_C4,
	WEAPON_KABAR,

	// iraqi weapons
	WEAPON_RGD5,
	WEAPON_AK47,
	WEAPON_RPK,
	WEAPON_RPG7,
	WEAPON_MAKAROV,
	WEAPON_FNFAL,
	WEAPON_SKS,
	WEAPON_TOZ,
	WEAPON_L42A1,
	WEAPON_BAYONET,

	// dev weapons
	WEAPON_AR2,

	// misc weapons
	WEAPON_CUBEMAP,

	MAX_WEAPONS
};

#endif // WEAPON_DEFINES_SHARED_H