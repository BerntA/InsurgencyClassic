//========= Copyright © 1996-2005, James "Pongles" Mansfield, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef STATS_SHARED_H
#define STATS_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "stats_global.h"
#include "playerstats.h"

//=========================================================
//=========================================================
#if defined( SERVER_STATSMAN ) || defined( GAME_DLL )

struct UpdateData_t
{
	UpdateData_t( );
	UpdateData_t( const UpdateData_t &UpdateData );

	int m_iMemberID;
	const CPlayerStats *m_pPlayerStats;
};

typedef UpdateData_t PlayerUpdateData_t[ MAX_STATS_PLAYERS ];

#endif

//=========================================================
//=========================================================
#ifndef INS_DLL

#include "socketw.h"
#include "stats_protocal.h"

//=========================================================
//=========================================================
typedef SWBaseSocket::SWBaseError SWBaseError;

class CNetworkError : public SWBaseError
{
public:
	NetworkError( ) { }

	bool SocketOk( void );
};

extern bool SocketOk( SWBaseError &Error );

//=========================================================
//=========================================================
class CNetworkSocket
{
public:
	NetworkSocket( SWInetSocket *pSocket );

	void SetSocket( SWInetSocket *pSocket ) { m_pSocket = pSocket; }
	SWInetSocket *GetSocket( void ) { return m_pSocket; }

	void SendPacket( int iLength, NetworkError &Error );
	int ReadPacket( char *pszPacket, NetworkError &Error );

	bool SendInt( int iValue, NetworkError &Error );
	bool ReadInt( int &iValue, NetworkError &Error );

protected:
	SWInetSocket *m_pSocket;

private:
	char m_szHeldBuffer[ SAC_MAX_LENGTH ];
	int m_iHeldBytes;
};

#endif // !INS_DLL

#endif // STATS_SHARED_H
