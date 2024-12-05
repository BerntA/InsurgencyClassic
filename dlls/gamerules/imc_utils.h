//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef IMC_UTILS_H
#define IMC_UTILS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#define IMC_PATH_PREFIX "maps/"
#define IMC_PATH_SUFFIX ".imc"

#define GetIMCLocation(string, keyvalues) \
	memset(string, 0, sizeof(string)); \
	Q_strcpy((char*)string, IMC_PATH_PREFIX); \
	Q_strcat((char*)string, STRING(gpGlobals->mapname)); \
	Q_strcat((char*)string, IMC_PATH_SUFFIX); \
	if(keyvalues) \
		Q_strcat((char*)string, "2");

#endif // IMC_UTILS_H
