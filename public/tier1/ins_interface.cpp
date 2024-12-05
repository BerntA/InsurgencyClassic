//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//

#ifdef _WIN32
#include <windows.h>
#endif

#include "interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
void *Sys_GetProcAddress( CSysModule* hModule, const char *pName )
{
	return GetProcAddress( reinterpret_cast<HMODULE>(hModule), pName );
}