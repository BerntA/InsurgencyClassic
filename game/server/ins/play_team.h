//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef PLAY_TEAM_H
#define PLAY_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "team_shared.h"
#include "squad_data.h"
#include "imc_format.h"
#include "utlmap.h"

//=========================================================
//=========================================================
class CINSPlayer;
class CTeamLookup;
class CIMCTeamConfig;
class CINSSquad;

//=========================================================
//=========================================================
enum PlayerRespawnType_t
{
	PRESPAWNTYPE_REINFORCEMENT = 0,		// reinforcement respawn
	PRESPAWNTYPE_RESTART,				// respawn everybody
	PRESPAWNTYPE_COUNT
};

//=========================================================
//=========================================================
class CPositionList
{
public:
	typedef CUtlVector< SquadData_t > ClassPositions_t;

public:
	CPositionList( );
	CPositionList( const CPositionList &PositionList );

	void Add( int iSquadID, int iSlotID );
	ClassPositions_t &Get( void ) { return m_Positions; }

private:
	ClassPositions_t m_Positions;
};

//=========================================================
//=========================================================
class CClassPositionList
{
protected:
	typedef CUtlMap< int, CPositionList > ClassList_t;

public:
	CClassPositionList( );
	CClassPositionList( const CClassPositionList & );

	void Add( int iPlayerClass, int iSquadID, int iSlotID );

protected:
	void Init( const CClassPositionList &SourceList );

protected:
	ClassList_t m_Classes;
};

//=========================================================
//=========================================================
class CPlayTeam : public CTeam
{
	DECLARE_CLASS( CPlayTeam, CTeam );
	DECLARE_SERVERCLASS( );
	DECLARE_DATADESC( );

public:
	CPlayTeam( );

	static void Create( CIMCTeamConfig *pIMCTeamConfig );

	void Spawn( void );
	void Precache( void );

	static void LevelInit( void );

	void ResourceThink( void );

	const char *GetName( void ) const;
	const char *GetModel( void ) const;

	void AddPlayer( CINSPlayer *pPlayer );
	void RemovePlayer( CINSPlayer *pPlayer );

	CIMCTeamConfig *GetIMCTeamConfig( void ) const;
	CTeamLookup *GetTeamLookup( void ) const;
	int GetTeamLookupID(void);

	// Squad Management

	// ... access
	const CUtlVector< CINSSquad* > &GetSquads( void ) const { return m_Squads; }

	int GetSquadCount( void ) const;
	CINSSquad *GetSquad( int iID ) const;
	CINSSquad *GetSquad( const SquadData_t &SquadData ) const;

	bool IsValidSquad( int iID ) const;
	bool IsValidSquadData( const SquadData_t &SquadData ) const;

	// ... player a management
	bool IsFull( void ) const;

	void AddSquadPlayer( int iSquadID );
	void RemoveSquadPlayer( void );

	void MassSquadRemove( void );

	// misc
	bool GetRandomSquad( SquadData_t &SquadInfo ) const;
	void UpdateSquadArea( bool bFullReset = false );

	// Class Management
	const CClassPositionList &GetClassPositionList( void ) const { return m_ClassPositionList; }

	// Score Management
	void IncrementScore( void );
	int GetScore( void ) const { return m_iScore; }
	int GetTotalPlayerScore( void );
	void ResetScores( void );
	void UpdateRanks( void );
	void UpdatePlayerRanks( void );

	// Player Management
	void PlayerKilled( CINSPlayer *pPlayer );
	bool AreAllPlayersDead( void ) const;

	static int GetTotalPlayerCount( void ) { return m_iTotalPlayerCount; }

