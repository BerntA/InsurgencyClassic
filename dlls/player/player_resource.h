//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//

#ifndef PLAYER_RESOURCE_H
#define PLAYER_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class CPlayerResource : public CBaseEntity
{
public:
	DECLARE_CLASS( CPlayerResource, CBaseEntity );
	DECLARE_SERVERCLASS( );
	DECLARE_DATADESC( );

public:
	~CPlayerResource( );

	static void Create( void );

	void Spawn( void );
	int ObjectCaps( void );
	void ResourceThink( void );
	void UpdatePlayerData( void );
	int UpdateTransmitState( void );

	void UpdatePlayerTeamID( int iID, int iTeamID );
	void UpdatePlayerDeveloper( int iID, int iDeveloper );

public:
	int	m_nUpdateCounter;

	CNetworkArray( bool, m_bConnected, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iPing, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iDeveloper, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iTeamID, MAX_PLAYERS + 1 );
	CNetworkArray( bool, m_bAlive, MAX_PLAYERS + 1 );

	CNetworkArray( int, m_iMorale, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iKills, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iDeaths, MAX_PLAYERS + 1 );
	
};

//=========================================================
//=========================================================
extern CPlayerResource *PlayerResource( void );

#endif // PLAYER_RESOURCE_H
