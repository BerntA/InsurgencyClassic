//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team Management Class. Contains all the Details for a Specific Team
//
// $NoKeywords: $
//=============================================================================//

#ifndef TEAM_H
#define TEAM_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class CINSPlayer;

//=========================================================
//=========================================================
class CTeam : public CBaseEntity
{
public:
	DECLARE_CLASS( CTeam, CBaseEntity );
	DECLARE_SERVERCLASS( );

	CTeam( );
	virtual ~CTeam( );

	int UpdateTransmitState( void );

	int GetTeamID( void ) const { return m_iTeamID; }

	virtual const char *GetName( void ) const = 0;
	virtual const char *GetModel( void ) const = 0;

	virtual void AddPlayer( CINSPlayer *pPlayer );
	virtual void RemovePlayer( CINSPlayer *pPlayer );

	int GetNumPlayers( void ) const;
	CINSPlayer *GetPlayer( int iID ) const;

	virtual int PlayerSpawnType( void ) const = 0;

	const CUtlVector<CINSPlayer*>& GetPlayerList(void) const { return m_Players; }

protected:
	void Init( int iTeamID );

protected:
	CNetworkVar( int, m_iTeamID );

	CUtlVector< CINSPlayer* > m_Players;
};

//=========================================================
//=========================================================
void AddTeamRecipients( CTeam *pTeam, CSendProxyRecipients *pRecipients );

#endif // TEAM_H
