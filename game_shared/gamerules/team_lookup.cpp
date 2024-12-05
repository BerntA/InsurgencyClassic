//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Lookup Tables for Team Configuration.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "team_lookup.h"
#include "imc_format.h"
#include "filesystem.h"
#include "keyvalues.h"
#include "weapon_defines.h"
#include "clipdef.h"
#include "play_team_shared.h"
#include "equipment_helpers.h"
#include "ins_player_shared.h"

#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
AddTeam_t g_CreateTeamLookup[ MAX_LOOKUP_TEAMS ] = {
	NULL,	// TEAM_USMC
	NULL,	// TEAM_IRAQI
};

CPlayerClassHelper *g_TeamClassLookup[ MAX_LOOKUP_TEAMS ] = {
	NULL,	// TEAM_USMC
	NULL,	// TEAM_IRAQI
};

//=========================================================
//=========================================================
CPlayerModelData::CPlayerModelData( )
{
	m_szPath[ 0 ] = '\0';

	m_Customization.aBodygroups.RemoveAll( );
	m_Customization.aSkins.RemoveAll( );
	m_Customization.bLoaded = false;
}

//=========================================================
//=========================================================
CPlayerModelData::~CPlayerModelData( )
{
	if( m_Customization.bLoaded )
	{
		int i, j;

		for( i = 0; i < m_Customization.aBodygroups.Count( ); i++ )
		{
			if( !m_Customization.aBodygroups[ i ].bLoaded )
				continue;

			if( m_Customization.aBodygroups[ i ].pszItempos )
				delete [ ]m_Customization.aBodygroups[ i ].pszItempos;

			delete [ ]m_Customization.aBodygroups[ i ].pszName;
			delete [ ]m_Customization.aBodygroups[ i ].pszAttachment;

			for( j = 0; j < m_Customization.aBodygroups[ i ].aSubmodels.Count( ); j++)
			{
				if( !m_Customization.aBodygroups[ i ].aSubmodels[ j ].bLoaded )
					continue;

				delete [ ]m_Customization.aBodygroups[ i ].aSubmodels[ j ].pszName;
				delete [ ]m_Customization.aBodygroups[ i ].aSubmodels[ j ].pszDesc;
				delete [ ]m_Customization.aBodygroups[ i ].aSubmodels[ j ].pszItemcode;
			}

			m_Customization.aBodygroups[ i ].aSubmodels.RemoveAll( );
		}

		m_Customization.aBodygroups.RemoveAll( );

		for( i = 0; i < m_Customization.aSkins.Count( ); i++)
		{
			if( !m_Customization.aSkins[ i ].bLoaded )
				continue;

			delete [ ]m_Customization.aSkins[ i ].pszDesc;
			delete [ ]m_Customization.aSkins[ i ].pszName;
			delete [ ]m_Customization.aSkins[ i ].pszModel;
		}

		m_Customization.aSkins.RemoveAll( );

		delete [ ]m_Customization.pszModelFile;
		delete [ ]m_Customization.pszVersion;
		delete [ ]m_Customization.pszHeadAttachment;
		delete [ ]m_Customization.pszHeadName;
	}
}

//=========================================================
//=========================================================
void CPlayerModelData::SetPath( const char *pszModel )
{
	Q_snprintf( &m_szPath[ 0 ], MODEL_STRING_LENGTH, "models/player/%s.mdl", pszModel );
}

//=========================================================
//=========================================================
void CPlayerModelData::SetupCustomization( void )
{
	SetupModelCustomization( m_szPath, m_Customization );
}

//=========================================================
//=========================================================
RankNames_t::RankNames_t( )
{
	m_pszName = NULL;
	m_pszShort = NULL;
}

//=========================================================
//=========================================================
CTeamLookup::CTeamLookup( int iTeamClass )
{
	g_pTeamLookup[ iTeamClass ] = this;

	m_pszName = NULL;
	m_pszFullName = NULL;
	m_pszFileName = NULL;

	m_iCommanderClassID = INVALID_CLASS;

	m_iDefaultWeapon = 0;

	m_bForeignLanguage = false;
}

//=========================================================
//=========================================================
CTeamLookup::~CTeamLookup( )
{
	m_Classes.PurgeAndDeleteElements( );
}

//=========================================================
//=========================================================
void CTeamLookup::Precache( void )
{
	// precache player model
	CBaseEntity::PrecacheModel( m_PlayerModelData.GetPath( ) );

	// now do the classes
	for( int i = 0; i < m_Classes.Count(); i++ )
	{
		CPlayerClass *pPlayerClass = m_Classes[ i ];

		if( pPlayerClass )
			pPlayerClass->Precache( );
	}
}

