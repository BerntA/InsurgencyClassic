//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//
// Notes: 
//=============================================================================//

#include "cbase.h"
#include "filesystem.h"
#include "imc_config.h"
#include "imc_loader_k.h"
#include "keyvalues.h"
#include "ins_gamerules.h"

#ifdef GAME_DLL

#include "team_lookup.h"
#include "imc_utils.h"
#include "ins_gamerules.h"
#include "stringlookup.h"
#include "definedsquads.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
DEFINE_STRING_LOOKUP_CONSTANTS( int, gamemodes )
	ADD_LOOKUP( GAMETYPE_BATTLE )
	ADD_LOOKUP( GAMETYPE_FIREFIGHT )
	ADD_LOOKUP( GAMETYPE_PUSH )
	ADD_LOOKUP( GAMETYPE_STRIKE )
	ADD_LOOKUP( GAMETYPE_POWERBALL )
END_STRING_LOOKUP_CONSTANTS( )

DEFINE_STRING_LOOKUP_CONSTANTS( int, theatertypes )
	ADD_LOOKUP( THEATERTYPE_UNKNOWN )
	ADD_LOOKUP( THEATERTYPE_IRAQ )
	ADD_LOOKUP( THEATERTYPE_AFGHANISTAN )
END_STRING_LOOKUP_CONSTANTS( )

DEFINE_STRING_LOOKUP_CONSTANTS( int, teamclass )
	ADD_LOOKUP( TEAM_USMC )
	ADD_LOOKUP( TEAM_IRAQI )
END_STRING_LOOKUP_CONSTANTS( )

DEFINE_STRING_LOOKUP_CONSTANTS( Color, indicolors )
	ADD_LOOKUP( IMCCOLOR_BLACK )
	ADD_LOOKUP( IMCCOLOR_WHITE )
	ADD_LOOKUP( IMCCOLOR_RED )
	ADD_LOOKUP( IMCCOLOR_BLUE )
	ADD_LOOKUP( IMCCOLOR_GREEN )
	ADD_LOOKUP( IMCCOLOR_YELLOW )
	ADD_LOOKUP( IMCCOLOR_PURPLE )
	ADD_LOOKUP( IMCCOLOR_GREY )
	ADD_LOOKUP( IMCCOLOR_ORANGE )
END_STRING_LOOKUP_CONSTANTS( )

#ifdef GAME_DLL

DEFINE_STRING_LOOKUP_CONSTANTS( int, teamstart )
	ADD_LOOKUP( TEAMSTART_ONE )
	ADD_LOOKUP( TEAMSTART_TWO )
	ADD_LOOKUP( TEAMSTART_NEUTRAL )
END_STRING_LOOKUP_CONSTANTS( )

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

const char *g_pszSquadSlotNames[ MAX_SQUAD_SLOTS ] = {
	NULL,
	"slot_two",
	"slot_three",
	"slot_four",
	"slot_five",
	"slot_six",
	"slot_seven",
	"slot_eight",
};

#endif

//=========================================================
//=========================================================
enum IMCSubKey_t
{
	IMCSUBKEY_INVALID = -1,
	IMCSUBKEY_TEAMONE = 0,
	IMCSUBKEY_TEAMTWO,
	IMCSUBKEY_OBJECTIVEDATA,
	IMCSUBKEY_WEAPONCACHEDATA,
	IMCSUBKEY_PROFILEDATA,
	IMCSUBKEY_COUNT
};

const char *g_pszIMCSubKey[ IMCSUBKEY_COUNT ] = 
{
	"TeamOne",				// IMCKSUBKEY_TEAMONE
	"TeamTwo",				// IMCKSUBKEY_TEAMTWO
	"ObjectiveData",		// IMCKSUBKEY_OBJECTIVEDATA
	"WeaponCacheData",		// IMCKSUBKEY_WEAPONCACHEDATA
	"ProfileData"			// IMCKSUBKEY_PROFILEDATA
};

//=========================================================
//=========================================================
class CLoadIMCk : public CLoadIMC
{
public:
	const char *GetFileSuffix( void ) const { return "imc2"; }

	void Init( CIMCData *pIMCData );
	int LoadIMC( void );

private:
	int GetIMCSubKeyID( const char *pszSubKey );

