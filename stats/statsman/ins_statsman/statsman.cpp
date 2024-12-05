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

#include <string>
#include <vector>

using namespace std;
using namespace ZThread;

//=========================================================
//=========================================================
bool ServerAuth( NetworkSocket *pNetworkSocket, NetworkError &Error, const char *pszUsername, const char *pszPassword )
{
	if( !pNetworkSocket || !pszUsername || !pszPassword )
		return false;

	SWInetSocket *pSocket = pNetworkSocket->GetSocket( );

	if( !pSocket )
		return false;

	SACAuth_t Auth;

	char szPacket[ SAC_MAX_LENGTH ];

	MD5Data_t MD5Password;
	ClearString( MD5Password );
	HashPassword( pszPassword, &MD5Password );

	Auth.m_iUserLength = ( char )strlen( pszUsername );
	strncpy( Auth.m_szPassword, MD5Password, MD5_STRING_SIZE );

	// send auth
	pNetworkSocket->SendPacket( sizeof( SACAuth_t ) + Auth.m_iUserLength, Error );
	pSocket->send( &Auth.m_iUserLength, sizeof( char ), &Error );
	pSocket->send( Auth.m_szPassword, MD5_STRING_SIZE, &Error );
	pSocket->send( pszUsername, Auth.m_iUserLength, &Error );

	// wait for the result
	if( pNetworkSocket->ReadPacket( szPacket, Error ) == 0 )
		return false;

	char cResult;
	memcpy( &cResult, &szPacket, sizeof( char ) );

	return ( cResult == 1 ) ? true : false;
}

//=========================================================
//=========================================================
class CPlayerUpdate : public CStatsManThread
{
private:
	vector<UpdateData_t> m_UpdateData;

public:
	CPlayerUpdate( CStatsMan *pParent, const PlayerUpdateData_t &PlayerUpdateData )
		: CStatsManThread( pParent )
	{
		for( int i = 0; i < MAX_STATS_PLAYERS; i++ )
		{
			const UpdateData_t &UpdateData = PlayerUpdateData[ i ];

			if(!UpdateData.m_pPlayerStats )
				break;

			m_UpdateData.push_back(UpdateData);
		}
	}

	CPlayerUpdate(CStatsMan *pParent, const UpdateData_t &UpdateData)
		: CStatsManThread(pParent)
	{
		m_UpdateData.push_back(UpdateData);
	}

private:
	void run(void)
	{
		SWInetSocket Socket;
		NetworkError Error;

		// connect to the SAC
		if(!ConnectSAC(Socket, Error))
			return;

		SendHeader(SAC_MSGTYPE_UPDATE, Error);

		// send the auth details
		if( !ServerAuth( Error ) )
			return;

		// now, send off the player details
		char szPacket[92];
		char cPlayerStatus = 1;
		bool bFirstPlayer, bLastPlayer;
		bFirstPlayer = true;
		bLastPlayer = false;

		for(int i = 0; i < (int)m_UpdateData.size(); i++)
		{
			const CPlayerStats *pPlayerStats = m_UpdateData[i].m_pPlayerStats;

			if(!pPlayerStats)
				continue;

			if(!bFirstPlayer)
			{
				SendPacket(sizeof(char), Error);
				m_pSocket->send(&cPlayerStatus, sizeof(char), &Error);

				bFirstPlayer = false;
			}

			if(!SendInt(this, Error, m_UpdateData[i].m_iMemberID))
				return;

			if(ReadPacket(szPacket, Error) == 0)
				return;

			char cResult;
			memcpy(&cResult, &szPacket, sizeof(char));

			if(!cResult)
				continue;
		
			if(!SendPlayerStats(this, *pPlayerStats, true))
				return;

			if(i == (MAX_STATS_PLAYERS - 1))
				bLastPlayer = true;
		}

		if(!bLastPlayer)
		{
			cPlayerStatus = 0;

			SendPacket(sizeof(char), Error);
			m_pSocket->send(&cPlayerStatus, sizeof(char), &Error);
		}
	}
};

//=========================================================
//=========================================================
void CStatsMan::Init(const char *pszUsername, const char *pszPassword, CINSStats *pStats)
{
	m_pStats = pStats;

	if(!pszUsername || !pszPassword)
		return;

	SWInetSocket Socket;
	NetworkError Error;

	NetworkSocket NSocket(&Socket);

	// connect to SAC and auth
	if(!(ConnectSAC(Socket, Error, &NSocket) && 
		SendSACHeader(SAC_MSGTYPE_SAUTH, Error, &NSocket) &&
		ServerAuth(&NSocket, Error, pszUsername, pszPassword)))
		return;

	// save everything off
	strncpy(m_szUsername, pszUsername, MAX_USERNAME_LENGTH);
	strncpy(m_szPassword, pszPassword, MAX_PASSWORD_LENGTH);

	// set valid
	m_bValid = true;
}

//=========================================================
//=========================================================
bool CStatsMan::IsValid(void) const
{
	return (m_pStats != NULL); 
}

//=========================================================
//=========================================================
void CStatsMan::Update(const PlayerUpdateData_t &PlayerUpdateData)
{
	if(!IsValid())
		return;

	Thread t(new CPlayerUpdate(this, PlayerUpdateData), true);
}

//=========================================================
//=========================================================
void CStatsMan::Update(int iMemberID, const CPlayerStats *pPlayerStats)
{
	if(!IsValid())
		return;

	UpdateData_t UpdateData;
	UpdateData.m_iMemberID = iMemberID;
	UpdateData.m_pPlayerStats = pPlayerStats;

	Thread t(new CPlayerUpdate(this, UpdateData), true);
}

//=========================================================
//=========================================================
const char *CStatsMan::GetServerUsername(void) const
{
	if(!IsValid())
		return NULL;

	return m_szUsername;
}

//=========================================================
//=========================================================
const char *CStatsMan::GetServerPassword(void) const
{
	if(!IsValid())
		return NULL;

	return m_szPassword;
}