//========= Copyright © 2005, James "Pongles" Mansfield, All Rights Reserved. ============//
//
// Purpose: setup a Daemon that auth's users and manages updating a of user data
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>

#include "insstats.h"
#include "database.h"
#include "network.h"

#include "iniparser.h"
#include "stringtokenizer.h"

//=========================================================
//=========================================================
class CNetworkConnection : public Runnable, public NetworkSocket
{
public:
	NetworkConnection( SWInetSocket *pSocket );
	virtual ~NetworkConnection( );

private:
	void run( void );

	void PlayerAuth( const char *pszServerName );
};

//=========================================================
//=========================================================
CNetworkConnection::CNetworkConnection( SWInetSocket *pSocket )
	: Runnable( ), NetworkSocket( pSocket )
{
	NetworkManager( )->AddConnection( );
}

//=========================================================
//=========================================================
CNetworkConnection::~CNetworkConnection( )
{
	NetworkManager( )->RemoveConnection( );
}

//=========================================================
//=========================================================
void CNetworkConnection::run( void )
{
	NetworkError Error;

	char szPacket[ SAC_MAX_LENGTH ];
	int iPacketLength;

	try {

		// load header
		SACHeader_t Header;

		if( ( iPacketLength = ReadPacket( szPacket, Error ) ) != sizeof( SACHeader_t ) )
			throw Error;

		memcpy( &Header, &szPacket, iPacketLength );

	#ifdef _DEBUG

		LogPrefix( m_pSocket );

		printf( "Header Received: { %i, %i, %i }\n",
			Header.m_iMagicNumber, Header.m_iProtocalVersion, Header.m_iMessageType );

	#endif

		if( Header.m_iMagicNumber != SAC_HEADER_MAGICNUMBER || Header.m_iProtocalVersion != SAC_PROTOCAL_VERSION )
			throw Error;

		// handle type
		int iResult;

		switch( Header.m_iMessageType )
		{
			case SAC_MSGTYPE_SAUTH:
				iResult = ServerAuth( );
				break;

			case SAC_MSGTYPE_PAUTH_SERVER:
				iResult = PlayerAuthServer( );
				break;

			case SAC_MSGTYPE_PAUTH_CLIENT:
				iResult = PlayerAuthClient( );
				break;

			case SAC_MSGTYPE_UPDATE:
				iResult = PlayerUpdate( );
				break;

		default:
			throw Error;
		}

		if( iResult == 0 )
			throw Error;
	}
	catch( SWBaseSocket::SWBaseError &CaughtError )
	{
		// do nothing, just continue to socket kill
	}

	KillSocket( );
}

//=========================================================
//=========================================================
int CNetworkConnection::ServerAuth( void )
{
	return ServerAuth( NULL );
}

//=========================================================
//=========================================================
int CNetworkConnection::PlayerAuthServer( void )
{
	int iResult;
	char szServerName[ SAC_MAX_SERVERNAME ];

	iResult = ServerAuth( szServerName );

	if( iResult != 0 )
		return PLAYERAUTH_ERROR;

	return PlayerAuth( szServerName );
}

//=========================================================
//=========================================================
int CNetworkConnection::PlayerAuthClient( void )
{
	return PlayerAuth( NULL );
}

