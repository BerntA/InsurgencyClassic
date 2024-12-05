//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef STATS_PROTECTION_H
#define STATS_PROTECTION_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================

// only use "stats protection" when testing stats protection or when we're *not* debugging

// uncomment this macro when you want to test stats protection in debug
//#define TESTING_STATS_PROTECTION

// uncomment this macro for testing to behave like debug
#define IGNORE_STATS_PROTECTION

//=========================================================
//=========================================================
#ifdef TESTING_STATS_PROTECTION

#define STATS_PROTECTION

#else

#if !defined( IGNORE_STATS_PROTECTION ) && defined( TESTING ) && !defined( _DEBUG )

#define STATS_PROTECTION

#endif

#endif

#endif // STATS_PROTECTION_H