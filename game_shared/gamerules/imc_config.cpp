//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//
// Notes: 
//=============================================================================//

#include "cbase.h"
#include "filesystem.h"
#include "imc_config.h"
#include "team_lookup.h"
#include "imc_utils.h"
#include "ins_gamerules.h"
#include "play_team_shared.h"

#ifdef GAME_DLL

#include "ins_squad_shared.h"

#else

#include "c_ins_imcsync.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
const char *g_pszTheaterTypes[ THEATERTYPE_COUNT ] = { 
	"",				// THEATERTYPE_UNKNOWN
	"Iraq"			// THEATERTYPE_IRAQ
	"Afghanistan"	// THEATERTYPE_AFGHANISTAN
};

//=========================================================
//=========================================================
#ifdef GAME_DLL

struct MapCRC32_t
{
	const char *m_pszMapName;
	unsigned int m_iIMC_CRC32, m_iMap_CRC32;

} g_ValidIMCs[ ] = {
	/* { "ins_ramadi", 1745551462, 4260314295 }, */
	{ NULL, 0, 0 }
};

#endif

//=========================================================
//=========================================================
CUtlVector< CLoadIMC* > m_IMCLoadTypes;

ConVar currentprofile( "ins_currentprofile", "0", FCVAR_UNREGISTERED, "", true, 0, true, MAX_IMC_PROFILES - 1 );

#ifdef TESTING

ConVar imcforcedefault( "ins_imcforcedefault", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "", true, 0, true, 1 );
#define IMC_FORCE_DEFAULT imcforcedefault.GetBool( )

#else

#define IMC_FORCE_DEFAULT false

#endif

//=========================================================
//=========================================================
CIMCConfig *CIMCConfig::m_pIMCConfig = NULL;

CIMCConfig *CIMCConfig::IMCConfig( void )
{
	if( !m_pIMCConfig )
	{
		CLoadIMCHelper::CreateAllElements( );

		m_pIMCConfig = new CIMCConfig;
		m_pIMCConfig->InitialSetup( );
	}

	return m_pIMCConfig;
}

CIMCConfig *IMCConfig( void )
{
	CIMCConfig *pConfig = CIMCConfig::IMCConfig( );
	Assert( pConfig && pConfig->IsInit( ) );

	return pConfig;
}

//=========================================================
//=========================================================
bool CIMCConfig::IsValidIMCConfig( void )
{
	return ( m_pIMCConfig != NULL );
}

//=========================================================
//=========================================================
CIMCConfig::CIMCConfig( )
{
	Reset( );
}

//=========================================================
//=========================================================
CIMCConfig::~CIMCConfig( )
{
	ClearTeamLookup( );
}

//=========================================================
//=========================================================
void CIMCConfig::InitialSetup( void )
{
	SetupTeamLookup( );
}