	int LoadProfile( KeyValues *pProfile );
	KeyValues *FindNextProfile( void );
	int GetNextProfile( void ) const;

	int LoadTeam( KeyValues *pTeam, int iTeamID );

#ifdef GAME_DLL

	int LoadObjective( KeyValues *pObjective );
	int LoadWeaponCaches( KeyValues *pWeaponCaches, bool bProfileData );

	int FindProfile( const char *pszName );
	void SetupProfileNames( void );

	bool CalculatePhonetic( char cPhonetic, int &iPhonetic );

	CIMCWeaponCache *FindWeaponCache( int iID );
	void GetWeaponCacheRandomData( KeyValues *pData, CWeaponCacheRandomData &WeaponCacheRandomData );

	int LoadProfileWeaponCacheData( KeyValues *pProfileSub );
	int LoadProfileObjData( KeyValues *pProfileSub );
	int LoadProfileTeamData( KeyValues *pProfileSub, int iTeamID );
	inline bool GetObjectiveFlags( KeyValues *pObjective, int &iFlags );

#endif

	Color CalculateCustomColor( const char *pszColour, Color *DefaultColor = NULL );

private:
	FileHandle_t m_hFile;

	KeyValues *m_pProfiles;
};

//=========================================================
//=========================================================
DECLARE_IMCLOADER( CLoadIMCk );

//=========================================================
//=========================================================
void CLoadIMCk::Init( CIMCData *pIMCData )
{
	CLoadIMC::Init( pIMCData );

	char szIMCPath[ 64 ];
	GetIMCPath( szIMCPath, sizeof( szIMCPath ) );

	m_hFile = filesystem->Open( szIMCPath, "rb", "GAME" );

	m_pProfiles = NULL;
}

//=========================================================
//=========================================================
bool KeyValuesLoadFromHFile( KeyValues *pData, const char *pszResourceName, FileHandle_t hFile )
{
	Assert( _heapchk() == _HEAPOK );

	if( !hFile )
		return false;

	// load file into a null-terminated buffer
	int iFileSize = filesystem->Size( hFile );
	char *pszBuffer = ( char* )MemAllocScratch( iFileSize + 1 );

	 // read into local buffer
	Assert( pszBuffer );
	filesystem->Read( pszBuffer, iFileSize, hFile );

	// null terminate file as EOF
	pszBuffer[ iFileSize ] = '\0';

	// close file after reading
	filesystem->Close( hFile );

	bool bRetOK = pData->LoadFromBuffer( pszResourceName, pszBuffer, filesystem);

	MemFreeScratch( );

	return bRetOK;
}

