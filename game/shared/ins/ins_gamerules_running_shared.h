//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_GAMERULES_RUNNING_SHARED_H
#define INS_GAMERULES_RUNNING_SHARED_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#define GAMERUNNING_PLAYERWAIT	( 1 << 0 )
#define GAMERUNNING_RESTARTING	( 1 << 1 )
#define GAMERUNNING_ENDGAME		( 1 << 2 )
#define GAMERUNNING_WARMINGUP	( 1 << 3 )
#define GAMERUNNING_FREEZE		( 1 << 4 )

#define GAMERUNNING_MAXBITS 5

//=========================================================
//=========================================================
#define ROUNDTIMER_INVALID -1

//=========================================================
//=========================================================
#define FROZENLENGTH_BITS 4

#endif // INS_GAMERULES_RUNNING_SHARED_H