//=========================================================
//=========================================================
void CIMCConfig::Setup( void )
{
	Assert( !m_bIsInit );

	// we're running init
	m_bIsInit = true;

#ifdef GAME_DLL

	CINSRules::ResetError( );
	m_bCertifiedIMC = false;

#endif

	m_bUsingDefaults = false;

	// load an IMC
	int iError = GRWARN_IMC_INVALIDFILE;

#ifdef GAME_DLL

	const char *pszIMCSuffix = NULL;

#endif

	for( int i = 0; i < m_IMCLoadTypes.Count( ); i++ )
	{
		CLoadIMC *pLoadIMC = m_IMCLoadTypes[ i ];

	#ifdef TESTING

		if( IMC_FORCE_DEFAULT )
			continue;

	#endif

		if( !pLoadIMC->IMCExists( ) )
			continue;

		pLoadIMC->Init( this );
		iError = pLoadIMC->LoadIMC( );

		bool bLoaded = false;

		if( iError == 0 )
		{
			iError = Validate( );

			if( iError == 0 )
			{
				bLoaded = true;

			#ifdef GAME_DLL

				pszIMCSuffix = pLoadIMC->GetFileSuffix( );

			#endif

    			break;
			}
		}

		if( !bLoaded )
			Reset( );
	}

#ifdef GAME_DLL

	CINSRules::SetError( iError );

#endif

	// if theres an error, echo it and load defaults
	if( iError != 0 )
	{
		AssertMsg( IMC_FORCE_DEFAULT, "There was an Error in the Loaded IMC" );

		LoadDefaults( );
	}
	else
	{
		// set current profile
		m_iProfileID = GetNextProfile( );

	#ifdef GAME_DLL

		if( pszIMCSuffix )
		{
			// check CRC32's for valid IMC
			const char *pszMapName = STRING( gpGlobals->mapname );
			bool bValidIMC = false;

		#ifdef _DEBUG

			if( Q_strcmp( pszMapName, "testmap3" ) == 0 )
				bValidIMC = true;

		#endif

			for( int i = 0; !bValidIMC; i++ )
			{
				MapCRC32_t *pMapCRC32 = &g_ValidIMCs[ i ];

				if( !pMapCRC32->m_pszMapName )
					break;

				if( Q_strcmp( pMapCRC32->m_pszMapName, pszMapName ) != 0 )
					continue;

				char szIMCPath[ 128 ];
				unsigned int iIMC_CRC32, iMap_CRC32;

				Q_snprintf( szIMCPath, sizeof( szIMCPath ), "maps/%s.%s", pMapCRC32->m_pszMapName, pszIMCSuffix );

				if( !UTIL_GetCRC32File( szIMCPath, iIMC_CRC32 ) || iIMC_CRC32 != pMapCRC32->m_iIMC_CRC32 )
					continue;

				Q_snprintf( szIMCPath, sizeof( szIMCPath ), "maps/%s.bsp", pMapCRC32->m_pszMapName );

				if( !UTIL_GetCRC32File( szIMCPath, iMap_CRC32 ) || iMap_CRC32 != pMapCRC32->m_iMap_CRC32 )
					continue;

				bValidIMC = true;
			}

			m_bCertifiedIMC = bValidIMC;
		}

	#endif

	}

#ifdef GAME_DLL

	// reset the current profile
	m_iNextProfile = 0;

#endif

	// pre-finalise
	PreFinalise( );
}

//=========================================================
//=========================================================
void CIMCConfig::Reset( void )
{
	CIMCData::Reset( );

	m_bIsInit = false;

#ifdef CLIENT_DLL

	m_bSwitchTeams = false;

#endif
}

//=========================================================
//=========================================================
void CIMCConfig::LoadDefaults( void )
{
	// not a valid IMC
	m_bUsingDefaults = true;

	// setup basic vars
	SetGameType( GAMETYPE_TDM );

	SetProfileID( INVALID_PROFILE );
	SetProfileName( DEFAULT_PROFILENAME );

#ifdef GAME_DLL

	SetRoundLength( MAX_ROUNDLENGTH );
	SetMovingSpawns( false );
	SetCapturingFallback( false );

#else
	
	SetMapName( DEFAULT_MAPNAME );
	SetOperationDate( DEFAULT_OPDATE );
	SetMapOverview( DEFAULT_MAPOVERVIEW );

#endif

	// setup teams
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		CIMCTeamConfig *pTeam = GetIMCTeam( i );

		pTeam->SetClass( i == TEAM_ONE ? TEAM_USMC : TEAM_IRAQI );

	#ifdef GAME_DLL

		pTeam->SetNumWaves( DEFAULT_MAXWAVES );
		pTeam->SetTimeWave( DEFAULT_TIMEWAVE );

		SlotData_t SlotData;
		memset( SlotData, DEFAULT_SLOT, sizeof( SlotData ) );

		pTeam->AddRawSquad( "Squad", SlotData );

	#else

		pTeam->SetBriefing( DEFAULT_BRIEFING );
		
	#endif
	}
}

