//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef IMC_CONFIG_H
#define IMC_CONFIG_H
#ifdef _WIN32
#pragma once
#endif

#include "imc_format.h"

#ifdef GAME_DLL 

#include "color.h"
#include "ins_weaponcache_shared.h"

#endif

//=========================================================
//=========================================================
class CIMCData;
class CIMCConfig;
class CTeamLookup;

#ifdef GAME_DLL

class CIMCObjective;
class CINSSquad;

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

struct ProfileDataChunk_t
{
	char m_szProfileName[ MAX_PROFILENAME_LENGTH ];
	int m_iGameModeID;
};

//=========================================================
//=========================================================
class CIMCGlobalObj
{
public:
	void Reset( void );

	void SetID( int iID );
	void SetName( const char *pszName );
	void SetPhonetic( int iPhonetic ) { m_iPhonetic = iPhonetic; }
	void SetColor( const Color &ObjColor ) { m_Color = ObjColor; }

	int GetID( void ) const { return m_iID; }
	const char *GetName( void ) const { return m_szName; }
	int GetPhonetic( void ) const { return m_iPhonetic; }
	const Color &GetColor( void ) const { return m_Color; }

protected:
	int m_iID;
	char m_szName[ MAX_OBJNAME_LENGTH ];
	int m_iPhonetic;
	Color m_Color;
};

//=========================================================
//=========================================================
class CWeaponCacheRandomData
{
public:
	CWeaponCacheRandomData( );

	void Init( bool bUseRandom, float flMax, float flMin );

	bool UseRandom( void ) const { return m_bUseRandom; }
	float GetMax( void ) const { return m_flMax; }
	float GetMin( void ) const { return m_flMin; }

private:
	bool m_bUseRandom;
	float m_flMax, m_flMin;
};

//=========================================================
//=========================================================
class CIMCWeaponCache
{
public:
	CIMCWeaponCache( int iID );

	void SetParentObjID( int iID ) { m_iParentObjID = iID; }
	void SetFlags( int iFlags ) { m_iFlags |= iFlags; }

	int GetID( void ) const { return m_iID; }
	int GetParentObjID( void ) const { return m_iParentObjID; }
	int GetFlags( void ) const { return m_iFlags; }

	const CWeaponCacheBlueprint &GetBlueprint( void ) const { return m_Blueprint; }

private:
	int m_iID;
	int m_iParentObjID;
	int m_iFlags;

	CWeaponCacheBlueprint m_Blueprint;
};

//=========================================================
//=========================================================
class CIMCObjective : public CIMCGlobalObj
{
public:
	CIMCObjective(const CIMCGlobalObj *pGlobalObj);

	bool IsParent(void) { return m_bIsParent; }
	void SetParent(void) { m_bIsParent = true; }

	void SetReqPercent(int iReqPercent);
	void SetCapTime(int iCapTime);
	void SetFlags(int iFlags);
	void SetSpawnGroup(int iSpawnGroup);
	void SetReinforcementSpawnGroup(int iSpawnGroup);
	void SetInvincibilityTime( int iTime );

	void SetWeaponCacheRandomData(float flMax, float flMin);

	int GetInitialTeam( void ) const;
	bool IsStartingSpawn( void ) const;
	bool IsHidden( void ) const;
	int GetReqPercent( void ) { return m_iReqPercent; }
	int GetCapTime( void ) { return m_iCapTime; }
	bool HasReinforcementSpawns( void ) const;
	int GetSpawnGroup( void ) const { return m_iSpawnGroup; }
	int GetReinforcementSpawnGroup( void ) { return m_iReinforcementSpawnGroup; }
	bool IsKOTH( void ) const;
	bool IsMixedSpawns( void ) const;
	int GetInvincibilityTime( void ) const { return m_iInvincibilityTime; }

	CWeaponCacheRandomData &GetWeaponCacheRandomData(void) { return m_WeaponCacheRandomData; }

private:
	int m_iFlags;

	int m_iReqPercent;
	int m_iCapTime;

	CWeaponCacheRandomData m_WeaponCacheRandomData;

	int m_iSpawnGroup, m_iReinforcementSpawnGroup;
	int m_iInvincibilityTime;

	bool m_bIsParent;
};

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

struct IMCSquadData_t
{
	IMCSquadData_t( const char *pszName, const SlotData_t &SlotData );

	char m_szName[ MAX_SQUADNAME_LENGTH ];
	SlotData_t m_SlotData;
};

#endif

class CIMCTeamConfig
{
public:
	CIMCTeamConfig( int iID );

	void Reset( void );

	void SetClass( int iClass ) { m_iClass = iClass; }

	void SetNumWaves( int iNumWaves ) { m_iNumWaves = iNumWaves; }

#ifdef GAME_DLL

	void Finalise( void );

	void SetColor( Color &TheColor ) { m_Color = TheColor; }

	void SetTimeWave( int iTimeWave ) { m_iTimeWave = iTimeWave; }
	void SetEmergencyWaves( int iEmergencyWaves ) { m_iEmergencyWaves = iEmergencyWaves; }
	void SetObjWaves( int iObjWaves ) { m_iObjWaves = iObjWaves; }

#else

