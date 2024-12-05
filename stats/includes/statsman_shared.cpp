//========= Copyright © 1996-2005, James Mansfield, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "stdio.h"

#include "statsman.h"
#include "statsman_shared.h"
#include "ins_stats_shared.h"

#include "md5.h"

#ifdef _WIN32

#include "delayload.h"

#endif

#include <string>
#include <vector>

using namespace std;
using namespace ZThread;

//=========================================================
//=========================================================
bool ConnectSAC( SWInetSocket &Socket, NetworkError &Error, NetworkSocket *pNetworkSocket )
{
	if( !pNetworkSocket )
		return false;

	// try and connect
	int i = 0;

	while( true )
	{
		Socket.connect( SAC_SERVER_PORT, STATS_SAC_HOSTNAME, &Error );

		// check for errors
		if( Error == SWBaseSocket::ok )
			break;

		// no response, try again later
		if( i > STATS_SAC_TIMEOUT )
			return false;

		Thread::sleep( STATS_SAC_WAIT );
		i++;
	}

	// set the socket
	pNetworkSocket->SetSocket( &Socket );

	return true;
}

//=========================================================
//=========================================================
bool SendSACHeader(int iMsgType, NetworkError &Error, NetworkSocket *pNetworkSocket)
{
	if(!pNetworkSocket)
		return false;

	SWInetSocket *pSocket = pNetworkSocket->GetSocket();

	if(!pSocket)
		return false;

	// send header
	SACHeader_t Header = { SAC_HEADER_MAGICNUMBER, SAC_PROTOCAL_VERSION, (char)iMsgType };
	pNetworkSocket->SendPacket(sizeof(Header), Error);

	pSocket->send(&Header.m_iMagicNumber, sizeof(char), &Error);
	pSocket->send(&Header.m_iProtocalVersion, sizeof(char), &Error);
	pSocket->send(&Header.m_iMessageType, sizeof(char), &Error);

	return true;
}

//=========================================================
//=========================================================
CSACThread::CSACThread( )
	: Runnable( ), NetworkSocket( NULL )
{
}

//=========================================================
//=========================================================
bool CSACThread::ConnectSAC(SWInetSocket &Socket, NetworkError &Error)
{
	return ::ConnectSAC(Socket, Error, this);
}

//=========================================================
//=========================================================
bool CSACThread::SendHeader(int iMsgType, NetworkError &Error)
{
	return ::SendSACHeader(iMsgType, Error, this);
}

//=========================================================
//=========================================================
CStatsManThread::CStatsManThread(CStatsMan *pParent)
	: CSACThread()
{
	m_pParent = pParent;
}

#ifdef SERVER_STATSMAN

extern bool ServerAuth(NetworkSocket *pNetworkSocket, NetworkError &Error, const char *pszUsername, const char *pszPassword);

bool CStatsManThread::ServerAuth(NetworkError &Error)
{
	return ::ServerAuth(this, Error, m_pParent->GetServerUsername(), m_pParent->GetServerPassword());
}

#endif

//=========================================================
//=========================================================
struct HTMLReplace_t {

	char m_cCharCode;
	char *m_pszEntity;

} g_HTMLReplace[ ] = {
	{ '"',	"&quot;" },
	{ '\'',	"&#039;" },
	{ '\'',	"&#39;" },
	{ '<',	"&lt;" },
	{ '>',	"&gt;" },
	{ '&',	"&amp;" },
	{ 0, NULL }
};

void htmlentities( string &strString )
{
	string::iterator StringIterator;
	string strTemp;

	for( StringIterator = strString.begin( ); StringIterator != strString.end( ); StringIterator++ )
	{
		int j;
		HTMLReplace_t HTMLReplace = g_HTMLReplace[ 0 ];

		for( j = 0, HTMLReplace; ; j++ )
		{
			HTMLReplace = g_HTMLReplace[ j ];

			if( HTMLReplace.m_cCharCode == 0 )
			{
				strTemp += *StringIterator;
				break;
			}

			if( *StringIterator == HTMLReplace.m_cCharCode )
			{
				strTemp += HTMLReplace.m_pszEntity;
				break;
			}
		}

	}
	strString = strTemp;
}

//=========================================================
//=========================================================
class CPlayerAuth : public CStatsManThread
{
private:
	CINSStats *m_pStats;
	
	CINSPlayer *m_pPlayer;
	LoginCallback_t m_LoginCallback;
	
	string m_strUsername, m_strPassword;