//=========================================================
//=========================================================
int CNetworkConnection::PlayerUpdate( void )
{
	int iResult;
	char szServerName[ SAC_MAX_SERVERNAME ];

	iResult = ServerAuth( szServerName );

	if( iResult == 0 )
		return 0;

	// find all players
	NetworkError Error;

	char szPacket[ SAC_MAX_LENGTH ];
	int iPacketLength;

	for( int i = 0; i < MAX_STATS_PLAYERS; i++ )
	{
		// now, work out which member needs updating
		int iMemberID = 0;

		if( !ReadInt( this, Error, iMemberID ) )
			return 0;

		// work out wether or not this member has the right server
		char cResult = CanPlayerUpdate( iMemberID, szServerName ) ? 1 : 0;

		SendPacket( sizeof( char ), Error );
		m_pSocket->send( &cResult, sizeof( char ), &Error );

		if( cResult == 0 )
			continue;

		CPlayerStats PlayerStats;

		// now, start receiving the player data
		if( !ReadPlayerStats( this, PlayerStats, true ) )
			return 0;

	#ifdef _DEBUG

		LogPrefix(m_pSocket);
		printf("Player Stats: ");

		for(int i = 0; i < PlayerStats.CountTypes(); i++)
			printf("%i ", PlayerStats.GetValue(i));

		printf("\n");

		printf("Weapon Stats: ");

		for(int i = 0; i < MAX_WEAPONS; i++)
			for(int j = 0; j < PlayerStats.cGetWeaponStats(i).CountTypes(); j++)
				printf("%i ", PlayerStats.cGetWeaponStats(i).GetValue(j));

		printf("\n");

	#endif

		DatabaseManager( )->UpdateStatsMySQL( iMemberID, pszServerName, PlayerStats );
	}

	return 1;
}

//=========================================================
//=========================================================
int CNetworkConnection::ServerAuth( char *pszServerName )
{
	NetworkError Error;

	char szPacket[ SAC_MAX_LENGTH ];
	int iPacketLength;

	// otherwise, it's an auth
	SACAuth_t Auth;

	if( ( iPacketLength = ReadPacket( szPacket, Error ) ) == 0 )
		return SERVERAUTH_ERROR;

	memcpy( &Auth, &szPacket, iPacketLength );

	if( Auth.m_iUserLength == 0 || Auth.m_iUserLength > SAC_MAX_USERNAME )
		return SERVERAUTH_ERROR;

	// ... get his username
	char *pszUsername = szPacket + sizeof( SACAuth_t );
	szPacket[ sizeof( SACAuth_t ) + Auth.m_iUserLength ] = 0;

	char szPassword[ MD5_STRING_SIZE + 1 ];
	strncpy( szPassword, Auth.m_szPassword, MD5_STRING_SIZE );
	szPassword[ MD5_STRING_SIZE ] = '\0';

#ifdef WIN32

	LogPrefix(m_pSocket);
	printf("Username: %s\n", pszUsername);

	LogPrefix(m_pSocket);
	printf("Password: %s\n", szPassword);

#endif

	// now check with MySQL
	bool bValid = false;

	if( !DatabaseManager( )->ServerAuth( pszUsername, szPassword ) )
		return SERVERAUTH_INVALID;

	// send the result
	char cResult = bValid ? 1 : 0;
	SendPacket( sizeof( char ), Error );
	m_pSocket->send( &cResult, sizeof( char ), &Error );

#ifdef WIN32

	LogPrefix(m_pSocket);

	printf("Sent Result { %i }\n",
		cResult);

#endif

	return SERVERAUTH_VALID;
}

