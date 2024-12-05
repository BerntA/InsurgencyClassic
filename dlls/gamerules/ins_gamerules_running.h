//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_GAMERULES_RUNNING_H
#define INS_GAMERULES_RUNNING_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
enum RoundTimerType_t
{
	ROUNDTIMER_NONE = 0,
	ROUNDTIMER_VALUE,
	ROUNDTIMER_IMC
};

//=========================================================
//=========================================================
class CRunningMode : public CModeBase
{
public:
	int Init( void );
	void Shutdown( void );

	bool ShouldThink( void ) const { return true; }
	int Think( void );

	bool AllowBotAdd( void ) const { return true; }

	void PlayerSpawn( CINSPlayer *pPlayer );

	bool AllowGameEnd( void ) const;
	bool ShouldGameEnd( void ) const;

	int GetDeadCamMode( void ) const;
	int GetDeadCamTargets( void ) const;

	void SetCurrentSpawns( int iT1Spawn, int iT2Spawn );
	void GetCurrentSpawns( int &T1Spawn, int &iT2Spawn );

	void UpdateWarmupInfo( void );
	void StartWarmingup( void );

	int CheckRoundReset( void );
	bool RequestReset( int iType, float flResetTime, bool bRequested );
	void EchoGameReset( CINSPlayer *pPlayer, int iTime );
	void RepeatGameReset( CINSPlayer *pPlayer );
	int ExecuteRoundReset( void );
	void EndGameFinished( void );

	bool CanWinRound( void );
	void RoundWon( int iTeam, int iWinType, CBaseEntity *pWinningObject = NULL, bool bVerbose = false );
	void CheckDeathRoundWin( CPlayTeam *pTeam, CBasePlayer *pAttacker = NULL );
	static bool IsValidWinType( int iWinType );

	inline void UpdateWarmupStartTime(void) { m_flWarmupStartTime = gpGlobals->curtime; }
	inline void SetWarmupLength(int iWarmupLength) { m_iWarmupLength = iWarmupLength; }

	void AddStatus(int iStatus);
	void RemoveStatus(int iStatus);
	inline void ClearStatus(void);
	inline void ResetCurrentObjs(void);
	inline void SetRoundTimerType(int iRoundTimerType) { m_iRoundTimerType = iRoundTimerType; }
	inline void SetRoundLength(int iRoundLength) { m_iRoundLength = iRoundLength; }
	void RoundExtended( void );
	void CalculateEndValues( int iWinningTeam, int iWinType );

	inline int GetStatus(void) const { return m_iStatus; }
	inline bool GetStatus(int iStatus) const { return ((m_iStatus & iStatus) != 0); }
	bool IsScoringAllowed(void) const;

	bool IsWarmingup(void) const;
	inline int GetWarmupType(void) const { return m_iWarmupType; }
	inline float GetWarmupStartTime(void) const { return m_flWarmupStartTime; }
	inline int GetWarmupLength(void) const { return m_iWarmupLength; }

	inline int GetRoundTimerType(void) const { return m_iRoundTimerType; }
	inline bool IsRoundTimed(void) const { return GetRoundTimerType() != ROUNDTIMER_NONE; }
	inline float GetRoundStartTime(void) const { return m_flRoundStartTime; }
	inline int GetRoundLength(void) const { return m_iRoundLength; }
	inline bool IsRoundExtended(void) const { return m_bRoundExtended; }

	void SetupEndGame(int iWinningTeam, int iWinType);
	KeyValues *GetEndGameValues(void) { return m_pEndGameValues; }
	void ShowEndDialog(void);
	void HideEndDialog(void);

	void UpdateClanStatus(CPlayTeam *pTeam);
	bool IsValidClanWarmupType(void) const;

	bool IsOtherTeamMasked( void ) const { return m_bMaskOtherTeam; }
	bool IsScoreFrozen(void) const { return m_bScoreFrozen; }
	bool IsDeathInfoFull(void) const { return m_bDeathInfoFull; }

	void HandlePlayerCount( void );

	void UpdateConsoleValues( void );

	bool IsRestarting( void ) const;
	bool IsEnding( void ) const;
	bool IsWaitingForPlayers( void ) const;

	bool ObjectivesEnabled( void ) const;

	void PlayerJoinedPlayTeam( CINSPlayer *pPlayer, bool bSlain );
	void PlayerJoinedSquad( CINSPlayer *pPlayer, bool bFirstSquadChange );

	bool DetonationsAllowed( void ) const { return m_bDetonationsAllowed; }

	bool PlayerViewpointSpawn( void ) const { return false; }
	void PlayerJoinedTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam ) { }
	void PlayerLeftTeam( CINSPlayer *pPlayer, CPlayTeam *pTeam ) { }

	bool IsFrozen(void) const;
	void SetFrozenState( bool bState );
	int GetFrozenTime( void ) const { return m_iFreezeTimeLength; }

	void SetupRagdollFadeTime( void );
	void RagdollSpawn( CINSRagdoll *pPlayer );
	void EntitiesCleaned( void );

private:
	bool m_bObjectivesEnabled;

	int m_iStatus;

	bool m_bFirstRestart;
	int m_iRestartType;
	float m_flRestartTimeMarker;

	int m_iT1CurrentSpawn, m_iT2CurrentSpawn;

	int m_iWarmupType;
	int m_iWarmupLength;
	float m_flWarmupStartTime;
	bool m_bReadyWaiting;

	int m_iRoundTimerType;
	float m_flRoundStartTime;
	int m_iRoundLength;
	bool m_bRoundExtended;

	int m_iFreezeTimeLength;
	float m_flFreezeTime;

	bool m_bAllowEndGame;
	bool m_bWantEndGame;
	float m_flEndGameTime;
	KeyValues *m_pEndGameValues;

	int m_iDeadCamModes, m_iDeadCamTargets;
	bool m_bMaskOtherTeam;
	bool m_bScoreFrozen;
	bool m_bDeathInfoFull;
	bool m_bDetonationsAllowed;

	float m_flDefaultOrderRollout;

	CUtlVector< EHANDLE > m_RagdollFadeList;
	float m_flRagdollFadeTime;
};

#endif // INS_GAMERULES_RUNNING_H
