//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef STEAMIDCHECK_H
#define STEAMIDCHECK_H
#ifdef _WIN32
#pragma once
#endif

class CINSPlayer;

//=========================================================
//=========================================================
enum SteamIDType_t
{
	STEAMIDTYPE_NORMAL = 0,
	STEAMIDTYPE_DEVELOPER,
	STEAMIDTYPE_BANNED,
	STEAMIDTYPE_GIMPED
};

//=========================================================
//=========================================================
extern int UTIL_FindSteamIDType( CINSPlayer *pPlayer );

#endif // STEAMIDCHECK_H