//=========================================================
//=========================================================
int CLoadIMCk::LoadIMC( void )
{
	// load the file
	KeyValues *pIMCData, *pSubIMCData;

	pIMCData = new KeyValues( "IMCData" );

	if( !KeyValuesLoadFromHFile( pIMCData, "IMCData", m_hFile ) )
		return false;

	// find basic info
	if( !pIMCData )
		return false;

	const char *pszTheaterType = pIMCData->GetString( MAP_THEATER, NULL );

	if( pszTheaterType )
	{
		int iTheaterType;

		if( !GetTheaterType( pszTheaterType, iTheaterType ) )
			m_pIMCData->SetTheaterID( iTheaterType );
	}

#ifdef GAME_DLL

	m_pIMCData->SetVersion( pIMCData->GetInt( MAP_VERSION, 1 ) );
	m_pIMCData->AllowAdditionalMapSupport( pIMCData->GetInt( MAP_AMAPSUPPORT ) ? true : false );

	int iGravity = pIMCData->GetInt( MAP_GRAVITY, -1 );

	if( iGravity > 0 )
		m_pIMCData->SetGravity( iGravity );

#else

	const char *pszVGUIMapName = pIMCData->GetString( MAP_VGUINAME, "" );
	const char *pszMapName = pIMCData->GetString( MAP_NAME, NULL );

	if( !pszMapName || !pszVGUIMapName )
		return GRWARN_IMC_MISSINGDATA;

	m_pIMCData->SetMapName( pszMapName );
	m_pIMCData->SetVGUIMapName( pszVGUIMapName );
	m_pIMCData->SetMapOverview( pIMCData->GetString( MAP_OVERVIEW, "" ) );
	m_pIMCData->SetOperationDate( pIMCData->GetInt( MAP_OPDATE, DEFAULT_TIMESTAMP ) );

	// handle mapname colours
	Color DefaultColor;
	const char *pszMapNColorF = pIMCData->GetString( MAP_NCOLORF, NULL );

	if( pszMapNColorF )
	{
		Color MapNColorF;

		if( !GetIMCColor( pszMapNColorF, MapNColorF ) )
		{
			DefaultColor = IMC_DEFAULT_FCOLOR;
			MapNColorF = CalculateCustomColor( pszMapNColorF, &DefaultColor );
		}

		m_pIMCData->SetMapNColorF( MapNColorF );
	}

	const char *pszMapNColorB = pIMCData->GetString( MAP_NCOLORB, NULL );

	if( pszMapNColorB )
	{
		Color MapNColorB;

		if(!GetIMCColor(pszMapNColorB, MapNColorB))
		{
			DefaultColor = IMC_DEFAULT_BCOLOR;
			MapNColorB = CalculateCustomColor(pszMapNColorB, &DefaultColor);
		}

		m_pIMCData->SetMapNColorB(MapNColorB);
	}

#endif

	// start finding subkeys
	pSubIMCData = pIMCData->GetFirstTrueSubKey( );

	if( !pSubIMCData )
		return GRWARN_IMC_MISSINGDATA;

	KeyValues *pIMCSubKeys[ IMCSUBKEY_COUNT ];
	memset( pIMCSubKeys, NULL, sizeof( pIMCSubKeys ) );

	for( ; pSubIMCData; pSubIMCData = pSubIMCData->GetNextKey( ) )
	{
		const char *pszName = pSubIMCData->GetName( );

		int iIMCSubKeyID = GetIMCSubKeyID( pszName );

		if( iIMCSubKeyID == IMCSUBKEY_INVALID )
			continue;

		pIMCSubKeys[ iIMCSubKeyID ] = pSubIMCData;
	}

	// execute them in the right order
	for( int i = 0; i < IMCSUBKEY_COUNT; i++ )
	{
		KeyValues *pIMCSubKey = pIMCSubKeys[ i ];

		if( !pIMCSubKey )
		{
			if( i != IMCSUBKEY_WEAPONCACHEDATA )
				return GRWARN_IMC_INVALIDFORMAT;
			else
				continue;
		}

		int iError = 0;

		switch( i )
		{
			case IMCSUBKEY_TEAMONE:
				iError = LoadTeam( pIMCSubKey, TEAM_ONE );
				break;

			case IMCSUBKEY_TEAMTWO:
				iError = LoadTeam( pIMCSubKey, TEAM_TWO );
				break;

		#ifdef GAME_DLL

			case IMCSUBKEY_OBJECTIVEDATA:
				iError = LoadObjective( pIMCSubKey );
				break;

			case IMCSUBKEY_WEAPONCACHEDATA:
				iError = LoadWeaponCaches( pIMCSubKey, false );
				break;

		#endif

			case IMCSUBKEY_PROFILEDATA:
				iError = LoadProfile( pIMCSubKey );
				break;
		}

		if( iError != 0 )
			return iError;
	}

	return 0;
}

//=========================================================
//=========================================================
int CLoadIMCk::GetIMCSubKeyID( const char *pszSubKey )
{
	for( int i = 0; i < IMCSUBKEY_COUNT; i++ )
	{
		if( Q_stricmp( pszSubKey, g_pszIMCSubKey[ i ] ) == 0 )
			return i;
	}

	return IMCSUBKEY_INVALID;
}

