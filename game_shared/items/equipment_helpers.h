//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef EQUIPMENT_HELPERS_H
#define EQUIPMENT_HELPERS_H
#ifdef _WIN32
#pragma once
#endif

#include "stringlookup.h"

//=========================================================
//=========================================================
enum Item_t
{
	INVALID_ITEM = -1,
	ITEM_BANDAGE = 0,
	ITEM_M67,
	ITEM_M18,
	ITEM_RGD5,
	ITEM_C4,
	ITEM_COUNT
};

//=========================================================
//=========================================================
DECLARE_STRING_LOOKUP_CONSTANTS( int, itemtypes )
#define GetItemID( k, j ) STRING_LOOKUP( itemtypes, k, j )

#endif // EQUIPMENT_HELPERS_H