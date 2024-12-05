//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef MAPNAME_UTILS_H
#define MAPNAME_UTILS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

/*#define IMC_PATH_PREFIX "maps/"
#define IMC_PATH_SUFFIX ".imc"

#define CreateIMCString(string, mapname) \
	memset(string, 0, sizeof(string)); \
	Q_strcpy((char*)string, IMC_PATH_PREFIX); \
	Q_strcat((char*)string, (char*)mapname); \
	Q_strcat((char*)string, IMC_PATH_SUFFIX); \*/

#if !defined( CLIENT_DLL )

/*#define GetIMCLocation(string, mapname)	\
	CreateIMCString(string, mapname); \*/

#else

#include "utlvector.h"

#define MAPNAME_BLOAT_PREFIX_SIZE 5 // "maps\"
#define MAPNAME_BLOAT_SUFFIX_SIZE 3 // ".bsp"

#define ConvertClientMapNameToServerMapName(string, mapname) \
	memset(string, 0, sizeof(string)); \
	Q_strncpy(string, mapname + MAPNAME_BLOAT_PREFIX_SIZE, Q_strlen(mapname) - (MAPNAME_BLOAT_PREFIX_SIZE + MAPNAME_BLOAT_SUFFIX_SIZE)); \

#define GetIMCLocation(string, mapname) \
	char szMapName[1024]; \
	ConvertClientMapNameToServerMapName(szMapName, mapname); \
	CreateIMCString(string, szMapName); \

extern bool GetMapImagePath(const char *pszType, const char *pszName, bool bCycler, char *pszBuffer, int iLength, CUtlVector<int> *pValidImages = NULL);
extern void FormMapImagePath(const char *pszType, const char *pszName, bool bValid, char *pszBuffer, int iLength);
extern void GetVGUIMapName(char *pszBuffer);

#endif // !CLIENT_DLL

#endif // MAPNAME_UTILS_H
