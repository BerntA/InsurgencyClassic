//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef ACTION_HELPER_H
#define ACTION_HELPER_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#include "action_helper_types.h"

//=========================================================
//=========================================================
#include "inshud.h"

//=========================================================
//=========================================================
class IActionListener
{
public:
	IActionListener( );
	virtual ~IActionListener( );

	virtual void OnAction( int iActionID ) = 0;
};

//=========================================================
//=========================================================
class CActionHelper : public CAutoGameSystemPerFrame, public IINSPlayerDeath
{
public:
	CActionHelper( );

	int GetCurrentID( void ) const { return m_iCurrentID; }

	const char *Action( int iActionID ) const;

private:
	void Reset( void );

	void LevelInitPreEntity( void );
	void Update( float flFrameTime );

	bool CanUpdate( void ) const;
	int Update( void );

	void PlayerDeath( void );

private:
	int m_iCurrentID;
	float m_flNextUpdate;
};

//=========================================================
//=========================================================
extern CActionHelper g_ActionHelper;

#endif // ACTION_HELPER_H