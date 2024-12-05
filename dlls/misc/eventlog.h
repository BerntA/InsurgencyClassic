//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#if !defined EVENTLOG_H
#define EVENTLOG_H

#ifdef _WIN32
#pragma once
#endif

#include <igameevents.h>
#include <igamesystem.h>

// PNOTE: see eventlog.cpp for changes

//=========================================================
//=========================================================
class CEventLog : public IGameEventListener2, public CBaseGameSystem
{
	
public:
	CEventLog( );
	virtual ~CEventLog( );

protected:
	virtual bool PrintEvent( IGameEvent * event );
	virtual bool PrintPlayerEvent( IGameEvent * event );

private:
	bool Init( void );
	void Shutdown( void );

	void FireGameEvent( IGameEvent *pEvent );
};

//=========================================================
//=========================================================
extern IGameSystem *GameLogSystem( void );

#endif // EVENTLOG_H
