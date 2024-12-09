//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_GAMERULES_SERVER_H
#define INS_GAMERULES_SERVER_H
#ifdef _WIN32
#pragma once
#endif

#include "mapentities.h"
#include "utlsortvector.h"
#include "color.h"
#include "commander_shared.h"
#include "squad_data.h"
#include "weapon_defines.h"
#include "ins_player_shared.h"

//=========================================================
//=========================================================
class CINSRules;
class CIdleMode;
class CSquadMode;
class CRunningMode;
class CPlayTeam;
class CSpawnGroup;
class CObjSpawns;
class CINSWeaponCache;
class CINSRagdoll;

//=========================================================
//=========================================================
enum T1Orients_t
{
	T1ORIENTATION_INVALID = -1,
	T1ORIENTATION_LOW = 0,
	T1ORIENTATION_HIGH
};

//=========================================================
//=========================================================
class CModeBase
{
public:
	virtual int Init( void ) = 0;
	virtual void Shutdown( void ) = 0;

	virtual bool ShouldThink( void ) const = 0;
	virtual int Think( void ) = 0;

	virtual bool AllowBotAdd( void ) const = 0;

	virtual void HandlePlayerCount( void ) = 0;
	virtual bool PlayerViewpointSpawn( void ) const = 0;
	virtual void PlayerSpawn( CINSPlayer *pPlayer ) = 0;
	virtual void PlayerJoinedTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam ) = 0;
	virtual void PlayerLeftTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam ) = 0;
	virtual void PlayerJoinedSquad( CINSPlayer *pPlayer, bool bFirstSquadChange ) = 0;

	virtual void RagdollSpawn( CINSRagdoll *pPlayer ) = 0;

	virtual void EntitiesCleaned( void ) = 0;

	virtual bool AllowGameEnd( void ) const = 0;
	virtual bool ShouldGameEnd( void ) const = 0;
};

//=========================================================
//=========================================================
class CINSMapEntityFilter : public IMapEntityFilter
{
public:
	CINSMapEntityFilter( );

	void Setup( void );

private:
	bool ShouldCreateEntity( const char *pszClassname );
	CBaseEntity* CreateNextEntity( const char *pszClassname );

private:
	int m_iIterator;
};

//=========================================================
//=========================================================
class CObjectiveIterator
{
public:
	CObjectiveIterator( int iTeamID );

	bool End( void ) const;

	void operator++( int );

	CINSObjective *Current( void ) const;
	int CurrentID( void ) const { return m_iCurrentObjective; }

private:
	CObjectiveIterator( const CObjectiveIterator &ObjIterator );

private:
	int m_iTeamID;

	int m_iCurrentObjective, m_iMod;
};

//=========================================================
//=========================================================
class CINSRules : public CGameRules
{
	friend class CIdleMode;
	friend class CSquadMode;
	friend class CRunningMode;

public:
	DECLARE_CLASS( CINSRules, CGameRules );
	DECLARE_SERVERCLASS_NOBASE( );

public:
	CINSRules( );

	// General
	void InitRules( void );

	virtual int GetGameType( void ) const = 0;
	const char *GetGameTypeName( void ) const;

	static const char *GetGameDescription( void );
	static const char *GetGameVersion( void );

	virtual void CreateProxy( void );

	bool ShouldCollide( int collisionGroup0, int collisionGroup1 );

	// Objective Management
	bool IsUsingObjectives( void ) const;

	bool IsValidObjective( int iID ) const;
	CINSObjective *GetObjective( int iID ) const;
	int GetObjectiveCount( void ) const;

	CINSObjective *GetUnorderedObjective( int iID ) const;

	bool ObjectiveCaptureAllowed( void );
	virtual bool ObjectiveCaptureAllowed( const CINSObjective *pObjective, int iTeamID );
	virtual void ObjectiveCaptured( CINSObjective *pObjective );