	bool m_bClientLogin;

public:
	CPlayerAuth( CStatsMan *pParent, CINSStats *pStats, CINSPlayer *pPlayer, LoginCallback_t LoginCallback, string strUsername, string strPassword, bool bClientLogin )
		: CStatsManThread( pParent )
	{
		m_strUsername = strUsername;
		m_strPassword = strPassword;

		m_pStats = pStats;

		m_pPlayer = pPlayer;
		m_LoginCallback = LoginCallback;

		m_bClientLogin = bClientLogin;
	}

private:
	void run( void )
	{
		if( RunLogin( ) )
			return;

		SendLoginInfo( SAC_PLAYERTYPE_SERVER );
	}

private:
	bool RunLogin( void )
	{
		SWInetSocket Socket;
		NetworkError Error;

		// connect to SAC
		if( !ConnectSAC( Socket, Error ) )
			return false;

		int iHeaderType;

	#ifdef CLIENT_STATSMAN

		m_bClientLogin = true;

	#endif

		if( m_bClientLogin )
			iHeaderType = SAC_MSGTYPE_CPAUTH;
		else
			iHeaderType = SAC_MSGTYPE_PAUTH;

		if( !SendHeader( iHeaderType, Error ) )
			return false;

	#ifdef GAME_DLL

		if( !m_bClientLogin && !ServerAuth( Error ) )
			return false;

	#endif

		int iPlayerType, iMemberID;
		iPlayerType = iMemberID = 0;

		SACAuth_t Auth;
		SACPlayerResult_t Result;

		char szPacket[ SAC_MAX_LENGTH ];
		int iPacketLength;

		Auth.m_iUserLength = ( char )m_strUsername.size( );
		strncpy( Auth.m_szPassword, m_strPassword.c_str( ), MD5_STRING_SIZE );

		// send auth
		SendPacket( sizeof( SACAuth_t ) + Auth.m_iUserLength, Error );
		m_pSocket->send( &Auth.m_iUserLength, sizeof(char), &Error );
		m_pSocket->send( Auth.m_szPassword, MD5_STRING_SIZE, &Error );
		m_pSocket->send( m_strUsername.c_str( ), Auth.m_iUserLength, &Error );

		// wait for the result
		if( ( iPacketLength = ReadPacket( szPacket, Error ) ) != 0 )
			memcpy( &Result.m_iPlayerType, &szPacket, sizeof( char ) );

		if( ReadInt( this, Error, Result.m_iMemberID ) )
			SendLoginInfo( Result.m_iPlayerType, Result.m_iMemberID );
		else
			SendLoginInfo( SAC_PLAYERTYPE_INVALID );

		return true;
	}
		
	void SendLoginInfo( int iType, int iMemberID = 0 )
	{
		(*m_pStats.*m_LoginCallback)(m_pPlayer, iType, iMemberID);
	}
};

//=========================================================
//=========================================================
CStatsMan::CStatsMan()
{
	sw_setVerboseMode(false);

	m_pStats = NULL;

#ifdef SERVER_STATSMAN
	m_bValid = false;
#endif
}

//=========================================================
//=========================================================
void CStatsMan::Login( CINSPlayer *pPlayer, LoginCallback_t LoginCallback, const char *pszUsername, const char *pszPassword, bool bClientLogin )
{
#ifdef SERVER_STATSMAN

	// exit if we're doing a server login and we're not valid
	if( !bClientLogin && !IsValid( ) )
		return;

#endif

	// do funky stuff to the password
	string strPassword = pszPassword;
	htmlentities( strPassword );

	// encyrpt the user password
	MD5Data_t MD5Password;
	ClearString( MD5Password );

	HashPassword( strPassword.c_str( ), &MD5Password );

	// create a thread that auth's the player
	Thread t( new CPlayerAuth( this, m_pStats, pPlayer, LoginCallback, pszUsername, MD5Password, bClientLogin ), true );
}

//=========================================================
//=========================================================
CStatsMan *GetStatsMan( void )
{
	return new CStatsMan( );
}

//=========================================================
//=========================================================
#ifdef WIN32

void InitDLL( char *pszSearchPath )
{
	__DLInitDelayLoadDLL( __DLNoFail );

	char szPath[ MAX_PATH ];
	strcpy( szPath, pszSearchPath );
	__DLSetSearchPath( szPath );

#ifdef _DEBUG

	__DLLoadDelayLoadDLL( "ins_socketsd.dll" );
	__DLLoadDelayLoadDLL( "ins_threadsd.dll" );

#else

	__DLLoadDelayLoadDLL( "ins_sockets.dll" );
	__DLLoadDelayLoadDLL( "ins_threads.dll" );

#endif

	__DLFinishDelayLoadDLL;
}

#endif
// deathz0rz ]