//=========================================================
//=========================================================
void CTeamLookup::SetupCustomization( void )
{
	// setup own model
	m_PlayerModelData.SetupCustomization( );

	// now do the classes
	for( int i = 0; i < m_Classes.Count( ); i++ )
	{
		CPlayerClass *pPlayerClass = m_Classes[ i ];

		if( pPlayerClass )
			pPlayerClass->SetupCustomization( );
	}
}

//=========================================================
//=========================================================
void CTeamLookup::AddAdditionalName( const char *pszAdditionalName )
{
	m_AdditionalNames.AddToTail( pszAdditionalName );
}

//=========================================================
//=========================================================
void CTeamLookup::SetModel( const char *pszModel )
{
	m_PlayerModelData.SetPath( pszModel );
}

//=========================================================
//=========================================================
void CTeamLookup::SetRankName( int iID, const char *pszName, const char *pszShort )
{
	RankNames_t &RankNames = m_RankNames[ iID ];
	RankNames.m_pszName = pszName;
	RankNames.m_pszShort = pszShort;
}

//=========================================================
//=========================================================
enum RandomNameType_t
{
	RNTYPE_NORMAL = 0,
	RNTYPE_FULL,
	RNTYPE_ADDITIONAL
};

const char *CTeamLookup::GetRandomName( void ) const
{
	int iRandomNameType;

	if( m_AdditionalNames.Count( ) == 0 )
		iRandomNameType = random->RandomInt( RNTYPE_NORMAL, RNTYPE_ADDITIONAL );
	else
		iRandomNameType = random->RandomInt( RNTYPE_NORMAL, RNTYPE_FULL );

	switch( iRandomNameType )
	{
		case RNTYPE_NORMAL:
			return m_pszName;

		case RNTYPE_FULL:
			return m_pszFullName;

		case RNTYPE_ADDITIONAL:
			return m_AdditionalNames[ random->RandomInt( 0, m_AdditionalNames.Count( ) - 1 ) ];
	}

	return NULL;
}

//=========================================================
//=========================================================
const char *CTeamLookup::GetFullRankName( int iID ) const
{
	Assert( UTIL_ValidRank( iID ) );
	return m_RankNames[ iID ].m_pszName;
}

//=========================================================
//=========================================================
const char *CTeamLookup::GetRankName( int iID ) const
{
	Assert( UTIL_ValidRank( iID ) );
	return m_RankNames[ iID ].m_pszShort;
}

//=========================================================
//=========================================================
#define WEIGHTOFFSET_CONVENTIONAL	0.75f
#define WEIGHTOFFSET_UNCONVENTIONAL 1.0f

float CTeamLookup::GetWeightOffset( void ) const
{
	return ( ( m_iType == TEAMTYPE_CONVENTIONAL ) ? WEIGHTOFFSET_CONVENTIONAL : WEIGHTOFFSET_UNCONVENTIONAL );
}

//=========================================================
//=========================================================
const char *g_pszWeaponTypes[ WEAPONTYPE_COUNT ] = {
	"PrimaryWeapons",	// WEAPONTYPE_PRIMARY
	"SecondaryWeapons",	// WEAPONTYPE_SECONDARY
	"Equipment",		// WEAPONTYPE_EQUIPMENT
	NULL,				// WEAPONTYPE_MELEE
};

//=========================================================
//=========================================================
bool CTeamLookup::AddClass( const DefinedPlayerClass_t &DefinedPlayerClass )
{
	const char *pszFileName = DefinedPlayerClass.m_pszName;

	// parse file path
	char szFilePath[ 256 ];
	Q_snprintf( szFilePath, sizeof( szFilePath ), "scripts/pclasses/%s_%s", m_pszFileName, pszFileName );

	KeyValues *pClass = ReadEncryptedKVFile( ::filesystem, szFilePath, GetEncryptionKey( ) );

	if( !pClass )
		return false;

	LoadClass( DefinedPlayerClass, pClass );

	pClass->deleteThis( );

	return true;
}

//=========================================================
//=========================================================
void CTeamLookup::LoadClass( const DefinedPlayerClass_t &DefinedPlayerClass, KeyValues *pClass )
{
	if( pClass->IsEmpty( ) )
		return;

	// find the name and description
	const char *pszName, *pszDescription, *pszClassFileName, *pszModel;

	pszName = pClass->GetString( "name", NULL );
	pszDescription = pClass->GetString( "desc", NULL );
	pszClassFileName = pClass->GetString( "filename", NULL );
	pszModel = pClass->GetString( "model", NULL );

	if( !pszName || !pszDescription || !pszClassFileName )
		return;

	CPlayerClass *pPlayerClass = new CPlayerClass( );
	pPlayerClass->SetName( pszName );
	pPlayerClass->SetDescription( pszDescription );
	pPlayerClass->SetFileName( pszClassFileName );
	pPlayerClass->SetModel( pszModel );
	pPlayerClass->SetType( DefinedPlayerClass.m_iType );

	KeyValues *pInventoryData = pClass->FindKey( "InventoryData" );
	Assert( pInventoryData );

	pPlayerClass->ParseInventory( pInventoryData );

	// finished parsing
	m_Classes.AddToTail( pPlayerClass );
}

