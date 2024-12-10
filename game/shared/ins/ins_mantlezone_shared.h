//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_MANTLEZONE_SHARED_H
#define INS_MANTLEZONE_SHARED_H

#ifdef _WIN32
#pragma once
#endif

#include "ins_touch.h"

#ifdef CLIENT_DLL

#define CMantleZone C_MantleZone

#endif

class CMantleZone : public CINSTouch
{
public:
	DECLARE_CLASS( CMantleZone, CINSTouch );
	DECLARE_NETWORKCLASS( );

#ifdef GAME_DLL

	DECLARE_DATADESC( );

	void PlayerThink( void );

protected:

	void PlayerStartTouch( CINSPlayer *pPlayer );
	void PlayerEndTouch( CINSPlayer *pPlayer );

	CUtlVector< CINSPlayer* > m_Players;

#endif
};

#endif // INS_MANTLEZONE_SHARED_H