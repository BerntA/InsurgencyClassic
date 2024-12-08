//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_GAMERULES_CLIENT_H
#define INS_GAMERULES_CLIENT_H
#ifdef _WIN32
#pragma once
#endif

#include <igameevents.h>
#include "materialsystem/icolorcorrection.h"

//=========================================================
//=========================================================
class C_INSRules;
class C_RunningMode;
class C_SquadMode;
class C_INSObjective;
class C_INSPlayer;

//=========================================================
//=========================================================
#define CModeBase C_ModeBase

class C_ModeBase
{
public:
	virtual void Init( void ) = 0;
};

//=========================================================
//=========================================================
#define CINSRules C_INSRules

class C_INSRules : public C_GameRules, public IGameEventListener2
{
	DECLARE_CLASS( C_INSRules, CGameRules );
	DECLARE_CLIENTCLASS_NOBASE( );

public:
	C_INSRules( );
	virtual ~C_INSRules( );

	// General
	virtual int GetGameType( void ) const = 0;

	void LevelInitPreEntity( void );
	void LevelShutdownPreEntity( void );

	void Precache( void );

	bool ShouldCollide( int collisionGroup0, int collisionGroup1 );

	// Stats
	void SetStatsType( int iType ) { m_iStatsType = iType; }
	int GetStatsType( void ) const { return m_iStatsType; }

	// Objective Management
	virtual bool ObjectiveCaptureAllowed( const C_INSObjective *pObjective, int iTeamID );

	// Mode Management
	void SetMode( int iMode );

	int GetMode( void ) const;
	bool IsModeIdle( void ) const;
	bool IsModeRunning( void ) const;
	bool IsModeSquad( void ) const;

	C_ModeBase *CurrentMode( void ) const;
	C_RunningMode *RunningMode( void ) const;
	C_SquadMode *SquadMode( void ) const;

	C_RunningMode *RunningModeUpdate( void ) const;
	C_SquadMode *SquadModeUpdate( void ) const;

	// Round Management
	void RoundReset( void );

	bool IsRoundCold( void ) const;

	// Player Management
	void PlayerThink( CBasePlayer *pPlayer );

	bool FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker );
	bool PlayerAdjustDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker, CTakeDamageInfo &info );
	int PlayerRelationship( CBasePlayer *pPlayer, CBaseEntity *pTarget );
	int TeamRelationship( CBasePlayer *pPlayer, int iTeamID );

	bool CanChangeTeam( C_INSPlayer *pPlayer );
	bool CanChangeSquad( C_INSPlayer *pPlayer );

	int DefaultFOV( void ) const;
	int DefaultWeaponFOV( void ) const;

	int GetProneForward( void ) const;

	// Viewpoint Management
	void SetCurrentViewpointOrigin( float flX, float flY, float flZ );
	void SetCurrentViewpointAngles( float flX, float flY, float flZ );

	const Vector& GetCurrentViewpointOrigin( void ) const { return m_vecCurrentViewpoint; }
	const QAngle& GetCurrentViewpointAngle( void ) const { return m_angCurrentViewpoint; }

	// Team Management
	const Color &TeamColor( C_Team *pTeam ) const;
	const Color &TeamColor( C_INSPlayer *pPlayer ) const;
	const Color &TeamColor( int iTeamID ) const;

	static Color CalculateTeamColor( int iType, bool bBackup );

	// Misc
	void SetScriptsCRC32( unsigned int iCRC32 );

	bool AllowAttacking( void );

	int GetDeadCamMode( void ) const;
	int GetDeadCamTargets( void ) const;

	const ColorCorrectionHandle_t &GetColorCorrection( void ) const { return m_ColorCorrection; }

protected:
	void FireGameEvent( IGameEvent *pEvent );

private:
	int m_iStatsType;

	CUtlVector< C_INSObjective* > m_Objectives;

	int m_iCurrentMode;
	C_ModeBase *m_pModes[ GRMODE_COUNT ];

	Vector m_vecCurrentViewpoint;
	QAngle m_angCurrentViewpoint;

	unsigned int m_iScriptsCRC32;

	ColorCorrectionHandle_t m_ColorCorrection;
};

//=========================================================
//=========================================================
inline C_INSRules *INSRules( void )
{
	return static_cast< C_INSRules* >( g_pGameRules );
}

//=========================================================
//=========================================================
#define CIdleMode C_IdleMode

class C_IdleMode : public C_ModeBase
{
	void Init( void ) { }
};

//=========================================================
//=========================================================
#define CSquadMode C_SquadMode

class C_SquadMode : public C_ModeBase
{
public:
	void Init( void );

	void SetOrderLength( int iTeamID, int iSize );
	void SetOrder(int iTeamID, int iElement, int iValue);

	int GetPlayerCount( void ) const;
	int GetPlayer( int iPlayerID ) const;

private:
	bool IsValidData( int iTeamID ) const;

private:
	CUtlVector< int > m_PlayerOrder;
};

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CRunningMode C_RunningMode

#endif

class C_RunningMode : public C_ModeBase
{
public:
	void Init( void ) { }

	void SetStatus(int iStatus);
	void SetRoundStartTime(float flRoundStartTime);
	void SetRoundLength(int iRoundLength);
	void SetRoundExtended(bool bState);
	void SetFrozenTimeLength(int iLength);
	void SetDeathInfoFull(bool bState);

	float GetRoundStartTime(void) const;
	int GetRoundLength(void) const;
	bool GetStatus(int iStatus) const;
	bool IsRoundExtended(void) const;
	bool IsDeathInfoFull(void) const;

	bool IsWarmingup( void ) const;
	bool IsRestarting( void ) const;
	bool IsEnding( void ) const;
	bool IsWaitingForPlayers( void ) const;

	bool IsFrozen( void ) const;
	int GetFrozenTimeLength( void ) const;

	inline bool IsScoringAllowed(void) const;
	float GetEndTime(void) const;

	void SetDeadCamMode( int iMode ) { m_iDeadCamModes = iMode; }
	void SetDeadCamTargets( int iTargets ) { m_iDeadCamTargets = iTargets; }
	int GetDeadCamMode( void ) const { return m_iDeadCamModes; }
	int GetDeadCamTargets( void ) const { return m_iDeadCamTargets; }

private:
	int m_iStatus;
	float m_flRoundStartTime;
	int m_iRoundLength;
	bool m_bRoundExtended;
	int m_iFrozenTimeLength;
	bool m_bDeathInfoFull;
	int m_iDeadCamModes, m_iDeadCamTargets;
};

//=========================================================
//=========================================================
#define DECLARE_GAMETYPE( gametype, className )

//=========================================================
//=========================================================
extern char *g_pszGameDescriptions[ MAX_GAMETYPES ];

#endif // INS_GAMERULES_CLIENT_H