	virtual int ObjectiveCapturePoints( bool bFollowingOrders ) const { return 0; }
	virtual int ObjectiveLeaderPoints( void ) const { return 0; }

	bool IsValidObjOrder( int iTeamID, CINSObjective *pObjective );

#ifdef TESTING

	const CUtlVector< CINSObjective* > &GetObjectives( void ) const { return m_Objectives; }

#endif

	void GetStartSpawns( int &iT1Spawn, int &iT2Spawn );
	int GetT1ObjOrientation( void ) const { return m_iT1ObjOrientation; }

	// Weaponcache Management
	void RestockWeaponCaches( int iTeamID );

	// Mode Management
	int GetMode( void ) const;
	int GetLastMode( void ) const { return m_iLastMode; }

	bool IsModeIdle( void ) const;
	bool IsModeSquad( void ) const;
	bool IsModeRunning( void ) const;

	CModeBase *CurrentMode( void ) const;
	CRunningMode *RunningMode( void ) const;
	CSquadMode *SquadMode( void ) const;

	// Round Management
	virtual void RoundReset( void );
	virtual bool ForceTimerType( void ) { return false; }
	virtual void RoundUnfrozen( void );

	bool IsRoundCold( void ) const;

	// Team Management
	void SetupPlayerTeam( CINSPlayer *pPlayer, int iNewTeam );
	bool SetupPlayerSquad( CINSPlayer *pPlayer, bool bForce, EncodedSquadData_t *pEncodedSquadData, bool bWhenDie = false );
	bool SetupPlayerSquad( CINSPlayer *pPlayer, const SquadData_t *pSquadData );

	void PlayerJoinedTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam );
	void PlayerLeftTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam );

	bool ValidJoinTeam( int iTeamID );

	void SwapPlayTeams( void );

	bool ShouldBeWaitingForPlayers( void ) const;

	static Color CalculateTeamColor( int iType, bool bBackup );

	int GetTotalTeamScore( void ) const;

	// Squad Management
	void UpdateSquadsArea( void );
	bool SquadsForcedOpen( void ) const { return m_bForcedSquadsOpen; }

	// Player Management
	bool ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );
	bool ClientCommand( const char *pcmd, CBaseEntity *pEdict );
	virtual void ClientSettingsChanged( CBasePlayer *pPlayer );
	void ClientDisconnected( edict_t *pClient );

	void PlayerInitialSpawn( CINSPlayer *pPlayer );

	CBaseEntity *GetPlayerSpawnSpot( CBasePlayer *pPlayer );
	void PlayerSpawn( CBasePlayer *pPlayer );
	void PlayerThink( CBasePlayer *pPlayer );

	int PlayerSpawnType( CINSPlayer *pPlayer );
	bool FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker );
	bool PlayerAdjustDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker, CTakeDamageInfo &info );
	void PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	void CalculateScores( CINSPlayer *pVictim, CINSPlayer *pScorer );
	void PlayedFinishedDeathThink( CINSPlayer *pPlayer );

	bool PlayerCanCommunicate( CBasePlayer *pListener, CBasePlayer *pSpeaker );
	void UpdateClientData( CBasePlayer *pPlayer ) { }
	void PlayerKick( CBasePlayer *pPlayer );
	void PlayerKickBots( bool bCheckLAN );

	bool ArePlayersConnected( void ) const { return m_iConnectedPlayers != 0; }
	inline void PlayerConnected( void ) { PlayerConnectionUpdate( true ); }
	inline void PlayerDisconnected( void ) { PlayerConnectionUpdate( false ); }
	void PlayerConnectionUpdate( bool bConnected );

	bool PlayerLiveScores( void ) const;

	bool CanChangeTeam( CINSPlayer *pPlayer );
	bool CanChangeSquad( CINSPlayer *pPlayer );

	int DefaultFOV( void ) const;
	int DefaultWeaponFOV( void ) const;

	int GetProneForward( void ) const;

	int PlayerCalculateMorale( const CGamePlayerStats &Stats );
	void PlayerTK( CINSPlayer *pPlayer );

	virtual void PlayerAddWeapons( CINSPlayer *pPlayer ) { }

	// Viewpoint Management
	void PlayerViewpointAdd( CINSPlayer *pPlayer );
	void PlayerViewpointRemove( CINSPlayer *pPlayer );

	CBaseEntity *GetViewpoint( void );

	const Vector &GetCurrentViewpointOrigin( void );
	const QAngle &GetCurrentViewpointAngle( void );

	// Ragdoll Management
	void RagdollSpawn( CINSRagdoll *pRagdoll );

	// Error Handling
	static void ResetError( void );
	static void SetError( int IID );
	static void EchoError( const char *pszContext, const char *pszError );

	// Misc
	static bool ChangeLevel( const char *pszMapName, int iProfileID );

	static void LoadMessageOfTheDay( void );

	bool AllowOrderedSquadSelection( void ) const;

	bool ShouldForceDeathFade( void ) const;

	bool DetonationsAllowed( void ) const;

	bool IsClanMode( void ) const { return m_bClanMode; }
	void SetClanMode( bool bState ) { m_bClanMode = bState; }
	void UpdateClanMode( void );

	bool PunishTK( void ) const;

	bool CanSabotageCaches( void ) const;

	void DrawBadSpawns( void );

	bool AllowAttacking( void );

	bool AllowBotAdd( void ) const;

	// Debug
	void DrawSpawnDebug_SpawnGroup(const Color &ObjColor, CSpawnGroup *pSpawnGroup);

