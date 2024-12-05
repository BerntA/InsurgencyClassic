//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "eventlog.h"
#include "keyvalues.h"

//=========================================================
//=========================================================
class CINSEventLog : public CEventLog
{
	DECLARE_BASECLASS( CEventLog );

private:
	bool PrintEvent( IGameEvent *pEvent );
	bool PrintINSEvent( IGameEvent *pEvent );
};

//=========================================================
//=========================================================
bool CINSEventLog::PrintEvent( IGameEvent *pEvent )
{
	if( BaseClass::PrintEvent( pEvent ) )
		return true;
	
	/*if( Q_strcmp( pEvent->GetName( ), "ins_" ) == 0 )
	{
		return PrintINSEvent( event );
	}*/

	return false;
}

//=========================================================
//=========================================================
bool CINSEventLog::PrintINSEvent( IGameEvent *pEvent )
{
	return false;
}

//=========================================================
//=========================================================
CINSEventLog g_INSEventLog;

//=========================================================
//=========================================================
IGameSystem *GameLogSystem( void )
{
	return &g_INSEventLog;
}