	void SetName( const char *pszName );
	void SetBriefing( const char *pszBriefing );

#endif

	int GetID( void ) const { return m_iID; }
	CTeamLookup *GetTeamLookup( void ) const;
	int GetClass( void ) const { return m_iClass; }

	int GetNumWaves( void ) const { return m_iNumWaves; } 

#ifdef GAME_DLL

	const Color &GetColor( void ) const { return m_Color; }

	int GetTimeWave( void ) const { return m_iTimeWave; }
	int GetEmergencyWaves( void ) const { return m_iEmergencyWaves; }
	int GetObjWaves( void ) const { return m_iObjWaves; }

	bool AddRawSquad( const char *pszSquadName, const SlotData_t &SlotData );

	int GetSquadCount( void ) const;
	IMCSquadData_t *GetSquad( int iID ) const;

#else

	const char *GetName( void ) const { return m_szName; }
	const char *GetBriefing( void ) const { return m_szBriefing; }

#endif

private:
	int m_iID;
	int m_iClass;

	int m_iNumWaves;

#ifdef GAME_DLL

	Color m_Color;

	int m_iTimeWave;
	int m_iEmergencyWaves;
	int m_iObjWaves;

	CUtlVector< IMCSquadData_t* > m_RawSquads;

#else

	char m_szName[ MAX_TEAMNAME_LENGTH ];
	char m_szBriefing[ MAX_MAPBRIEFING_LENGTH ];

#endif
};

//=========================================================
//=========================================================
class CIMCData
{
public:
	CIMCData( );
	virtual ~CIMCData( );

	virtual void Reset( void );

	void SetGameType( int iGameType ) { m_iGameType = iGameType; }
	int GetGameType( void ) const { return m_iGameType; }
	const char *GetGameName( void ) const;

	void SetProfileID( int iProfileID ) { m_iProfileID = iProfileID; }
	int GetProfileID( void ) const { return m_iProfileID; }

	void SetProfileName( const char *pszProfileName );
	const char *GetProfileName( void ) const { return m_szProfileName; }

	CIMCTeamConfig *GetIMCTeam( int iTeam );

	void SetTheaterID( int iID );
	int GetTheaterID( void ) const { return m_iTheaterID; }
	const char *GetTheaterName( void ) const;

	int GetVersion( void ) const;
	int GetRoundLength( void ) const;
	bool HasCustomGravity( void ) const;

#ifdef GAME_DLL

	void AddProfile( const char *pszName, int iGameType );
	int GetProfileCount( void ) const { return m_Profiles.Count( ); }
	const ProfileDataChunk_t &GetProfile( int iProfileID ) const { return m_Profiles[iProfileID]; }

	void SetRoundLength( int iRoundLength ) { m_iRoundLength = iRoundLength; }
	void SetMovingSpawns( bool bState ) { m_bMovingSpawns = bState; }
	void SetCapturingFallback( bool bState ) { m_bCapturingFallback = bState; }
	CIMCObjective *AddObjective( int iObjGlobalID );
	CIMCObjective *SetObjParent( int iID );
	void SetVersion( int iVersion ) { m_iVersion = iVersion; }
	void AllowAdditionalMapSupport( bool bState ) { m_bAdditionalMapSupport = bState; }
	void SetCutupAllowed( bool bState ) { m_bAllowCutups = bState; }
	void SetGravity( int iGravity );

	bool IsMovingSpawns( void ) const { return m_bMovingSpawns; }
	bool IsCapturingFallback( void ) const { return m_bCapturingFallback; }
	CIMCGlobalObj *GetGlobalObj( int iID );
	CIMCObjective *GetObj( int iID );
	bool FindGlobalObjective( const char *pszName, int iPossibleID, int &iID );
	int GetNumObjectives( void ) { return m_Objectives.Count( ); }
	bool AdditionalMapSupportAllowed( void ) const { return m_bAdditionalMapSupport; }
	bool IsCutupsAllowed( void ) const { return m_bAllowCutups; }

	void AddWeaponCache( CIMCWeaponCache *pWeaponCache );
	int GetWeaponCacheCount( void ) const { return m_WeaponCaches.Count( ); }
	CIMCWeaponCache *GetWeaponCache( int iID ) const { return m_WeaponCaches[ iID ]; }
	CWeaponCacheRandomData &GetWeaponCacheRandomData( void ) { return m_WeaponCacheRandomData; }

	int GetGravity( void ) const { return m_iGravity; }

#else

	void SetVGUIMapName( const char *pszVGUIMapName );
	const char *GetVGUIMapName( void ) const { return m_szVGUIMapName; }

	void SetMapName( const char *pszMapName );
	void SetOperationDate( int iOperationDate );
	void SetMapOverview( const char *pszOverview );

	void SetMapNColorF( const Color &MapNColorF ) { m_MapNColorF = MapNColorF; }
	void SetMapNColorB( const Color &MapNColorB ) { m_MapNColorB = MapNColorB; }

	void SetColorCorrection( const char *pszFilename );