//=========================================================
//=========================================================
int CIMCConfig::Validate( void )
{
	int iGameType = GetGameType( );

	// check gametype
	if( iGameType < 0 || iGameType >= MAX_GAMETYPES )
		return GRWARN_IMC_BOUNDS;

	// check teams
	int iError;

	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		iError = ValidateTeam( GetIMCTeam( i ) );

		if( iError != 0 )
			return iError;
	}

#ifdef GAME_DLL

	// check round length
	int iRoundLength = GetRoundLength( );

	if( iRoundLength < MIN_ROUNDLENGTH || iRoundLength > MAX_ROUNDLENGTH )
		return GRWARN_IMC_BOUNDS;

	// since we can't capture, it would be silly to have it while moving spawns was disabled
	if( IsCapturingFallback( ) && !IsMovingSpawns( ) )
		return GRWARN_IMC_NONMOVINGFALLBACK;

	// validate objectives
	CUtlVector< const char * > ObjectiveNames;

	for( int i = 0; i < GetNumObjectives(); i++ )
	{
		CIMCObjective *pObjective = GetObj( i );
		Assert( pObjective );

		const char *pszObjName = pObjective->GetName( );

		// ensure no duped names
		if( ObjectiveNames.Find( pszObjName ) != ObjectiveNames.InvalidIndex( ) )
			return GRWARN_IMC_INVALIDOBJECTIVES;

		ObjectiveNames.AddToTail( pObjective->GetName( ) );

		// set lookupid
		m_iLookupIDs[ pObjective->GetID( ) ] = i;

		if( pObjective->IsHidden( ) )
			continue;

		// check required percentage
		int iReqPercent = pObjective->GetReqPercent( );

		if( iReqPercent > MAX_OBJ_REQPERCENT )
			return GRWARN_IMC_BOUNDS;

		// check capture time
		int iCapTime = pObjective->GetCapTime( );

		if( iCapTime < MIN_OBJ_CAPTIME || iCapTime > MAX_OBJ_CAPTIME )
			return GRWARN_IMC_BOUNDS;

		// check invis time
		int iInvincibilityTime = pObjective->GetInvincibilityTime( );

		if( iInvincibilityTime < MIN_OBJ_INVTIME || iInvincibilityTime > MAX_OBJ_INVTIME )
			return GRWARN_IMC_BOUNDS;
	}

	// validate weapon caches
	CUtlVector< int > m_UsedWeaponCacheIDs;

	for( int i = 0; i < GetWeaponCacheCount( ); i++ )
	{
		CIMCWeaponCache *pWeaponCache = GetWeaponCache( i );

		if( !pWeaponCache )
			continue;

		int iWeaponCacheID = pWeaponCache->GetID( );

		if( m_UsedWeaponCacheIDs.Find( iWeaponCacheID ) != m_UsedWeaponCacheIDs.InvalidIndex( ) )
			return GRWARN_IMC_INVALIDCACHES;

		m_UsedWeaponCacheIDs.AddToTail( iWeaponCacheID );
	}

#endif
	
	return 0;
}

//=========================================================
//=========================================================
void CIMCConfig::LevelInitPreEntity( void )
{

}

//=========================================================
//=========================================================
bool CIMCConfig::IsCertifiedIMC( void ) const
{
#ifdef GAME_DLL

	return m_bCertifiedIMC;

#else

	return IMCSync( )->IsOffical( );

#endif
}

#ifdef GAME_DLL

//=========================================================
//=========================================================
void CIMCConfig::Finalise( void )
{
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		GetIMCTeam( i )->Finalise( );
}

//=========================================================
//=========================================================
void CIMCConfig::UpdateCurrentProfile( void )
{
	currentprofile.SetValue( m_iProfileID );
}

#endif

