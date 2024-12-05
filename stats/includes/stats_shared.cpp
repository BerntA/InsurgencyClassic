//========= Copyright © 2005, James "Pongles" Mansfield, All Rights Reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "stats_protocal.h"
#include "stats_shared.h"

//=========================================================
//=========================================================
#if defined( SERVER_STATSMAN ) || defined( INS_DLL )

UpdateData_t::UpdateData_t( )
{
	m_iMemberID = 0;
	m_pPlayerStats = NULL;
}

//=========================================================
//=========================================================
UpdateData_t::UpdateData_t( const UpdateData_t &UpdateData )
{
	m_iMemberID = UpdateData.m_iMemberID;
	m_pPlayerStats = new CPlayerStats( *UpdateData.m_pPlayerStats );
}

#endif

//=========================================================
//=========================================================
#ifndef INS_DLL

#include <assert.h>

static const int g_iPrefixSize = 1;

using namespace std;

//=========================================================
//=========================================================
bool SocketOk( SWBaseError &Error )
{
	return ( SWBaseSocket::ok == Error );
}

//=========================================================
//=========================================================
bool NetworkError::SocketOk( void )
{
	return ::SocketOk( *this );
}

//=========================================================
//=========================================================
NetworkSocket::NetworkSocket(SWInetSocket *pSocket)
{
	m_pSocket = pSocket;

	m_szHeldBuffer[0] = 0;
	m_iHeldBytes = 0;
}

//=========================================================
//=========================================================
void NetworkSocket::SendPacket(int iLength, NetworkError &Error)
{
	m_pSocket->send((char*)&iLength, g_iPrefixSize, &Error);
}

//=========================================================
//=========================================================
int NetworkSocket::ReadPacket(char *pszPacket, NetworkError &Error)
{
	int iBytesRead = 0;
	int iPacketSize = 0;
	bool bBuildingPrefix = true;

	// copy any data remaining from previous call into output buffer
	if(m_iHeldBytes > 0)
	{
		memcpy(pszPacket, m_szHeldBuffer, m_iHeldBytes);
		iBytesRead += m_iHeldBytes;
		m_iHeldBytes = 0;
	}

	// read the packet
	while(true)
	{ 
		if(bBuildingPrefix)
		{
			if(iBytesRead >= g_iPrefixSize)
			{
				iPacketSize = pszPacket[0];

				if(iPacketSize <= 0)
					return 0;

				bBuildingPrefix = false;
			}
		}

		if(!bBuildingPrefix && (iBytesRead - g_iPrefixSize >= iPacketSize))
			break;

		int iNewBytesRead = m_pSocket->recv(pszPacket + iBytesRead, SAC_MAX_LENGTH - iBytesRead, &Error);

		if((iNewBytesRead == 0) || (!Error.SocketOk()))
			return 0;

		iBytesRead += iNewBytesRead;
	}

	// if anything is left in the read buffer, keep a copy of it for the next call
	m_iHeldBytes = iBytesRead - (iPacketSize + g_iPrefixSize);

	if(m_iHeldBytes > 0)
		memcpy(m_szHeldBuffer, pszPacket + iPacketSize + g_iPrefixSize, m_iHeldBytes);

	// skip past length
	char szNewPacket[SAC_MAX_LENGTH];
	strncpy(szNewPacket, pszPacket + g_iPrefixSize, iPacketSize);
	strncpy(pszPacket, szNewPacket, iPacketSize);
	pszPacket[iPacketSize] = '\0';

	return iPacketSize;
}

//=========================================================
//=========================================================
bool NetworkSocket::SendInt(NetworkSocket *pNetworkSocket, NetworkError &Error, int iValue)
{
	if(!pNetworkSocket)
		return false;

	SWInetSocket *pSocket = pNetworkSocket->GetSocket();

	char szBuffer[32];
	szBuffer[0] = '\0';

	Q_itoa(htonl(iValue), szBuffer);
	int iBufferSize = (int)strlen(szBuffer);

	pNetworkSocket->SendPacket(iBufferSize, Error);
	pSocket->send(szBuffer, iBufferSize, &Error);

	return true;
}

//=========================================================
//=========================================================
bool NetworkSocket::ReadInt(NetworkSocket *pNetworkSocket, NetworkError &Error, int &iValue)
{
	if(!pNetworkSocket)
		return false;

	SWInetSocket *pSocket = pNetworkSocket->GetSocket();

	char szPacket[92];

	if(pNetworkSocket->ReadPacket(szPacket, Error) == 0)
		return false;

	iValue = ntohl(atoi(szPacket));

	return true;
}

#endif // !INS_DLL