//========= Copyright © 1996-2005, James "Pongles" Mansfield, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef STATS_PROTOCAL_H
#define STATS_PROTOCAL_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#define MD5_STRING_SIZE 32

//=========================================================
//=========================================================
#define SAC_HEADER_MAGICNUMBER 26
#define SAC_PROTOCAL_VERSION 2
#define SAC_SERVER_PORT 2871

//=========================================================
//=========================================================
#define SAC_NETWORK_TIMEOUT 5
#define SAC_NETWORK_WAIT 50

#define SAC_MAX_CONNECTIONS 128

//=========================================================
//=========================================================
#define SAC_MAX_LENGTH 92

//=========================================================
//=========================================================
struct SACHeader_t
{
	char m_iMagicNumber;
	char m_iProtocalVersion;
	char m_iMessageType;
};

struct SACAuth_t
{
	char m_iUserLength;
	char m_szPassword[ MD5_STRING_SIZE ];
};

// char [ m_iUserLength ]

struct SACPlayerResult_t
{
	char m_iPlayerType;
	int m_iMemberID;
};

// char (depends on m_iPlayerType)

//=========================================================
//=========================================================
enum SACMsgType_t
{
	SAC_MSGTYPE_SAUTH = 0,			// server auth
	SAC_MSGTYPE_PAUTH_SERVER,		// player auth from server
	SAC_MSGTYPE_PAUTH_CLIENT,		// player auth from client
	SAC_MSGTYPE_PUPDATE				// update player stats
};

//=========================================================
//=========================================================
enum SACPlayerType_t
{
	SAC_PLAYERTYPE_SERVER = 0,
	SAC_PLAYERTYPE_INVALID,
	SAC_PLAYERTYPE_BANNED,
	SAC_PLAYERTYPE_VALID,
	SAC_PLAYERTYPE_DEVELOPER,
	SAC_PLAYERTYPE_COUNT
};

//=========================================================
//=========================================================
#define SAC_MAX_USERNAME 32
#define SAC_MAX_SERVERNAME SAC_MAX_USERNAME

#endif // STATS_PROTOCAL_H