//=========================================================
//=========================================================
int CLoadIMCk::LoadTeam( KeyValues *pTeam, int iTeamID )
{
	// get the basic data
	int iTeamClass;

	const char *pszType = pTeam->GetString( TEAM_TYPE );

	if( !GetTeamClass( pszType, iTeamClass ) )
		return GRWARN_IMC_INVALIDTEAM;

	int iNumWaves = pTeam->GetInt( TEAM_NUMWAVES );

	if( pTeam->IsEmpty( TEAM_NUMWAVES ) )
		return GRWARN_IMC_MISSINGDATA;

#ifdef GAME_DLL

	int iTimeWave, iEmergencyWaves, iObjWaves;

	iTimeWave = pTeam->GetInt( TEAM_TIMEWAVE );
	iEmergencyWaves = pTeam->GetInt( TEAM_EWAVES );
	iObjWaves = pTeam->GetInt( TEAM_OBJWAVES );

#endif

	// find team config
	CIMCTeamConfig *pTeamConfig = m_pIMCData->GetIMCTeam( iTeamID );

	pTeamConfig->SetClass( iTeamClass );

	pTeamConfig->SetNumWaves( iNumWaves );

#ifdef GAME_DLL

	pTeamConfig->SetTimeWave( iTimeWave );
	pTeamConfig->SetEmergencyWaves( iEmergencyWaves );
	pTeamConfig->SetObjWaves( iObjWaves );

	// now start loading the squads
	KeyValues *pSquads = pTeam->FindKey( TEAM_SQUADS );

	if( !pSquads )
		return GRWARN_IMC_NOSQUADS;

	KeyValues *pSquad = pSquads->GetFirstSubKey( );

	if( !pSquad )
		return GRWARN_IMC_NOSQUADS;

	int iSquadCount = 0;

	for( ; pSquad; pSquad = pSquad->GetNextKey( ) )
	{
		const char *pszSquadType = pSquad->GetName( );

		bool bValidSquad = false;
		char szSquadName[ MAX_SQUADNAME_LENGTH ];
		SlotData_t SlotData;

		memset( SlotData, DEFAULT_SLOT, sizeof( SlotData_t ) );

		if( Q_stricmp( pszSquadType, SQUADS_DEFINED ) == 0 )
		{
			const char *pszSquadFileName = pSquad->GetString( );

			if( pszSquadFileName )
				bValidSquad = LoadDefinedSquad( pszSquadFileName, iTeamClass, szSquadName, SlotData );
		}
		else if( Q_stricmp( pszSquadType, SQUADS_DATA ) == 0 )
		{
			bValidSquad = LoadDefinedSquadData( pSquad, iTeamClass, szSquadName, SlotData );
		}

		if( !bValidSquad )
			return GRWARN_IMC_INVALIDSQUAD;

		if( !pTeamConfig->AddRawSquad( szSquadName, SlotData ) )
			return GRWARN_IMC_INVALIDSQUAD;

		iSquadCount++;
	}

	if( iSquadCount == 0 )
		return GRWARN_IMC_NOSQUADS;

#else

	const char *pszName = pTeam->GetString( TEAM_NAME, NULL );
	const char *pszBriefing = pTeam->GetString( TEAM_BRIEFING, NULL );

	if( pszName )
		pTeamConfig->SetName( pszName );

	if( pszBriefing )
		pTeamConfig->SetBriefing( pszBriefing );

#endif

	return 0;
}

#ifdef GAME_DLL

//=========================================================
//=========================================================
int CLoadIMCk::LoadObjective( KeyValues *pObjectives )
{
	KeyValues *pObjective = pObjectives->GetFirstTrueSubKey( );

	if( !pObjective )
		return GRWAWN_IMC_NOOBJECTIVES;

	int iObjectiveID = 0;

	for( ; pObjective; pObjective = pObjective->GetNextKey( ), iObjectiveID++ )
	{
		CIMCGlobalObj *pGlobalObj = m_pIMCData->GetGlobalObj( iObjectiveID );

		if( !pGlobalObj )
			return GRWARN_IMC_INVALIDGLOBALOBJ;

		const char *pszObjName, *pszPhonetic, *pszColor;
		int iPhonetic;

		pszObjName = pObjective->GetName( );
		pszPhonetic = pObjective->GetString( OBJ_PHONETIC, NULL );
		pszColor = pObjective->GetString( OBJ_COLOR, NULL );
		
		if( !pszObjName || !pszPhonetic || !pszColor || ( !CalculatePhonetic( pszPhonetic[ 0 ], iPhonetic ) ) )
			return GRWARN_IMC_INVALIDGLOBALOBJ;

		pGlobalObj->SetID( iObjectiveID );
		pGlobalObj->SetName( pszObjName );
		pGlobalObj->SetPhonetic( iPhonetic );

		Color IMCColor;

		if( !GetIMCColor( pszColor, IMCColor ) )
			IMCColor = CalculateCustomColor( pszColor );

		pGlobalObj->SetColor( IMCColor );
	}

	return 0;
}

