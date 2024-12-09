//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_OBJ_H
#define INS_OBJ_H
#ifdef _WIN32
#pragma once
#endif

#include <color.h>
#include "team.h"
#include "team_spawnpoint.h"
#include <igameevents.h>
#include <igamesystem.h>
#include "imc_format.h"
#include "ins_touch.h"

//=========================================================
//=========================================================
class CINSObjective;
class CINSObjMarker; 
class CIMCObjective;
class CSpawnGroup;
class CPlayTeam;
class CINSPlayer;
class CINSWeaponCache;
class CWeaponCacheRandomData;

//=========================================================
//=========================================================
#define OBJ_OUTSIDECAPTIME 2.5f
#define OBJ_IDBITS 5

//=========================================================
//=========================================================
class CSpawnGroupHandler
{
public:
	virtual bool IsValid( CINSObjective *pParent ) = 0;

	virtual CSpawnGroup *GetSpawnGroup( int iTeamID, int iSquadID ) = 0;

    virtual void Print( void ) = 0;
	virtual void DrawSpawnDebug( const Color &ObjColor ) = 0;

protected:
	void PrintGroup( CSpawnGroup *pGroup, int iTeamID, int iSquadID );

protected:
	void DrawSpawnGroup( CSpawnGroup &SpawnGroup, const Color &ObjColor );
};

//=========================================================
//=========================================================
class CSpawnHandlerTeamSquads : public CSpawnGroupHandler
{
public:
	bool IsValid( CINSObjective *pParent );

	CSpawnGroup *GetSpawnGroup( int iTeamID, int iSquadID );

	void Print( void );
	void DrawSpawnDebug( const Color &ObjColor );

private:
	CSpawnGroup m_SpawnPoints[ MAX_PLAY_TEAMS ][ MAX_SQUADS ];
};

class CSpawnHandlerTeam : public CSpawnGroupHandler
{
public:
	bool IsValid( CINSObjective *pParent );

	CSpawnGroup *GetSpawnGroup( int iTeamID, int iSquadID );

	void Print( void );
	void DrawSpawnDebug( const Color &ObjColor );

private:
	CSpawnGroup m_SpawnPoints[ MAX_PLAY_TEAMS ];
};

class CSpawnHandlerMixedSquads : public CSpawnGroupHandler
{
public:
	bool IsValid( CINSObjective *pParent );
	
	CSpawnGroup *GetSpawnGroup( int iTeamID, int iSquadID );

	void Print( void );
	void DrawSpawnDebug( const Color &ObjColor );

private:
	CSpawnGroup m_SpawnPoints[ MAX_SQUADS ];
};

class CSpawnHandlerMixed : public CSpawnGroupHandler
{
public:
	bool IsValid( CINSObjective *pParent );
	
	CSpawnGroup *GetSpawnGroup( int iTeamID, int iSquadID );

	void Print( void );
	void DrawSpawnDebug( const Color &ObjColor );

private:
	CSpawnGroup m_SpawnPoints;
};

//=========================================================
//=========================================================
class CObjSpawns
{
public:
	CObjSpawns( );
	~CObjSpawns( );

	void Init( CINSObjective *pParent );

	bool IsSetup( void ) const;
	void Setup( CSpawnPoint *pSpawnPoint );

	bool IsValid( void ) const;

	CSpawnGroupHandler *GetSpawnHandler( void ) const { return m_pHandler; }

private:
	CINSObjective *m_pParent;
	CSpawnGroupHandler *m_pHandler;
};

//=========================================================
//=========================================================
class CINSObjective : public CINSTouch
{
	typedef void ( CINSObjective::*ObjTouchCall_t )( CINSPlayer* );

	struct PausedCaptureData_t {
		bool m_bPaused;
		int m_iProgress;
	};

public:
	DECLARE_CLASS( CINSObjective, CINSTouch );
	DECLARE_DATADESC( );
	DECLARE_SERVERCLASS( );

	CINSObjective( );

	// Entity Basics
	int UpdateTransmitState( void );

	virtual void Init( void );
	void Reset( void );
	void FinalSetup( void );

	// Capture Management
	bool IsCapturable( void ) const;

	bool IsCaptured( void ) const;

