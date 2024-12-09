//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_AREA_H
#define INS_AREA_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_touch.h"

//=========================================================
//=========================================================
class CINSArea : public CINSTouch
{
public:
	DECLARE_CLASS( CINSArea, CINSTouch );
	DECLARE_DATADESC( );

public:
	void Spawn( void );

	const char *GetTitle( void ) const;

private:
	void PlayerStartTouch( CINSPlayer *pPlayer );
	void PlayerEndTouch( CINSPlayer *pPlayer );

private:
	string_t m_strTitle;
};

//=========================================================
//=========================================================
extern CUtlVector< CINSArea* > g_INSAreas;

#endif // INS_AREA_H