//=========================================================
//=========================================================
bool CLoadIMCk::CalculatePhonetic( char cPhonetic, int &iPhonetic )
{
	if( cPhonetic > 122 )
		return false;

	if( cPhonetic > 90 )
		iPhonetic = cPhonetic - 97;
	else
		iPhonetic = cPhonetic - 65;

	return ( iPhonetic >= 0 && iPhonetic <= 26 );
}

//=========================================================
//=========================================================
int CLoadIMCk::LoadWeaponCaches( KeyValues *pWeaponCaches, bool bProfileData )
{
	/*KeyValues *pWeaponCache = pWeaponCaches->GetFirstSubKey( );

	if( !pWeaponCache )
		return 0;

	if( !bProfileData )
		GetWeaponCacheRandomData( pWeaponCaches, m_pIMCData->GetWeaponCacheRandomData( ) );

	for( ; pWeaponCache; pWeaponCache = pWeaponCache->GetNextKey( ) )
	{
		const char *pszWeaponCacheID = pWeaponCache->GetName( );

		if( !pszWeaponCacheID )
			return GRWARN_IMC_INVALIDCACHES;

		int iWeaponCacheID = atoi( pszWeaponCacheID );

		if( iWeaponCacheID < 0 )
			return GRWARN_IMC_INVALIDCACHES;

		bool bJustCreated = false;
		CIMCWeaponCache *pWeaponCacheData = FindWeaponCache( iWeaponCacheID );

		if( !pWeaponCacheData )
		{
			pWeaponCacheData = new CIMCWeaponCache( iWeaponCacheID );
			bJustCreated = true;
		}

		const char *pszParentObjective = pWeaponCache->GetString( WCACHE_PARENTOBJ );

		if( pszParentObjective && *pszParentObjective != '\0' )
		{
			int iObjGlobalID;

			if( !m_pIMCData->FindGlobalObjective( pszParentObjective, INVALID_OBJECTIVE, iObjGlobalID ) )
				return GRWARN_IMC_INVALIDCACHES;

			CIMCGlobalObj *pObjective = m_pIMCData->GetGlobalObj( iObjGlobalID );
			Assert( pObjective );

			pWeaponCacheData->SetParentObjID( pObjective->GetID( ) );
		}

		int iFlags = 0;

		if( pWeaponCache->GetInt( WCACHE_NORESPAWN ) )
			iFlags |= WCACHE_FLAG_NORESPAWN;

		if( pWeaponCache->GetInt( WCACHE_SPAWNPARENT ) )
			iFlags |= WCACHE_FLAG_SPAWNPARENT;

		if( pWeaponCache->GetInt( WCACHE_DEPLOYSTOCK ) )
			iFlags |= WCACHE_FLAG_DEPLOYSTOCK;

		if( pWeaponCache->GetInt( WCACHE_TEAMONESPAWN ) )
			iFlags |= WCACHE_FLAG_TEAMONESPAWN;

		if( pWeaponCache->GetInt( WCACHE_TEAMTWOSPAWN ) )
			iFlags |= WCACHE_FLAG_TEAMTWOSPAWN;

		pWeaponCacheData->SetFlags( iFlags );

		bool bSuccess = true;
		const char *pszDefinedWeaponCache = NULL;
		KeyValues *pWeaponCacheDefinedData = NULL;

		CWeaponCacheInventoryBlueprint *pBlueprint = pWeaponCacheData->GetBlueprint( );

		if( ( pszDefinedWeaponCache = pWeaponCache->GetString( WEAPONCACHE_DEFINED, NULL ) ) != NULL )
			bSuccess = LoadDefinedWeaponCache( pszDefinedWeaponCache, pBlueprint );
		else if( ( pWeaponCacheDefinedData = pWeaponCache->FindKey( WEAPONCACHE_DATA, NULL ) ) != NULL )
			bSuccess = LoadDefinedWeaponCacheData( pWeaponCacheDefinedData, pBlueprint );

		if( !bSuccess )
		{
			if( bJustCreated )
				delete pWeaponCacheData;

			return GRWARN_IMC_INVALIDCACHES;
		}

		if( bJustCreated )
			m_pIMCData->AddWeaponCache( pWeaponCacheData );
	}*/

	return 0;
}

