//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ========//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef INS_INTERFACE_H
#define INS_INTERFACE_H

#ifdef _WIN32
#pragma once
#endif

#include "interface.h"

//=========================================================
//=========================================================
extern void *Sys_GetProcAddress( CSysModule* hModule, const char *pName );

#endif // INS_INTERFACE_H