	int GetCapturedTeam( void ) const { return m_iCapturedTeam; }

	// Timed Capture Management

	// ... Player Management
	void PlayerExit( CINSPlayer *pPlayer );

	static void UpdateAllRequiredPlayers( void );
	int GetRequiredPlayers( int iTeamID );
	int GetRequiredPlayers( void );

	int CountCapturePlayers( void );

	const CUtlVector< CINSPlayer* > &GetCapturePlayers( void ) const { return m_CapturePlayers; }
	const CUtlVector< CINSPlayer* > &GetOutsideCapturePlayers( void ) const { return m_OutsideCapturePlayers; }
	const CUtlVector< CINSPlayer* > &GetEnemyPlayers( void ) const { return m_EnemyPlayers; }

	// ... Capturing
	void CaptureStepThink( void );
	int GetCaptureTeam( void ) const { return m_iCaptureTeam; }
	bool IsCapturing( void ) const;
	int GetCaptureProgress( void ) const { return m_iCaptureProgress; }
	bool CheckTimedCapture( void );

	bool IsCapturePaused( void ) const;
	int GetPausedCaptureProgress( void ) const;

	// General
	static void SetupObjectiveMarkers( void );
	void SetMarker( const Vector &MarkerOrigin );
	bool HasMarker( void ) const;

	// Spawn Management
	static bool FindSpawnPoints( void );

	void AddSpawn( CSpawnPoint *pSpawnPoint, int iID );
	CObjSpawns &GetObjSpawn( bool bReinforcementWave );
	int GetSpawnGroup( bool bReinforcement ) const;
	CSpawnGroup *GetSpawnGroup( bool bReinforcementWave, int iTeamID, int iSquadID );
	CSpawnGroup* GetSpawnGroupPlayer( CINSPlayer *pPlayer );

	CObjSpawns &GetMainSpawns( void ) { return m_MainSpawns; }
	CObjSpawns &GetReinforcementSpawns( void ) { return m_ReinforcementSpawns; }

	bool UseReinforcementSpawns( void ) const;

	bool CheckSpawns( void );
	bool CheckSpawns( CObjSpawns &ObjSpawns );
	bool CheckSpawnGroup( int iTeamID, int iSquadID );

	void PrintSpawnStatus( void );
	void PrintSpawnStatus( const char *pszName, CObjSpawns &ObjSpawns );

	// Weapon Caches
	void AddWeaponCache( CINSWeaponCache *pWeaponCache );
	void CreateWeaponCaches( void );
	static void CreateWeaponCaches( CINSObjective *pObjective, CUtlVector< CINSWeaponCache* > &WeaponCaches, const CWeaponCacheRandomData *pWeaponCacheRandomData );
	void CacheDestroyed( CINSWeaponCache *pWeaponCache, CINSPlayer *pPlayer );
	void SetCacheCount( int iCount ) { m_iCacheCount = iCount; }

	// General
	bool IsValid(void) const;

	void SetOrderID( int iOrderID );

	void SetCutUp( bool bState );
	bool IsCutUp(void) const { return m_bCutUp; }

	void SetOrdersAllowed( bool bState );
	bool IsOrdersAllowed( void ) const { return m_bOrdersAllowed; }

	const char *GetTitle( void ) const;

	bool TouchSetupSpawn( void ) const { return false; }

	// IMC Access
	bool HasParent( void ) const;
	const char *GetName( void ) const;
	int GetID(void) const { return m_iID; }
	int GetOrderID(void) const { return m_iOrderID; }
	int GetPhonetic(void) const;
	const Color &GetColor(void) const;
	bool IsStartSpawn(void) const;
	bool IsHidden(void) const;
	int GetInitialTeam(void) const;
	float GetCapTime( void ) const;
	bool IsMixedSpawns( void ) const;
	int GetInvincibilityTime( void ) const;

#ifdef TESTING

	// Misc
	static void DumpLog( void );
	void WriteList( const CUtlVector< CINSPlayer* > &PlayerList, char *pszBuffer, int iBufferLength );

#endif

private:

	// Capture Management
	bool CaptureAllowed( int iTeamID ) const;

	void Captured( int iTeamID, bool bAwardPoints );