//=========================================================
//=========================================================
CIMCWeaponCache *CLoadIMCk::FindWeaponCache( int iID )
{
	for( int i = 0; i < m_pIMCData->GetWeaponCacheCount( ); i++ )
	{
		CIMCWeaponCache *pWeaponCache = m_pIMCData->GetWeaponCache( i );

		if( pWeaponCache && pWeaponCache->GetID( ) == iID )
			return pWeaponCache;
	}

	return NULL;
}

//=========================================================
//=========================================================
void CLoadIMCk::GetWeaponCacheRandomData( KeyValues *pData, CWeaponCacheRandomData &WeaponCacheRandomData )
{
	WeaponCacheRandomData.Init( pData->GetInt( WCACHE_RANDOM, 0 ) ? true : false,
		pData->GetFloat( WCACHE_RANDOM_MIN, 0.0f ),
		pData->GetFloat( WCACHE_RANDOM_MAX, 1.0f ) );
}

#endif

//=========================================================
//=========================================================
int CLoadIMCk::LoadProfile( KeyValues *pProfiles )
{
	// ensure there are profiles to load
	KeyValues *pProfile = pProfiles->GetFirstTrueSubKey( );

	if( !pProfile )
		return GRWARN_IMC_NOPROFILES;

	// assign a pointer for a quick setup later
	m_pProfiles = pProfiles;

	// try and load the next one
	KeyValues *pCurrentProfile = NULL;
	
	if( ( pCurrentProfile = FindNextProfile( ) ) == NULL )
		return GRWARN_IMC_INVALIDPROFILE;		

#ifdef GAME_DLL

	// find the names of all the profiles
	SetupProfileNames( );

#endif

	// load the profile
	const char *pszProfileName = pCurrentProfile->GetName( );

	if( !pszProfileName )
		return GRWARN_IMC_INVALIDPROFILE;

	m_pIMCData->SetProfileName( pszProfileName );

	if( pCurrentProfile->IsEmpty( PROFILE_GAMETYPE ) )
		return GRWARN_IMC_MISSINGDATA;

	const char *pszGameType = pCurrentProfile->GetString( PROFILE_GAMETYPE );
	int iGameType;

	if( !GetGameMode( pszGameType, iGameType ) )
		return GRWARN_IMC_MISSINGDATA;

	m_pIMCData->SetGameType( iGameType );

#ifdef GAME_DLL

	if( pCurrentProfile->IsEmpty( PROFILE_ROUNDLENGTH ) )
		return GRWARN_IMC_MISSINGDATA;

	m_pIMCData->SetRoundLength( pCurrentProfile->GetInt( PROFILE_ROUNDLENGTH ) );

	m_pIMCData->SetMovingSpawns( pCurrentProfile->GetInt( PROFILE_MOVINGSPAWNS, 1 ) ? true : false );

	// NOTE: the default is the same as moving spawns because you want to
	// encourage this type of gameplay but at the same time not force the mappers
	// to set this 0 for non-moving spawn maps
	m_pIMCData->SetCapturingFallback( pCurrentProfile->GetInt( PROFILE_CAPTURINGFALLBACK, m_pIMCData->IsMovingSpawns( ) ) ? true : false );

	KeyValues *pProfileSub = pCurrentProfile->GetFirstTrueSubKey( );

	if( !pProfileSub )
		return GRWARN_IMC_INVALIDPROFILE;

	for( ; pProfileSub; pProfileSub = pProfileSub->GetNextKey( ) )
	{
		const char *pszSubName = pProfileSub->GetName( );

		int iError = 0;

		if( Q_stricmp( PROFILE_OBJECTIVEDATA, pszSubName ) == 0 )
			iError = LoadProfileObjData( pProfileSub );
		else if( Q_stricmp( PROFILE_WEAPONCACHEDATA, pszSubName ) == 0)
			iError = LoadWeaponCaches( pProfileSub, true );
		else if( Q_stricmp( PROFILE_TEAMONE, pszSubName) == 0 )
			iError = LoadProfileTeamData( pProfileSub, TEAM_ONE );
		else if( Q_stricmp( PROFILE_TEAMTWO, pszSubName) == 0 )
			iError = LoadProfileTeamData( pProfileSub, TEAM_TWO );

		if( iError != 0 )
			return iError;
	}

#else

	const char *pszColorCorrection = pCurrentProfile->GetString( PROFILE_CORRECTION, NULL );

	if( pszColorCorrection )
		m_pIMCData->SetColorCorrection( pszColorCorrection );

#endif

	return 0;
}

