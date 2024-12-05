//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef COMMAND_REGISTER_H
#define COMMAND_REGISTER_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

class CINSPlayer;

#endif

//=========================================================
//=========================================================

// NOTE: make sure all of these don't have any effect
// until the player joins a game (see gameuipanel.cpp)
enum CmdRegister_t
{
	CMDREGISTER_DEATHINFO = 0,
	CMDREGISTER_PFIREMODE,
	CMDREGISTER_STANCE,
	CMDREGISTER_IRONSIGHTHOLD,
	CMDREGISTER_3DSCOPE,
	CMDREGISTER_HIDEHINTS,
	CMDREGISTER_SWITCHDRAW,

	CMDREGISTER_COUNT
};

//=========================================================
//=========================================================
extern bool UTIL_IsValidCommandRegister( int iID );

//=========================================================
//=========================================================
#ifdef GAME_DLL

extern void UTIL_UpdateCommandRegister( CINSPlayer *pPlayer, int iID, int iValue );

#else

extern void UTIL_UpdateCommandRegisters( void );
extern void UTIL_SetCommandRegister( int iID, int iValue );

extern ConVar *UTIL_CommandRegisterCommand( int iID );
extern int UTIL_CommandRegisterValue( int iID );

#endif

#endif // COMMAND_REGISTER_H