	// Reinforcement Management
	void StartReinforcement( int iSquadID );
	void CheckReinforcements( void );
	int ReinforcementsLeft( int iSquadID ) const { return m_iReinforcementsLeft[ iSquadID ]; }
	int AverageReinforcementsLeft( void );
	bool HasReinforcementLeft( void ) const;
	void ResetReinforcements( void );
	void ResetReinforcementTime( void );
	bool HadDeployment( void ) const { return m_bHadDeployment; }
	bool HadReinforcement( void ) const { return m_bHadReinforcement; }
	bool IsDeploying( void ) const { return m_bDeploying; }
	bool IsUnlimitedWaves( void ) const;
	bool HasReinforcementStarted( void ) const;
	bool CanEmergencyReinforce( int iSquadID ) const;
	int GetReinforcementTime( void ) const;
	int GetReinforcementTime( bool bEmergency ) const;

	void DoReinforcement( int iType, int iSquadID = INVALID_SQUAD, bool bProtectReinforcements = false );
	int DoEmergencyReinforcement( int iSquadID );

#ifdef _DEBUG

	void ForceRespawnPlayers( void );
	void DisableRespawn( void );

#endif

	// Round Management
	void RoundReset( void );
	void RoundUnfrozen( void );

	// Objective Management
	void CapturedObjective( void );

	// Misc
	bool IsValidForRespawn( int iType, CINSPlayer *pPlayer );
	int GetNumWaves( void ) const;
	int GetType( void ) const;
	int GetEmergencyWaves( void ) const;
	const Color &GetColor( void ) const;

#ifdef TESTING

	// Squad Debug
	void PrintSquads( void );

#endif

	// Clan Leader Management
	void RegisterClanLeader( CINSPlayer *pPlayer );
	bool HasClanLeader( void ) const;
	CINSPlayer *GetClanLeader( void ) const;
	void SetClanReady( bool bState );
	bool IsClanReady( void ) const { return m_bClanReady; }

	// Objective Management
	static int GetObjectiveCaptures( void ) { return m_iObjectiveCaptures; }
	static void ObjectiveCaptured( void );

private:

	void Init( CIMCTeamConfig *pTeamConfig );

	// Squad Management
	int GetAreaCount( void ) const;
	int GetAreaQuota( void ) const;
	void ModifySquadArea( int iAreaDiff, bool bOpeningUp );

	// Player Management
	int PlayerSpawnType( void ) const;

	// Reinforcement Management
	bool IsRedeploymentAllowed( void ) const;
	bool HasRedeployments( int iSquadID = INVALID_SQUAD ) const;
	void StartReinforcement( int iSquadID, bool bEmergency );
	float EndReinforcementTime( void ) const;
	void SendReinforcementMsg( CUtlVector< CINSPlayer* > &PlayersSpawned );

private:
	CIMCTeamConfig *m_pIMCTeamConfig;
	CUtlVector< CINSSquad* > m_Squads;

	// Score Management
	CNetworkVar( int, m_iScore );

	// Squad Management
	int m_iEnabledArea;
	int m_iFreeSlots;

	// Class Management
	CClassPositionList m_ClassPositionList;

	// Reinforcement Managerment
	bool m_bHadDeployment, m_bHadReinforcement;

	bool m_bDeploying;

	CNetworkVar( float, m_flStartReinforcementTime );
	CNetworkVar( bool, m_bEmergencyReinforcement );
	float m_flLastReinforcementTime;

	CNetworkArray( int, m_iReinforcementsLeft, MAX_SQUADS );

	CNetworkArray( int, m_iEmergencyReinforcementsLeft, MAX_SQUADS );
	float m_flNextEmergencyReinforcement;

	CNetworkArray( int, m_iSquadData, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iHealthType, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iRank, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iOrder, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iStatusType, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iStatusID, MAX_PLAYERS + 1 );
	CNetworkArray( bool, m_bNeedsHelp, MAX_PLAYERS + 1 );

#ifdef _DEBUG

	bool m_bDisabledSpawn;

#endif

	// Clan Leader Management
	EHANDLE m_ClanLeader;
	bool m_bClanReady;

	// Misc
	static int m_iTotalPlayerCount;
	static int m_iObjectiveCaptures;
};

#endif // PLAY_TEAM_H