//=========================================================
//=========================================================
KeyValues *CLoadIMCk::FindNextProfile( void )
{
	KeyValues *pProfile = m_pProfiles->GetFirstTrueSubKey( );

	for( int iProfileID = 0; pProfile; pProfile = pProfile->GetNextKey( ), iProfileID++ )
	{
		if( iProfileID == CIMCConfig::GetNextProfile( ) )
			return pProfile;
	}

	return NULL;
}

#ifdef GAME_DLL

//=========================================================
//=========================================================
void CLoadIMCk::SetupProfileNames( void )
{
	KeyValues *pProfile = m_pProfiles->GetFirstTrueSubKey( );

	const char *pszName, *pszGameType;
	int iGameType;

	for( int iProfileID = 0; pProfile; pProfile = pProfile->GetNextKey( ), iProfileID++ )
	{
		pszName = pProfile->GetName( );
		pszGameType = pProfile->GetString( PROFILE_GAMETYPE );

		if( !pszName || !GetGameMode( pszGameType, iGameType ) )
			continue;

		m_pIMCData->AddProfile( pszName, iGameType );
	}
}

//=========================================================
//=========================================================
int CLoadIMCk::FindProfile( const char *pszName )
{
	if( !m_pProfiles )
		return INVALID_PROFILE;

	KeyValues *pProfile = m_pProfiles->GetFirstTrueSubKey( );

	for( int iProfileID = 0; pProfile; pProfile = pProfile->GetNextKey( ), iProfileID++ )
	{
		if( Q_strcmp( pProfile->GetName( ), pszName ) != 0 )
			continue;

		return iProfileID;
	}

	return INVALID_PROFILE;
}

//=========================================================
//=========================================================
int CLoadIMCk::LoadProfileObjData( KeyValues *pProfileSub )
{
	KeyValues *pObjective = pProfileSub->GetFirstTrueSubKey( );

	for( int iPossibleID = 0; pObjective; pObjective = pObjective->GetNextKey( ), iPossibleID++ )
	{
		// NOTE: name memory seems to change location
		if( !pObjective->GetName( ) )
			return GRWARN_IMC_INVALIDPROFILE;

		int iObjFlags;

		if( !GetObjectiveFlags( pObjective, iObjFlags ) )
			return GRWARN_IMC_INVALIDPROFILE;

		int iObjGlobalID;

		if( !m_pIMCData->FindGlobalObjective( pObjective->GetName( ), iPossibleID, iObjGlobalID ) )
			return GRWARN_IMC_INVALIDPROFILE;

		CIMCObjective *pIMCObjective = m_pIMCData->AddObjective( iObjGlobalID );

		if( !pIMCObjective )
			return GRWARN_IMC_INVALIDPROFILE;

		pIMCObjective->SetFlags( iObjFlags );

		pIMCObjective->SetReqPercent( pObjective->GetInt( PROFILE_OBJ_REQPERCENT ) );
		pIMCObjective->SetCapTime( pObjective->GetInt( PROFILE_OBJ_CAPTIME ) );

		KeyValues *pWeaponCacheData = pObjective->FindKey( WEAPONCACHE_DATA );

		if( pWeaponCacheData )
			GetWeaponCacheRandomData( pWeaponCacheData, pIMCObjective->GetWeaponCacheRandomData( ) );

		pIMCObjective->SetSpawnGroup( pObjective->GetInt( PROFILE_OBJ_SPAWNGROUP ) );
		pIMCObjective->SetReinforcementSpawnGroup( pObjective->GetInt( PROFILE_OBJ_SPAWNGROUP_REINFORCEMENT ) );
		pIMCObjective->SetInvincibilityTime( pObjective->GetInt( PROFILE_OBJ_INVISTIME ) );
	}

	return 0;
}

