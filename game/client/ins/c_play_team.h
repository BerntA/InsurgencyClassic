//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef C_PLAY_TEAM_H
#define C_PLAY_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "team_shared.h"
#include "team_lookup.h"
#include "squad_data.h"

//=========================================================
//=========================================================
class CIMCTeamConfig;
class CTeamLookup;
class C_INSSquad;

//=========================================================
//=========================================================
class C_PlayTeam : public C_Team
{
public:
	DECLARE_CLASS( C_PlayTeam, C_Team );
	DECLARE_CLIENTCLASS( );

	C_PlayTeam( );

	void Setup( void );

	const char *GetName( void ) const;

	void OnDataChanged( DataUpdateType_t Type );

	CIMCTeamConfig *GetIMCTeamConfig( void ) const;

	void SetTeamLookup( int iTeamLookup );
	int GetTeamLookupID( void ) const { return m_iTeamLookup; }
	CTeamLookup *GetTeamLookup( void ) const;

	static const char *GetNameFromID( int iTeamID );

	// Squad Mamagement
	int GetSquadCount(void) const { return m_Squads.Count(); }
	C_INSSquad *GetSquad(int iID) const;

	// Score Management
	int GetScore( void ) const { return m_iScore; }
	int GetTotalPlayerScore( void );

	// Reinforcement Management
	float GetStartReinforcementTime( void ) const { return m_flStartReinforcementTime; }
	int GetReinforcementsLeft( int iSquadID );
	bool IsUnlimitedWaves( void ) const;
	bool IsEmergencyReinforcement( void ) const { return m_bEmergencyReinforcement; }

	int GetEmergencyReinforcementsLeft( int iSquadID );

	// Misc
	void SetNumWaves( int iNumWaves ) { m_iNumWaves = iNumWaves; }
	int GetNumWaves( void ) const { return m_iNumWaves; }

	void SetTimeWave( int iTimeWave ) { m_iTimeWave = iTimeWave; }
	int GetTimeWave( void ) const { return m_iTimeWave; }

	int GetType( void );

	int CountTotalFreeSlots( void ) const;
	int CountTotalSlots( void ) const;

	int Get_Ping( void ) const { return m_iPing; }
	void UpdatePing( void );

	bool GetSquadData( int iIndex, SquadData_t &SquadData );
	int GetHealthType( int iIndex );
	int GetRank( int iIndex );
	int GetOrder( int iIndex );
	int GetStatusType( int iIndex );
	int GetStatusID( int iIndex );
	int NeedsHelp( int iIndex );

	void SetColor( int iColor );
	const Color &GetColor( void ) const { return m_Color; }

public:
	CUtlVector< int > m_Squads;

private:
	CIMCTeamConfig *m_pIMCTeamConfig;

	CTeamLookup *m_pTeamLookup;
	int m_iTeamLookup;

	Color m_Color;

	int m_iScore;

	float m_flStartReinforcementTime;
	bool m_bEmergencyReinforcement;

	int m_iReinforcementsLeft[ MAX_SQUADS ];

	int m_iEmergencyReinforcementsLeft[ MAX_SQUADS ];

	int m_iNumWaves;
	int m_iTimeWave;

	bool m_bShouldUpdatePing;
	int	m_iPing;

	int m_iSquadData[ MAX_PLAYERS + 1 ];
	int m_iHealthType[ MAX_PLAYERS + 1 ];
	int m_iRank[ MAX_PLAYERS + 1 ];
	int m_iOrder[ MAX_PLAYERS + 1 ];
	int m_iStatusType[ MAX_PLAYERS + 1 ];
	int m_iStatusID[ MAX_PLAYERS + 1 ];
	bool m_bNeedsHelp[ MAX_PLAYERS + 1 ];
};

//=========================================================
//=========================================================
extern C_PlayTeam *GetPlayersPlayTeam( int iPlayerIndex );
extern C_PlayTeam *GetPlayersPlayTeam( C_INSPlayer *pPlayer );

#endif // C_PLAY_TEAM_H