//=========================================================
//=========================================================
bool CTeamLookup::IsValidClass( int iClassID )
{
	return m_Classes.IsValidIndex( iClassID );
}

//=========================================================
//=========================================================
CPlayerClass *CTeamLookup::GetClass( int iClassID )
{
	if( iClassID == INVALID_CLASS || !m_Classes.IsValidIndex( iClassID ) )
		return NULL;

	return m_Classes[ iClassID ];
}

//=========================================================
//=========================================================
int CTeamLookup::FindClass( const char *pszClassName )
{
	for( int i = 0; i < m_Classes.Count( ); i++ )
	{
		CPlayerClass *pClass = m_Classes[ i ];

		if( pClass && Q_stricmp( pszClassName, pClass->GetName( ) ) == 0 )
			return i;
	}

	return INVALID_CLASS;
}

//=========================================================
//=========================================================
void CTeamLookup::InitCommanderClass( void )
{
	for( int i = 0; i < m_Classes.Count( ); i++ )
	{
		CPlayerClass *pClass = m_Classes[ i ];

		if( pClass && pClass->GetType( ) == CLASSTYPE_COMMANDER )
		{
			m_iCommanderClassID = i;
			return;
		}
	}

	AssertMsg( false, "CTeamLookup::InitCommanderClass, Unable to Find Commander Class" );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CPlayerInventoryBlueprint::Write( CINSPlayer *pPlayer ) const
{
	WriteWeapon( pPlayer, WEAPONTYPE_PRIMARY );
	WriteWeapon( pPlayer, WEAPONTYPE_SECONDARY );
	
	for( int i = 0; i < m_Equipment.Count( ); i++ )
	{
		const ItemEquipmentData_t &EquipmentData = m_Equipment[ i ];
		pPlayer->AddEquipment( EquipmentData.m_iID, EquipmentData.m_iCount );
	}
}

//=========================================================
//=========================================================
void CPlayerInventoryBlueprint::WriteWeapon( CINSPlayer *pPlayer, int iType ) const
{
	const CUtlVector< ItemWeaponData_t > &Weapons = m_Weapons[ iType ];
	int iWeaponsCount = Weapons.Count( );

	if( iWeaponsCount == 0 )
		return;

	int iID = pPlayer->GetLayoutCustomisation( iType );

	if( !Weapons.IsValidIndex( iID ) )
		iID = random->RandomInt( 0, iWeaponsCount - 1 );

	const ItemWeaponData_t &WeaponData = Weapons[ iID ];
	pPlayer->AddWeapon( WeaponData.m_iID, WeaponData.m_iClipCount, WeaponData.m_iAmmoCount );
}

#endif

//=========================================================
//=========================================================
CPlayerClass::CPlayerClass( )
{
	m_pPlayerModelData = NULL;
	m_pszDescription = NULL;
	m_iType = CLASSTYPE_INVALID;
}

//=========================================================
//=========================================================
CPlayerClass::~CPlayerClass( )
{
	delete m_pPlayerModelData;
}

//=========================================================
//=========================================================
void CPlayerClass::Precache( void )
{
	if( m_pPlayerModelData )
		CBaseEntity::PrecacheModel( m_pPlayerModelData->GetPath( ) );
}

//=========================================================
//=========================================================
void CPlayerClass::SetupCustomization( void )
{
	if( m_pPlayerModelData )
		m_pPlayerModelData->SetupCustomization( );
}

//=========================================================
//=========================================================
void CPlayerClass::SetName(const char *pszName)
{
	Q_strncpy(m_szName, pszName, sizeof(IMCString));
}

//=========================================================
//=========================================================
void CPlayerClass::SetFileName(const char *pszFileName)
{
	Q_strncpy(m_szFileName, pszFileName, sizeof(IMCString));
}

//=========================================================
//=========================================================
void CPlayerClass::SetModel( const char *pszModel )
{
	if( pszModel && *pszModel != '/0' )
	{
		m_pPlayerModelData = new CPlayerModelData( );
		m_pPlayerModelData->SetPath( pszModel );
	}
}

//=========================================================
//=========================================================
void CPlayerClass::ParseInventory( KeyValues *pInventoryData )
{
	KeyValues *pInventory = pInventoryData->GetFirstTrueSubKey( );

	for( ; pInventory; pInventory = pInventory->GetNextKey( ) )
	{
		const char *pszName = pInventory->GetName( );

		if( !pszName || *pszName == '\0' )
			continue;

		// find the item type
		int iWeaponType = WEAPONTYPE_INVALID;

		for( int i = 0; i < WEAPONTYPE_COUNT; i++ )
		{
			if( i == WEAPONTYPE_MELEE )
				continue;

			if( g_pszWeaponTypes[ i ] && Q_stricmp( pszName, g_pszWeaponTypes[ i ] ) == 0 )
			{
				iWeaponType = i;
				break;
			}
		}

		if( iWeaponType == WEAPONTYPE_INVALID )
			continue;

		// parse the item
		if( iWeaponType == WEAPONTYPE_EQUIPMENT )
			ParseEquipment( pInventory );
		else
			ParseWeapons( pInventory, iWeaponType );
	}
}

//=========================================================
//=========================================================
void CPlayerClass::ParseWeapons( KeyValues *pWeaponData, int iWeaponType )
{
	const char *pszWeaponName = NULL;
	int iWeaponID = WEAPONTYPE_INVALID;

	for( KeyValues *pWeapon = pWeaponData->GetFirstSubKey( ); pWeapon; pWeapon = pWeapon->GetNextKey( ) )
	{
		pszWeaponName = pWeapon->GetString( "name" );

		if( !pszWeaponName || *pszWeaponName == '\0' )
			continue;

		if( !LookupWeaponIDType( pszWeaponName, iWeaponID ) )
			continue;

		if( WeaponIDToType( iWeaponID ) != iWeaponType )
			continue;

		m_Blueprint.AddWeapon( iWeaponID, pWeapon->GetInt( "clips" ), pWeapon->GetInt( "ammocount" ) );
	}
}

//=========================================================
//=========================================================
void CPlayerClass::ParseEquipment( KeyValues *pAmmoData )
{
	const char *pszItemName = NULL;
	int iItemID = INVALID_ITEM;

	for( KeyValues *pWeapon = pAmmoData->GetFirstSubKey( ); pWeapon; pWeapon = pWeapon->GetNextKey( ) )
	{
		pszItemName = pWeapon->GetString( "name" );

		if( !pszItemName || *pszItemName == '\0' )
			continue;
		
		if( !GetItemID( pszItemName, iItemID ) )
			continue;

		m_Blueprint.AddEquipment( iItemID, pWeapon->GetInt( "count" ) );
	}
}

//=========================================================
//=========================================================
void SetupTeamLookup(void)
{
	for(int i = 0; i < MAX_LOOKUP_TEAMS; i++)
	{
		AddTeam_t AddTeam = g_CreateTeamLookup[i];

		if(!AddTeam)
		{
			AssertMsg(0, "Team has not been Allocated");
			continue; // return false?
		}

		// setup basic team
		CTeamLookup *pTeamLookup = new CTeamLookup(i);
		AddTeam(pTeamLookup);

		// setup classes in team
		CPlayerClassHelper *pPlayerClassHelper = g_TeamClassLookup[i];

		if(!pPlayerClassHelper)
		{
			AssertMsg(0, "Team has no Defined Classes");
			continue; // return false?
		}

		for(int i = 0; i < pPlayerClassHelper->GetClassCount(); i++)
		{
			if(!pTeamLookup->AddClass(pPlayerClassHelper->GetClass(i)))
				AssertMsg(false, "CTeamLookup::AddClass failed");
		}

		// work out commander classes for quick lookup
		pTeamLookup->InitCommanderClass();
	}
}

//=========================================================
//=========================================================
//deathz0rz [
void SetupTeamLookup_VidInit(void) //level init actually
{
/*
	static bool bDoneSetupTeamLookup_VidInit=false;
	if (bDoneSetupTeamLookup_VidInit)
		return;
*/

	for(int i = 0; i < MAX_LOOKUP_TEAMS; i++)
		g_pTeamLookup[i]->Precache();

	for(int i = 0; i < MAX_LOOKUP_TEAMS; i++)
		g_pTeamLookup[i]->SetupCustomization();

/*
	bDoneSetupTeamLookup_VidInit=true;
*/
}
//deathz0rz ]

//=========================================================
//=========================================================
void ClearTeamLookup( void )
{
	for( int i = 0; i < MAX_LOOKUP_TEAMS; i++ )
		delete g_pTeamLookup[ i ];
}

//=========================================================
//=========================================================
CTeamLookup *GetTeamLookup( int iTeamClassID )
{
	if( iTeamClassID < 0 || iTeamClassID >= MAX_LOOKUP_TEAMS )
		return NULL;

	return g_pTeamLookup[ iTeamClassID ];
}