//=========================================================
//=========================================================
int CLoadIMCk::LoadProfileTeamData( KeyValues *pProfileSub, int iTeamID )
{
	CIMCTeamConfig *pIMCTeamConfig = m_pIMCData->GetIMCTeam( iTeamID );

	if( !pProfileSub->IsEmpty( TEAM_NUMWAVES ) )
		pIMCTeamConfig->SetNumWaves( pProfileSub->GetInt( TEAM_NUMWAVES ) );

	if(!pProfileSub->IsEmpty( TEAM_TIMEWAVE ) )
		pIMCTeamConfig->SetTimeWave( pProfileSub->GetInt( TEAM_TIMEWAVE ) );

#ifdef CLIENT_DLL

	if( !pProfileSub->IsEmpty( TEAM_NAME ) )
		pIMCTeamConfig->SetName( pProfileSub->GetString( TEAM_NAME ) );

	if( !pProfileSub->IsEmpty( TEAM_BRIEFING ) )
		pIMCTeamConfig->SetBriefing( pProfileSub->GetString( TEAM_BRIEFING ) );

#endif

	return 0;
}

//=========================================================
//=========================================================
bool CLoadIMCk::GetObjectiveFlags( KeyValues *pObjective, int &iFlags )
{
	iFlags = 0;

	const char *pszTeamStart = pObjective->GetString( PROFILE_OBJ_TEAMSTART );
	int iTeamStart;

	if( !GetTeamStart( pszTeamStart, iTeamStart ) )
		return false;

	switch( iTeamStart )
	{
		case TEAMSTART_ONE:
			iFlags |= PROFILE_OBJ_FLAG_TEAMSTART_ONE;
			break;

		case TEAMSTART_TWO:
			iFlags |= PROFILE_OBJ_FLAG_TEAMSTART_TWO;
			break;

		case TEAMSTART_NEUTRAL:
			iFlags |= PROFILE_OBJ_FLAG_TEAMSTART_NEUTRAL;
			break;
	}

	if( pObjective->GetInt( PROFILE_OBJ_STARTSPAWN ) )
		iFlags |= PROFILE_OBJ_FLAG_STARTINGSPAWN;

	if( pObjective->GetInt( PROFILE_OBJ_HIDDEN ) )
		iFlags |= PROFILE_OBJ_FLAG_HIDDEN;

	if( pObjective->GetInt( PROFILE_OBJ_RSPAWNS ) )
		iFlags |= PROFILE_OBJ_FLAG_HASREINFORCEMENTS;

	if( pObjective->GetInt( PROFILE_OBJ_KOTH ) )
		iFlags |= PROFILE_OBJ_FLAG_KOTH;

	if( pObjective->GetInt( PROFILE_OBJ_MIXEDSPAWNS ) )
		iFlags |= PROFILE_OBJ_FLAG_MIXEDSPAWNS;

	return true;
}

#endif

//=========================================================
//=========================================================
Color CLoadIMCk::CalculateCustomColor( const char *pszColour, Color *DefaultColor )
{
	Color CustomColor;

	if( DefaultColor )
		CustomColor = *DefaultColor;
	else
		CustomColor = IMCCOLOR_WHITE;

	char szColor[ 32 ];
	Q_strncpy( szColor, pszColour, sizeof( szColor ) );
	char *pszToken;

	int iColor[ 3 ];
	memset( iColor, 0, sizeof( iColor ) );

	pszToken = strtok( szColor, " " );

	for( int i = 0; pszToken != NULL && i < 3; pszToken = strtok( NULL, " " ), i++ )
		iColor[ i ] = atoi( pszToken );

	CustomColor.SetColor( iColor[ 0 ], iColor[ 1 ], iColor[ 2 ] );

	return CustomColor;
}
