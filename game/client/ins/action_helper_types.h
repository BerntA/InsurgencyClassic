//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef ACTION_HELPER_TYPES_H
#define ACTION_HELPER_TYPES_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
enum ActionTypes_t
{
	ACTION_INVALID = -1,
	ACTION_DOOR = 0,
	ACTION_BIPOD,
	ACTION_WEAPON,
	ACTION_WCACHE,
	ACTION_AMMOBOX,
	ACTION_COUNT
};

#endif // ACTION_HELPER_TYPES_H