//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_TEAM_H
#define C_TEAM_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class C_INSPlayer;

//=========================================================
//=========================================================
class C_Team : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_Team, C_BaseEntity );
	DECLARE_CLIENTCLASS( );

public:
	C_Team( );
	~C_Team( );

	static bool ValidTeams( void );

	virtual void OnDataChanged( DataUpdateType_t Type );

	int	GetTeamID( void ) const { return m_iTeamID; }

	virtual const char *GetName( void ) const;

	int GetNumPlayers( void ) const;
	C_INSPlayer *GetPlayer( int iID ) const;
	int GetPlayerID( int iID ) const;

public:
	int	m_iTeamID;

	CUtlVector< int > m_Players;
};

//=========================================================
//=========================================================
extern C_Team *GetPlayersTeam( int iPlayerID );
extern C_Team *GetPlayersTeam( C_INSPlayer *pPlayer );

#endif // C_TEAM_H