protected:

	// General
	virtual void Precache( void );

	// Objective Management
	virtual bool NeedsObjectives( void ) const { return true; }
	virtual bool IsEnoughObjectives( void ) const { return false; }
	virtual int FinalSetup( int &iT1Spawn, int &iT2Spawn, int &iT1Orient ) { return 0; }

	virtual bool CheckForWinner( int &iWinningTeam ) = 0;
	virtual int GetDefaultWinner( void ) const = 0;

	bool HasSpawns( int &iWinningTeam );

	virtual void ResetSpawnPoints( void );
	virtual void UpdateSpawnPoints( CINSObjective *pCapturedObj, int &iT1Spawn, int &iT2Spawn );

	void DefaultOrderRollout( bool bReportInvalid );
	virtual void CalculateDefaultOrders( int iTeamID, CObjOrder &ObjOrders );

	void ObjectiveCapturingUpdate( void );

	int GetFirstObjectiveID( void );
	int GetLastObjectiveID( void );
	CINSObjective *GetFirstObjective( void );
	CINSObjective *GetLastObjective( void );

private:

	// General
	void Think( void );

	void LevelInitPreEntity( void );
	void LevelInitPostEntity( void );
	void LevelShutdown( void );
	void LevelShutdownPreEntity( void );

	// Objective Management
	int SetupObjectives( void );
	void AddObjectives( const char *pszObjectiveClass );
	bool SortObjectives( void );

	void RemoveCollidingSpawns( void );

	// Weaponcache Management
	int SetupWeaponCaches( void );
	void CreateWeaponCaches( void );

	// Mode Management
	void SetMode( int iMode );

	void HandleModeThink( void );

	// Team Management
	void HandlePlayerCount( void );

	bool PlayerConfigUpdate( CINSPlayer *pPlayer );

	int CalculateAutoAssign( void ) const;
	int FindTeamWithLowestPlayers( void ) const;
	void EchoNoScoringMessage( void ) const;

	// Squad Management
	void UpdateSquadOrderType( void );
	int GetSquadOrderType( void ) const { return m_iSquadOrderType; }

	// Player Management
	CBaseEntity *GetObjectiveSpawnPoint( CINSPlayer *pPlayer );
	CBaseEntity *GetLastSpawnOption( CINSPlayer *pPlayer ) const;
	CBaseEntity *GetLastSpawnOption( void ) const;

	int PlayerRelationship( CBasePlayer *pPlayer, CBaseEntity *pTarget );
	int TeamRelationship( CBasePlayer *pPlayer, int iTeamID );

	int CalculateScoreFactor( CINSPlayer *pVictim, int iScorerTeamID );

	// Viewpoint Management
	void SendCurrentViewpoint( CINSPlayer *pPlayer );
	void SendViewpoint( CINSPlayer *pPlayer, int iViewpointID, bool bHacked );
	void UpdateCurrentViewpoint( void );
	void UpdateViewpointRotateTime( void );

	// Error Handling
	void EchoRepeatingError( void );
	static void EchoError( void );
	void EchoSpawnError( CINSPlayer *pPlayer ) const;

	// Changelevel Management
	bool ShouldMapRotate( void );

	// Misc
	void HandleObjectiveDebug( void );

	void ExecServerConfig(void);

	void SetDetonationsAllowed( bool bState );

	void CleanUpEntities(void);

	bool TimeLimitPassed( void ) const;

	int GetDeadCamMode( void ) const;
	int GetDeadCamTargets( void ) const;

	// Debug
	void DrawSpawnDebug_Obj(CINSObjective *pObjective, const Color *pForceColor = NULL);
	void DrawSpawnDebug_ObjSpawns(CINSObjective *pObjective, const Color &ObjColor, CObjSpawns *pObjSpawns);
	void DrawSpawnDebug_Entity(const Color &ObjColor, CBaseEntity *pEntity, bool bSpawn);