//=========================================================
//=========================================================
void CIMCConfig::PreFinalise( void )
{
	CIMCTeamConfig *pT1, *pT2;
	pT1 = GetIMCTeam( TEAM_ONE );
	pT2 = GetIMCTeam( TEAM_TWO );

	int iT1Type, iT2Type;
	iT1Type = pT1->GetTeamLookup( )->GetType( );
	iT2Type = pT2->GetTeamLookup( )->GetType( );

	int iT1Class, iT2Class;
	iT1Class = pT1->GetClass( );
	iT2Class = pT2->GetClass( );

#ifdef GAME_DLL

	// work out the team colors
	Color T1Color, T2Color;

	if( iT1Type != iT2Type )
	{
		T1Color = CINSRules::CalculateTeamColor( iT1Type, false );
		T2Color = CINSRules::CalculateTeamColor( iT2Type, false );
	}
	else
	{
		bool bT1Backup = ( iT2Class > iT1Class );

		T1Color = CINSRules::CalculateTeamColor( iT1Type, bT1Backup );
		T2Color = CINSRules::CalculateTeamColor( iT2Type, !bT1Backup );
	}

	// set the team color
	pT1->SetColor( T1Color );
	pT2->SetColor( T2Color );

#else

	// work out if the teams need to be visually switched
	if( iT1Type != iT2Type )
	{
		if( iT1Type > iT2Type )
			m_bSwitchTeams = true;
	}
	else
	{
		if( iT2Class > iT1Class )
			m_bSwitchTeams = true;
	}

#endif
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

int CIMCConfig::m_iNextProfile = 0;

#endif

int CIMCConfig::GetNextProfile( void )
{
	// PNOTE: currentprofile lets the client know which profile 
	// to load - so, in effect, it is the "current" one

#ifdef GAME_DLL

	return m_iNextProfile;

#else

	return currentprofile.GetInt( );

#endif
}

//=========================================================
//=========================================================
int CIMCConfig::ValidateTeam( CIMCTeamConfig *pTeamConfig )
{
#ifdef GAME_DLL

	int iClass = pTeamConfig->GetClass( );

	// check team
	if( iClass < 0 || iClass >= MAX_LOOKUP_TEAMS )
		return GRWARN_IMC_BOUNDS;

	int iNumWaves = pTeamConfig->GetNumWaves( );

	// check numwave
	if( ( iNumWaves < MIN_WAVES || iNumWaves > MAX_WAVES ) && ( iNumWaves != UNLIMITED_SUPPLIES ) )
		return GRWARN_IMC_BOUNDS;

	// check timewave
	if( iNumWaves > 0 )
	{
		int iTimeWave = pTeamConfig->GetTimeWave( );

		if( iTimeWave < MIN_TIMEWAVE || iTimeWave > MAX_TIMEWAVE )
			return GRWARN_IMC_BOUNDS;
	}

	// check ewaves
	int iEWaves = pTeamConfig->GetEmergencyWaves();

	if( iEWaves < MIN_EWAVES || iEWaves > MAX_EWAVES )
		return GRWARN_IMC_BOUNDS;

	// ensure squads are valid
	for( int i = 0; i < pTeamConfig->GetSquadCount(); i++ )
	{
		IMCSquadData_t *pSquadData = pTeamConfig->GetSquad( i );

		if( !pSquadData )
			return false;

		for( int j = 0; j < MAX_SQUAD_SLOTS; j++ )
		{
			if( pSquadData->m_SlotData[ j ] == INVALID_SLOT )
				return GRWARN_IMC_INVALIDSQUAD;
		}
	}

#endif

	return 0;
}

//=========================================================
//=========================================================
void CIMCConfig::Shutdown( void )
{
	ClearTeamLookup( );
}

//=========================================================
//=========================================================
CLoadIMCHelper *CLoadIMCHelper::m_sHelpers = NULL;

CLoadIMCHelper::CLoadIMCHelper( CreateIMCHelper_t CreateIMCHelper )
{
	if( m_sHelpers == NULL )
	{
		m_pNext = m_sHelpers;
		m_sHelpers = this;
	}
	else
	{
		CLoadIMCHelper *pPrev = m_sHelpers;
		CLoadIMCHelper *pCurrent = m_sHelpers->m_pNext;

		while( pCurrent != NULL )
		{
			pPrev = pCurrent;
			pCurrent = pCurrent->m_pNext;
		}

		pPrev->m_pNext = this;
		m_pNext = pCurrent;
	}

	Assert( CreateIMCHelper );
	m_CreateIMCHelper = CreateIMCHelper;
}

//=========================================================
//=========================================================
void CLoadIMCHelper::CreateAllElements( void )
{
	CLoadIMCHelper *pHelper = m_sHelpers;

	while( pHelper )
	{
		CreateIMCHelper_t CreateIMCHelper = pHelper->m_CreateIMCHelper;

		CLoadIMC *pNewIMCLoad = ( CreateIMCHelper )( );
		Assert( pNewIMCLoad );

		if( pNewIMCLoad )
			m_IMCLoadTypes.AddToTail( pNewIMCLoad );

		pHelper = pHelper->GetNext( );
	}
}

//=========================================================
//=========================================================
bool CLoadIMC::IMCExists( void )
{
	char szIMCPath[ 32 ];
	GetIMCPath( szIMCPath, sizeof( szIMCPath ) );

	return filesystem->FileExists( szIMCPath, "GAME" );
}

//=========================================================
//=========================================================
#define IMC_PATH_PREFIX "maps/"

void CLoadIMC::GetIMCPath( char *pszBuffer, int iLength )
{
	const char *pszMapName = NULL;

#ifdef GAME_DLL

	pszMapName = STRING( gpGlobals->mapname );

#else

	char szMapName[ 32 ];
	ConvertClientMapName( szMapName );
	pszMapName = szMapName;

#endif

	Q_snprintf( pszBuffer, iLength, "%s%s.%s", IMC_PATH_PREFIX, pszMapName, GetFileSuffix( ) );
}

//=========================================================
//=========================================================
void CLoadIMC::Init( CIMCData *pIMCData )
{
	m_pIMCData = pIMCData;
}

//=========================================================
//=========================================================
CIMCData::CIMCData( )
{
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		m_pTeams[ TeamToPlayTeam( i ) ] = new CIMCTeamConfig( i );

	Reset( );
}

//=========================================================
//=========================================================
CIMCData::~CIMCData( )
{
	for( int i = 0; i < MAX_PLAY_TEAMS; i++ )
		delete m_pTeams[ i ];
}

//=========================================================
//=========================================================
void CIMCData::Reset( void )
{
	for( int i = 0; i < MAX_PLAY_TEAMS; i++ )
		m_pTeams[ i ]->Reset( );

	m_iGameType = GAMETYPE_INVALID;

	m_iTheaterID = THEATERTYPE_UNKNOWN;

#ifdef GAME_DLL

	m_iVersion = 1;
	m_bMovingSpawns = true;
	m_bCapturingFallback = true;
	m_szProfileName[ 0 ] = '\0';
	m_bAdditionalMapSupport = false;
	memset( m_iLookupIDs, INVALID_OBJECTIVE, sizeof( m_iLookupIDs ) );

	for( int i = 0; i < MAX_OBJECTIVES; i++ )
		m_GlobalObjs[ i ].Reset( );

	m_Objectives.PurgeAndDeleteElements( );

	m_WeaponCaches.PurgeAndDeleteElements( );
	
	m_iGravity = atoi( INS_DEFAULT_GRAVITY );
	m_bHasCustomGravity = false;

#else

	m_szMapName[ 0 ] = '\0';
	m_szVGUIMapName[ 0 ] = '\0';

	m_iOperationDate = 0;

	m_szMapOverview[ 0 ] = '\0';

	m_MapNColorF = IMC_DEFAULT_FCOLOR;
	m_MapNColorB = IMC_DEFAULT_BCOLOR;

	m_szColorCorrection[ 0 ] = '\0';

#endif
}

//=========================================================
//=========================================================
void CIMCData::SetProfileName( const char *pszProfileName )
{
	Q_strncpy( m_szProfileName, pszProfileName, MAX_PROFILENAME_LENGTH );
}

//=========================================================
//=========================================================
CIMCTeamConfig *CIMCData::GetIMCTeam( int iTeamID )
{
	return m_pTeams[ TeamToPlayTeam( iTeamID ) ];
}

//=========================================================
//=========================================================
const char *CIMCData::GetGameName( void ) const
{
	return g_pszGameTypes[ m_iGameType ];
}

//=========================================================
//=========================================================
void CIMCData::SetTheaterID( int iID )
{
	m_iTheaterID = iID;
}

//=========================================================
//=========================================================
const char *CIMCData::GetTheaterName( void ) const
{
	return g_pszTheaterTypes[ m_iTheaterID ];
}

//=========================================================
//=========================================================
int CIMCData::GetVersion( void ) const
{
#ifdef GAME_DLL

	return m_iVersion;

#else

	return IMCSync( )->GetVersion( );

#endif
}

//=========================================================
//=========================================================
int CIMCData::GetRoundLength( void ) const
{
#ifdef GAME_DLL

	return m_iRoundLength;

#else

	return IMCSync( )->GetRoundLength( );

#endif
}

//=========================================================
//=========================================================
bool CIMCData::HasCustomGravity( void ) const
{
#ifdef GAME_DLL

	return m_bHasCustomGravity;

#else

	return IMCSync( )->HasCustomGravity( );

#endif
}

#ifdef GAME_DLL

//=========================================================
//=========================================================
CIMCObjective *CIMCData::AddObjective( int iGlobalObjID )
{
	CIMCGlobalObj *pGlobalObj = GetGlobalObj( iGlobalObjID );

	if( !pGlobalObj )
		return NULL;

	CIMCObjective *pObjective = new CIMCObjective( pGlobalObj );

	if( m_Objectives.Count( ) == MAX_OBJECTIVES )
		return NULL;

	m_Objectives.AddToTail( pObjective );

	return pObjective;
}

//=========================================================
//=========================================================
CIMCObjective *CIMCData::SetObjParent( int iID )
{
	int iIndex = m_iLookupIDs[ iID ];

	if( iIndex == INVALID_OBJECTIVE )
		return NULL;

	CIMCObjective *pObjective = m_Objectives[ iIndex ];

	if( pObjective )
		pObjective->SetParent( );

	return pObjective;
}

//=========================================================
//=========================================================
void CIMCData::SetGravity( int iGravity )
{
	m_iGravity = iGravity;
	m_bHasCustomGravity = true;
}

//=========================================================
//=========================================================
CIMCGlobalObj *CIMCData::GetGlobalObj( int iID )
{
	return &m_GlobalObjs[ iID ];
}

//=========================================================
//=========================================================
CIMCObjective *CIMCData::GetObj( int iID )
{
	if( !m_Objectives.IsValidIndex( iID ) )
	{
		AssertMsg( false, "CIMCData::GetObj, Bad ObjID Passed" );
		return NULL;
	}

	return m_Objectives[ iID ];
}

//=========================================================
//=========================================================
bool CIMCData::FindGlobalObjective( const char *pszName, int iPossibleID, int &iID )
{
	if( iPossibleID != INVALID_OBJECTIVE )
	{
		CIMCGlobalObj &GlobalObj = m_GlobalObjs[ iPossibleID ];

		if( Q_strcmp( GlobalObj.GetName( ), pszName ) == 0 )
		{
			iID = iPossibleID;
			return true;
		}
	}
	
	for( int i = 0; i < MAX_OBJECTIVES; i++ )
	{
		CIMCGlobalObj &GlobalObj = m_GlobalObjs[ i ];

		if( GlobalObj.GetID( ) == INVALID_OBJECTIVE )
			continue;

		if( Q_strcmp( GlobalObj.GetName( ), pszName ) == 0 )
		{
			iID = i;
			return true;
		}
	}

	return false;
}

//=========================================================
//=========================================================
void CIMCData::AddProfile( const char *pszName, int iGameType )
{
	int iProfileID = m_Profiles.AddToTail( );

	ProfileDataChunk_t &Data = m_Profiles[ iProfileID ];
	Q_strncpy( Data.m_szProfileName, pszName, MAX_PROFILENAME_LENGTH );
	Data.m_iGameModeID = iGameType;
}

//=========================================================
//=========================================================
void CIMCData::AddWeaponCache( CIMCWeaponCache *pWeaponCache )
{
	m_WeaponCaches.AddToTail( pWeaponCache );
}

#else 

//=========================================================
//=========================================================
void CIMCData::SetMapName( const char *pszMapName )
{
	Q_strncpy( m_szMapName, pszMapName, MAX_MAPNAME_LENGTH );
}

//=========================================================
//=========================================================
void CIMCData::SetVGUIMapName( const char *pszVGUIMapName )
{
	Q_strncpy( m_szVGUIMapName, pszVGUIMapName, MAX_MAPNAME_LENGTH );
}

//=========================================================
//=========================================================
void CIMCData::SetOperationDate( int iOperationDate )
{
	m_iOperationDate = iOperationDate;
}

//=========================================================
//=========================================================
void CIMCData::SetMapOverview( const char *pszMapOverview )
{
	Q_strncpy( m_szMapOverview, pszMapOverview, MAX_MAPOVERVIEW_LENGTH );
}

//=========================================================
//=========================================================
void CIMCData::SetColorCorrection( const char *pszFilename )
{
	Q_strncpy( m_szColorCorrection, pszFilename, MAX_COLORCORRECTION_LENGTH );
}

#endif

#ifdef GAME_DLL

//=========================================================
//=========================================================
CWeaponCacheRandomData::CWeaponCacheRandomData( )
{
	m_bUseRandom = false;
	m_flMax = 1.0f;
	m_flMin = 0.0f;
}

//=========================================================
//=========================================================
void CWeaponCacheRandomData::Init( bool bUseRandom, float flMax, float flMin )
{
	bUseRandom = false;
	m_flMin = max( flMin, 0.0f );
	m_flMax = min( flMax, 1.0f );
}

//=========================================================
//=========================================================
CIMCWeaponCache::CIMCWeaponCache( int iID )
{
	m_iID = iID;
	m_iParentObjID = WCACHE_NOPARENTOBJ;
	m_iFlags = 0;
}

//=========================================================
//=========================================================
void CIMCGlobalObj::Reset( void )
{
	m_iID = INVALID_OBJECTIVE;
	memset( m_szName, 0, MAX_OBJNAME_LENGTH );
	m_iPhonetic = 0;
}

//=========================================================
//=========================================================
void CIMCGlobalObj::SetID( int iID )
{
	m_iID = iID;
}

//=========================================================
//=========================================================
void CIMCGlobalObj::SetName( const char *pszName )
{
	Q_strncpy( m_szName, pszName, MAX_OBJNAME_LENGTH );
}

//=========================================================
//=========================================================
CIMCObjective::CIMCObjective( const CIMCGlobalObj *pGlobalObj )
{
	m_iID = pGlobalObj->GetID( );
	Q_strncpy( m_szName, pGlobalObj->GetName( ), MAX_OBJNAME_LENGTH );
	m_iPhonetic = pGlobalObj->GetPhonetic( );
	
	m_Color = pGlobalObj->GetColor( );
	m_Color[ 3 ] = 255;

	m_bIsParent = false;
}

//=========================================================
//=========================================================
void CIMCObjective::SetReqPercent( int iReqPercent )
{
	m_iReqPercent = iReqPercent;
}

//=========================================================
//=========================================================
void CIMCObjective::SetCapTime( int iCapTime )
{
	m_iCapTime = iCapTime;
}

//=========================================================
//=========================================================
void CIMCObjective::SetFlags( int iFlags )
{
	m_iFlags = iFlags;
}

//=========================================================
//=========================================================
void CIMCObjective::SetSpawnGroup( int iSpawnGroup )
{
	m_iSpawnGroup = iSpawnGroup;
}

//=========================================================
//=========================================================
void CIMCObjective::SetReinforcementSpawnGroup( int iSpawnGroup )
{
	m_iReinforcementSpawnGroup = iSpawnGroup;
}

//=========================================================
//=========================================================
void CIMCObjective::SetInvincibilityTime( int iTime )
{
	m_iInvincibilityTime = iTime;
}

//=========================================================
//=========================================================
int CIMCObjective::GetInitialTeam( void ) const
{
	if( m_iFlags & PROFILE_OBJ_FLAG_TEAMSTART_ONE )
		return TEAM_ONE;

	if( m_iFlags & PROFILE_OBJ_FLAG_TEAMSTART_TWO )
		return TEAM_TWO;

	return TEAM_NEUTRAL;
}

//=========================================================
//=========================================================
bool CIMCObjective::IsStartingSpawn( void ) const
{
	return ( ( m_iFlags & PROFILE_OBJ_FLAG_STARTINGSPAWN ) != 0 );
}

//=========================================================
//=========================================================
bool CIMCObjective::IsHidden( void ) const
{
	return ( m_iFlags & PROFILE_OBJ_FLAG_HIDDEN ) != 0;
}

//=========================================================
//=========================================================
bool CIMCObjective::HasReinforcementSpawns( void ) const
{
	return ( ( m_iFlags & PROFILE_OBJ_FLAG_HASREINFORCEMENTS ) != 0 );
}

//=========================================================
//=========================================================
bool CIMCObjective::IsKOTH( void ) const
{
	return ( ( m_iFlags & PROFILE_OBJ_FLAG_KOTH ) != 0 );
}

//=========================================================
//=========================================================
bool CIMCObjective::IsMixedSpawns( void ) const
{
	return ( ( m_iFlags & PROFILE_OBJ_FLAG_MIXEDSPAWNS ) != 0 );
}

//=========================================================
//=========================================================
IMCSquadData_t::IMCSquadData_t( const char *pszName, const SlotData_t &SlotData )
{
	Q_strncpy( m_szName, pszName, MAX_SQUADNAME_LENGTH );
	memcpy( &m_SlotData, &SlotData, sizeof( SlotData_t ) );
}

#endif

//=========================================================
//=========================================================
CIMCTeamConfig::CIMCTeamConfig( int iID )
{
	Assert( IsPlayTeam( iID ) );

	m_iID = iID;

	Reset( );
}

//=========================================================
//=========================================================
void CIMCTeamConfig::Reset( void )
{
	m_iNumWaves = 0;

#ifdef GAME_DLL

	m_iClass = m_iTimeWave = m_iEmergencyWaves = m_iObjWaves = 0;

	m_RawSquads.Purge( );

#else

	m_szName[ 0 ] = '\0';
	m_szBriefing[ 0 ] = '\0';

#endif
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CIMCTeamConfig::Finalise( void )
{
	CPlayTeam::Create( this );
}

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

bool CIMCTeamConfig::AddRawSquad( const char *pszSquadName, const SlotData_t &SlotData )
{
	if( m_RawSquads.Count( ) >= MAX_SQUADS )
		return false;

	IMCSquadData_t *pSquadData = new IMCSquadData_t( pszSquadName, SlotData );

	if( pSquadData )
	{
		m_RawSquads.AddToTail( pSquadData );
		return true;
	}

	return false;
}

//=========================================================
//=========================================================
#else

void CIMCTeamConfig::SetName( const char *pszName )
{
	Q_strncpy( m_szName, pszName, MAX_TEAMNAME_LENGTH );
}

void CIMCTeamConfig::SetBriefing( const char *pszBriefing )
{
	Q_strncpy( m_szBriefing, pszBriefing, MAX_MAPBRIEFING_LENGTH );
}

#endif

//=========================================================
//=========================================================
CTeamLookup *CIMCTeamConfig::GetTeamLookup( void ) const
{
	return g_pTeamLookup[ m_iClass ];
}

#ifdef GAME_DLL

//=========================================================
//=========================================================
int CIMCTeamConfig::GetSquadCount( void ) const
{
	return m_RawSquads.Count( );
}

//=========================================================
//=========================================================
IMCSquadData_t *CIMCTeamConfig::GetSquad( int iID ) const
{
	return m_RawSquads[ iID ];
}

#endif
