//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_PLAYERRESOURCE_H
#define C_PLAYERRESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "igameresources.h"
#include "squad_data.h"

//=========================================================
//=========================================================
class C_PlayerResource : public C_BaseEntity, public IGameResources
{
	DECLARE_CLASS( C_PlayerResource, C_BaseEntity );
	DECLARE_CLIENTCLASS( );

public:
	C_PlayerResource( );
	~C_PlayerResource( );

public:

	// resources interface
	const char *GetTeamName( int iID );
	int GetTeamScore( int iID );
	const Color &GetTeamColor( int iID );

	bool IsConnected( int iID );
	bool IsAlive( int iID );
	bool IsFakePlayer( int iID );
	bool IsLocalPlayer( int iID );
	bool IsHLTV( int iID );

	const char *GetPlayerName( int iID );
	inline int GetPlayerScore( int iID ) { return GetMorale( iID ); }
	int GetPing( int iID );
	int GetDeaths( int iID );
	int GetFrags( int iID );
	int GetTeam( int iID );
	inline int GetHealth( int iID ) { return 0; }

	// extended
	int GetDeveloper( int iID );
	inline int GetTeamID( int iID ) { return GetTeam( iID ); }
	int GetMorale( int iID );

	// helpers
	bool GetSquadData( int iID, SquadData_t &SquadData );
	int GetSquadID( int iID );
	int GetSlotID( int iID );

private:
	bool IsValidPlayerID( int iID );

protected:
	char m_szName[ MAX_PLAYERS + 1 ][ MAX_PLAYER_NAME_LENGTH ];

	bool m_bConnected[ MAX_PLAYERS + 1 ];
	int m_iPing[ MAX_PLAYERS + 1 ];
	int m_iDeveloper[ MAX_PLAYERS + 1 ];
	int m_iTeamID[ MAX_PLAYERS + 1 ];
	bool m_bAlive[ MAX_PLAYERS + 1 ];

	int m_iMorale[ MAX_PLAYERS + 1 ];
	int m_iKills[ MAX_PLAYERS + 1 ];
	int m_iDeaths[ MAX_PLAYERS + 1 ];
};

//=========================================================
//=========================================================
extern bool ValidPlayerResource( void );
extern C_PlayerResource *PlayerResource( void );

#endif // C_PLAYERRESOURCE_H