private:

	// Server String
	static char m_szServerString[ 64 ];

	// Error Management
	static int m_iError;
	float m_flErrorTime;

	// Objective Management
	bool m_bUseObjs;

	CUtlVector< CINSObjective* > m_Objectives;
	CINSObjective *m_pUnsortedObjectives[ MAX_OBJECTIVES ];

	int m_iStartObjs[ MAX_PLAY_TEAMS ];
	int m_iT1ObjOrientation;

	CUtlVector< CSpawnPoint* > m_BadSpawns;

	// Weaponcache Management
	CUtlVector< CINSWeaponCache* > m_WeaponCaches;
	CUtlVector< CINSWeaponCache* > m_WeaponCachesDeploy;

	// Mode Management
	int m_iCurrentMode, m_iLastMode;
	CModeBase *m_pModes[ GRMODE_COUNT ];

	// Player Management
	int m_iConnectedPlayers;

	// Viewpoint Management
	CUtlVector< CINSPlayer* > m_ViewpointPlayers;
	CNetworkVar( int, m_iCurrentViewpoint );
	float m_flViewpointRotateTime;

	// Misc
	CINSMapEntityFilter m_EntityFilter;
	bool m_bVirginWorld;
	int m_iSquadOrderType;
	bool m_bIgnoreNextDeath;
	bool m_bTimeLimitPassed;
	bool m_bForcedSquadsOpen;
	bool m_bClanMode;
	bool m_bDoneSquadMode;

	bool m_bAwardRoundPoints;

	CBaseEntity *m_pFallbackSpawn;
};

//=========================================================
//=========================================================
extern ConVar suppresskillhint;

//=========================================================
//=========================================================
inline CINSRules* INSRules( void )
{
	return static_cast< CINSRules* >( g_pGameRules );
}

//=========================================================
//=========================================================
extern const char *g_pszGameTypeClassnames[ MAX_GAMETYPES ];

class CGameTypeClassname
{
public:
	CGameTypeClassname( int iGameType, const char *pszClassname )
	{
		g_pszGameTypeClassnames[ iGameType ] = pszClassname;
	}
};

#define DECLARE_GAMETYPE( gametype, className )	\
	CGameTypeClassname g_GameType__##gametype( gametype, #className );

#endif // INS_GAMERULES_SERVER_H
