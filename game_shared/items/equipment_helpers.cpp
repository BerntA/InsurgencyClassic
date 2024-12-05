//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "equipment_helpers.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
DEFINE_STRING_LOOKUP_CONSTANTS( int, itemtypes )
	ADD_LOOKUP( ITEM_BANDAGE )
	ADD_LOOKUP( ITEM_M67 )
	ADD_LOOKUP( ITEM_M18 )
	ADD_LOOKUP( ITEM_RGD5 )
END_STRING_LOOKUP_CONSTANTS( )