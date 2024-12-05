//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifdef _WIN32

#define PROTECTED_THINGS_H

#include "cbase.h"
#include <windows.h>

#undef PROTECTED_THINGS_H

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
bool UTIL_IsVentRunning( void )
{
	HWND hWnd = FindWindow( NULL, "Ventrilo" );
	return ( hWnd != NULL );
}

#endif