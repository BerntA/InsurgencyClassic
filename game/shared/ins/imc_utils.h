//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef IMC_UTILS_H
#define IMC_UTILS_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL

#define MAPNAME_BLOAT_PREFIX_SIZE 5 // "maps\"
#define MAPNAME_BLOAT_SUFFIX_SIZE 3 // ".bsp"

#define ConvertClientMapName( string ) \
	string[ 0 ] = '\0'; \
	Q_strncpy( string, engine->GetLevelName( ) + MAPNAME_BLOAT_PREFIX_SIZE, Q_strlen( engine->GetLevelName( ) ) - ( MAPNAME_BLOAT_PREFIX_SIZE + MAPNAME_BLOAT_SUFFIX_SIZE ) );

#endif

#endif // IMC_UTILS_H