	const char *GetMapName( void ) const { return m_szMapName; }
	int GetOperationDate( void ) const { return m_iOperationDate; }
	const char *GetMapOverview( void ) const { return m_szMapOverview; }

	const Color &GetMapNColorF( void ) const { return m_MapNColorF; }
	const Color &GetMapNColorB( void ) const { return m_MapNColorB; }

	const char *GetColorCorrection( void ) const { return m_szColorCorrection; }

#endif

protected:
	int m_iGameType;

	int m_iProfileID;
	char m_szProfileName[ MAX_PROFILENAME_LENGTH ];

	CIMCTeamConfig *m_pTeams[ MAX_PLAY_TEAMS ];

	int m_iTheaterID;

#ifdef GAME_DLL

	CUtlVector< ProfileDataChunk_t > m_Profiles;

	int m_iVersion;
	int m_iRoundLength;
	bool m_bMovingSpawns;
	bool m_bCapturingFallback;
	bool m_bAllowCutups;

	CIMCGlobalObj m_GlobalObjs[ MAX_OBJECTIVES ];
	CUtlVector< CIMCObjective* > m_Objectives;

	bool m_bAdditionalMapSupport;

	CUtlVector< CIMCWeaponCache* > m_WeaponCaches;
	CWeaponCacheRandomData m_WeaponCacheRandomData;

	int m_iLookupIDs[ MAX_OBJECTIVES ];

	bool m_bHasCustomGravity;
	int m_iGravity;

#else

	char m_szMapName[ MAX_MAPNAME_LENGTH ];
	char m_szVGUIMapName[ MAX_MAPNAME_LENGTH ];

	int m_iOperationDate;

	Color m_MapNColorF, m_MapNColorB;

	char m_szMapOverview[ MAX_MAPBRIEFING_LENGTH ];

	char m_szColorCorrection[ MAX_COLORCORRECTION_LENGTH ];

#endif
};

//=========================================================
//=========================================================
class CIMCConfig : public CIMCData, public CAutoGameSystem
{
public:
	~CIMCConfig( );

	static CIMCConfig *IMCConfig( void );
	static bool IsValidIMCConfig( void );

	void InitialSetup( void );
	void Setup( void );
	bool IsInit( void ) const { return m_bIsInit; }

	void Reset( void );

	bool UsingDefaults( void ) const { return m_bUsingDefaults; }

	void LevelInitPreEntity( void );

	bool IsCertifiedIMC( void ) const;

#ifdef GAME_DLL

	void Finalise( void );

	void UpdateCurrentProfile( void );
	static void SetNextProfile( int iNextProfile ) { m_iNextProfile = iNextProfile; }

#else

	bool IsSwitchedTeams( void ) const { return m_bSwitchTeams; }

#endif

	static int GetNextProfile( void );

	void Shutdown( void );

private:
	CIMCConfig( );

private:
	void PreFinalise( void );

    void LoadDefaults( void );

	int Validate( void );
	int ValidateTeam( CIMCTeamConfig *pTeam );

private:
	static CIMCConfig *m_pIMCConfig;

	bool m_bIsInit;

#ifdef GAME_DLL

	bool m_bCertifiedIMC;

	static int m_iNextProfile;

#else

	bool m_bSwitchTeams;

#endif

	bool m_bUsingDefaults;
};

//=========================================================
//=========================================================
#define DEFAULT_PROFILENAME "Unknown Profile"

#ifdef GAME_DLL

#define DEFAULT_MAXWAVES 5
#define DEFAULT_TIMEWAVE 10

#else

#define DEFAULT_MAPNAME "Undefined Mapname"
#define DEFAULT_OPDATE 562291200
#define DEFAULT_MAPOVERVIEW "No Map Description"
#define DEFAULT_BRIEFING "No Map Briefing"
#define DEFAULT_TIMESTAMP 562291200

#endif

//=========================================================
//=========================================================
class CLoadIMC
{
public:
	bool IMCExists( void );
	void GetIMCPath( char *pszBuffer, int iLength );

	virtual const char *GetFileSuffix( void ) const = 0;

	virtual void Init( CIMCData *pIMCData );
	virtual int LoadIMC( void ) = 0;

protected:
	CIMCData *m_pIMCData;
};

class CLoadIMCHelper
{
	typedef CLoadIMC *( *CreateIMCHelper_t )( void );

public:
	static CLoadIMCHelper *m_sHelpers;

	static void CreateAllElements( void );

public:
	CLoadIMCHelper( CreateIMCHelper_t CreateIMCHelper );

	CLoadIMCHelper *GetNext( void ) { return m_pNext; }

private:
	CLoadIMCHelper *m_pNext;
	CreateIMCHelper_t m_CreateIMCHelper;
};

#define DECLARE_IMCLOADER( className ) \
	static CLoadIMC *Create_##className(void) { \
			return new className; \
		}; \
	static CLoadIMCHelper g_##className##_Helper( Create_##className );

//=========================================================
//=========================================================
extern CIMCConfig *IMCConfig( void );

#endif // IMC_CONFIG_H