	void InputCapture( inputdata_t &inputdata );

	// Timed Capture Management

	// ... Player Management
	void TouchHook( CBaseEntity *pOther, INSTouchCall_t TouchCall );
	void PlayerStartTouch( CINSPlayer *pPlayer );
	void PlayerEndTouch( CINSPlayer *pPlayer );
	void PlayerLeft( CINSPlayer *pPlayer, bool bFinishTouch );
	void PlayerRemoved( void );

	void PlayerAdd( CINSPlayer *pPlayer, CUtlVector<CINSPlayer*> &PlayerList );
	void PlayerRemove( CINSPlayer *pPlayer, CUtlVector<CINSPlayer*> &PlayerList );

	inline bool HasCapturePlayers( void ) { return ( !IsCapturePlayersEmpty( ) || !IsOutsideCapturePlayersEmpty( ) ); }
	inline bool IsCapturePlayersEmpty( void ) { return IsPlayersEmpty( m_CapturePlayers ); }
	inline bool IsEnemyPlayersEmpty( void ) { return IsPlayersEmpty( m_EnemyPlayers ); }
	inline bool IsOutsideCapturePlayersEmpty( void ) { return IsPlayersEmpty( m_OutsideCapturePlayers ); }
	bool IsPlayersEmpty( const CUtlVector< CINSPlayer* > &PlayerList ) const;

	bool CanPlayerCapture( CINSPlayer *pPlayer );

	void PlayerAwardPoints( CINSPlayer *pPlayer, bool bFullCapture );

	void UpdateRequiredPlayers( void );

	bool EnoughPlayersForCapture( void );

	// ... Capturing
	void SetNextStepThink( void );
	bool IsTimedCaptureAllowed( void );
	void StartTimedCapture( void );
	void AttemptTimedCapture( void );
	void StopTimedCapture( void );
	void UpdateCaptureSpeed( void );

	void ResetPausedCapture( void );
	void StartPausedCapture( int iValue );
	void StopPausedCapture( void );
	PausedCaptureData_t *GetPausedCapture( void );
	const PausedCaptureData_t *GetPausedCapture( void ) const;

	// General
	void SendObjectiveUpdate( IRecipientFilter& filter, int iMsgType );

	// Weapon Caches
	bool HasWeaponCaches( void ) const;
	void SetupWeaponCaches( void );

	// IMC Access
	CIMCObjective *GetParent( ) const;

	bool IsKOTH( void ) const;
	int GetReqPercent( void ) const;
	bool UsingReinforcementSpawns( void ) const;

	// Misc
	bool HideBrush( void ) const;

protected:
	CIMCObjective *m_pParent;

	int m_iID;
	CNetworkVar( int, m_iOrderID );

	CNetworkVector( m_vecOrigin );
	CNetworkVar( int, m_iChaseRadius );

	// Capture Management
	CNetworkVar( int, m_iCapturedTeam );

	COutputInt m_OnCapture;

	// Timed Capture Management

	// ... Player Management
	CNetworkVar( int, m_iCaptureTeam );
	int m_iRequiredPlayers[ MAX_PLAY_TEAMS ];

	CUtlVector< CINSPlayer* > m_CapturePlayers, m_OutsideCapturePlayers, m_EnemyPlayers;

	// ... Capturing
	int m_iCaptureProgress;
	float m_flCaptureProgressStep;

	PausedCaptureData_t m_CapturePaused[ MAX_PLAY_TEAMS ];

	// Spawn Management
	CObjSpawns m_MainSpawns, m_ReinforcementSpawns;

	// General
	bool m_bCutUp;

	CNetworkVar( bool, m_bOrdersAllowed );

	// Weapon Cache 
	CUtlVector< CINSWeaponCache* > m_WeaponCaches;
	CWeaponCacheRandomData *m_pWeaponCacheRandomData;
	int m_iCacheCount;

	// Misc
	bool m_bShowBrush;
};

//=========================================================
//=========================================================
class CObjCapturingRecipientFilter : public CRecipientFilter
{
public:
	CObjCapturingRecipientFilter(CUtlVector<CINSPlayer*> &CapturingPlayers);
};

#endif // INS_GAMERULES_OBJ_H
