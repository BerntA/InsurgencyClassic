//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef STATSMAN_SHARED_H
#define STATSMAN_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "stats_shared.h"
#include "zthread/thread.h"
#include <string>

//=========================================================
//=========================================================
#ifdef STATSMAN_EXPORTS

	#ifdef WIN32

		#define STATMANS_API __declspec(dllexport)

    #else

		#define STATMANS_API

    #endif

#else

	#ifdef WIN32

		#define STATMANS_API __declspec(dllimport)

    #else

		#define STATMANS_API

    #endif

#endif

//=========================================================
//=========================================================
extern bool ConnectSAC( SWInetSocket &Socket, NetworkError &Error, NetworkSocket *pNetworkSocket );
extern bool SendSACHeader( int iMsgType, NetworkError &Error, NetworkSocket *pNetworkSocket );

class CSACThread : public ZThread::Runnable, public NetworkSocket
{
public:
	CSACThread( );

protected:
	bool ConnectSAC( SWInetSocket &Socket, NetworkError &Error );

	bool SendHeader( int iMsgType, NetworkError &Error );
};

//=========================================================
//=========================================================
class CStatsManThread : public CSACThread
{
private:
	CStatsMan *m_pParent;

public:
	CStatsManThread( CStatsMan *pParent );

#ifdef SERVER_STATSMAN

	bool ServerAuth( NetworkError &Error );

#endif
};

//=========================================================
//=========================================================
void htmlentities( std::string &strString );

#endif // STATSMAN_SHARED_H