//=========================================================
//=========================================================
void CNetworkConnection::PlayerAuth( const char *pszServerName )
{
	NetworkError Error;

	char szPacket[ SAC_MAX_LENGTH ];
	int	iPacketLength;

	SACAuth_t Auth;

	if( ( iPacketLength = ReadPacket( szPacket, Error ) ) == 0 )
		return false;

	memcpy( &Auth, &szPacket, iPacketLength );

	if( Auth.m_iUserLength == 0 || Auth.m_iUserLength > SAC_MAX_USERNAME )
		return false;

	// ... get his username
	char *pszUsername =	szPacket + sizeof( SACAuth_t );
	szPacket[ sizeof( SACAuth_t) + Auth.m_iUserLength ] = '\0';

	char szPassword[ MD5_STRING_SIZE + 1 ];
	strncpy( szPassword, Auth.m_szPassword, MD5_STRING_SIZE );
	szPassword[ MD5_STRING_SIZE ]	= 0;

#ifdef _DEBUG

	LogPrefix( m_pSocket );
	printf("Username: %s\n", pszUsername);

	LogPrefix( m_pSocket );
	printf("Password: %s\n", szPassword);

#endif

	// now check with MySQL
	int	iPlayerType, iMemberID;
	DatabaseManager( )->PlayerAuth( pszUsername, szPassword, pszServerName, iPlayerType, iMemberID );

	// start constructing result
	SACPlayerResult_t Result = { ( char )iPlayerType, htons( iMemberID ) };

	// send	the	result
	char szBuffer[ 32 ];
	
	SendPacket( sizeof( char ), Error );
	m_pSocket->send( &Result.m_iPlayerType, sizeof( char ), &Error );

	// ... send	memberid
	if( !SendInt( this, Error, Result.m_iMemberID ) )
		return false;

#ifdef _DEBUG

	LogPrefix( m_pSocket );

	printf( "Sent Result	{ %i, %i }\n",
		Result.m_iPlayerType, Result.m_iMemberID );

#endif

	return true;
}

//=========================================================
//=========================================================
bool CNetworkConnection::ReadStats( const IStatsBase &StatsBase )
{
	NetworkError Error;
	int iValue;

	for( int i = 0; i < InfoStats.CountTypes( ); i++ )
	{
		if( !ReadInt( iValue, Error) )
			return false;

		InfoStats.SetType( i, iValue );
	}

	return true;
}

//=========================================================
//=========================================================
bool CNetworkConnection::ReadPlayerStats( const CPlayerStats & )
{
	// read player part
	if( !ReadStats( PlayerStats ) )
		return false;

	// read weapon part
	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if( !ReadStats( PlayerStats.GetWeaponStats( i ) ) )
			return false;
	}

	return true;
}

//=========================================================
//=========================================================
void CNetworkConnection::KillSocket( void )
{
#ifdef _DEBUG

	LogPrefix( m_pSocket );
	printf(	"Client	Disconnected\n"	);

#endif

	m_pSocket->disconnect( );

	delete m_pSocket;
}

//=========================================================
//=========================================================
CNetworkManager::CNetworkManager( )
{
	m_iConnections = 0;
}

//=========================================================
//=========================================================
bool CNetworkManager::Init( void )
{
	// set values
	sw_setThrowMode( true );

#ifdef _DEBUG

	sw_setVerboseMode( false );

#endif

	// bind a socket
	SWBaseError Error;
	m_Listener.bind( SAC_SERVER_PORT, &Error );

	if( !SocketOk( Error ) )
	{
		printf( "Unable to Bind to a Socket" );
		return false;
	}

	return true;
}

//=========================================================
//=========================================================
void CNetworkManager::Start( void )
{
	// setup network
	m_Listener.listen( );

#ifdef _DEBUG

	printf( "Server Running on %s:%d (%s)\n", g_Listener.get_hostName( ).c_str( ), g_Listener.get_hostPort( ), g_Listener.get_hostAddr( ).c_str( ) );

#endif
	
	while( true )
	{
		SWInetSocket *pSocket = ( SWInetSocket* )m_Listener.accept( );
		pSocket->set_timeout( STATS_SAC_TIMEOUT, 0 );

		if( m_iConnections >= STATS_MAXCONNECTIONS )
		{
			pSocket->disconnect( );
			continue;
		}

	#ifdef _DEBUG

		printf( "*** Client Connected (%s - %i)\n", pSocket->get_hostAddr( ).c_str( ), m_iConnections );

	#endif

		Thread t( new NetworkConnection( pSocket ) );
	}
}

//=========================================================
//=========================================================
void CNetworkManager::AddConnection( void )
{
	m_iConnections++;
}

//=========================================================
//=========================================================
void CNetworkManager::RemoveConnection( void )
{
	m_iConnections--;
}