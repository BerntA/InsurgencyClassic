//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapon data file parsing, shared by game & client dlls.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "weapon_parse.h"
#include <keyvalues.h>
#include <tier0/mem.h>
#include "filesystem.h"
#include "utldict.h"
#include "ammodef.h"
#include "weapon_defines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// the sound categories found in the weapon classname.txt files
const char *pWeaponSoundCategories[NUM_SHOOT_SOUND_TYPES] = 
{
	"empty",			// EMPTY
	"shot_single",		// SHOT_SINGLE,
	"shot_double",		// SHOT_DOUBLE,
	"shot_burst",		// SHOT_BURST,
	"reload",			// RELOAD,
	"melee_miss",		// MELEE_MISS,
	"melee_hit",		// MELEE_HIT,
	"melee_hit_world",	// MELEE_HIT_WORLD,
	"special1",			// SPECIAL1,
	"special2",			// SPECIAL2,
	"special3"			// SPECIAL3,
};

static CUtlDict<FileWeaponInfo_t*, unsigned short> m_WeaponInfoDatabase;

//=========================================================
//=========================================================
static WEAPON_FILE_INFO_HANDLE FindWeaponInfoSlot( const char *name )
{
	// complain about duplicately defined metaclass names...
	unsigned short lookup = m_WeaponInfoDatabase.Find( name );
	if ( lookup != m_WeaponInfoDatabase.InvalidIndex() )
	{
		return lookup;
	}

	// try and find it in file info lookups
	FileWeaponInfo_t *insert = NULL;

	for(int i = 0; i < MAX_WEAPONS && !insert; i++)
	{
		const char *pszWeaponName = WeaponIDToName(i);

		if(!pszWeaponName)
			continue;

		if(Q_strncmp(name, pszWeaponName, MAX_WEAPON_STRING) != 0)
			continue;

		insert = (*g_WeaponDataHelpers[i].m_WeaponDataCreate)();
	}
	
	// ... otherwise, create default
	if(!insert)
		insert = new FileWeaponInfo_t;

	lookup = m_WeaponInfoDatabase.Insert( name, insert );
	Assert( lookup != m_WeaponInfoDatabase.InvalidIndex() );
	return lookup;
}

//=========================================================
//=========================================================
WEAPON_FILE_INFO_HANDLE LookupWeaponInfoSlot( const char *name )
{
	// find a weapon slot, assuming the weapon's data has already been loaded.
	return m_WeaponInfoDatabase.Find( name );
}

//=========================================================
//=========================================================
static FileWeaponInfo_t gNullWeaponInfo;

FileWeaponInfo_t *GetFileWeaponInfoFromHandle( WEAPON_FILE_INFO_HANDLE handle )
{
	if ( handle < 0 || handle >= m_WeaponInfoDatabase.Count() )
	{
		return &gNullWeaponInfo;
	}

	if ( handle == m_WeaponInfoDatabase.InvalidIndex() )
	{
		return &gNullWeaponInfo;
	}

	return m_WeaponInfoDatabase[ handle ];
}

//=========================================================
//=========================================================
WEAPON_FILE_INFO_HANDLE GetInvalidWeaponInfoHandle( void )
{
	return (WEAPON_FILE_INFO_HANDLE)m_WeaponInfoDatabase.InvalidIndex();
}

//=========================================================
//=========================================================
KeyValues* ReadEncryptedKVFile( IFileSystem *filesystem, const char *szFilenameWithoutExtension, const unsigned char *pICEKey )
{
	Assert( strchr( szFilenameWithoutExtension, '.' ) == NULL );
	char szFullName[512];

	const char *pSearchPath = "MOD";

	if ( pICEKey == NULL )
	{
		pSearchPath = "GAME";
	}

	// Open the weapon data file, and abort if we can't
	KeyValues *pKV = new KeyValues( "WeaponDatafile" );

	Q_snprintf(szFullName,sizeof(szFullName), "%s.txt", szFilenameWithoutExtension);

	if ( !pKV->LoadFromFile( filesystem, szFullName, pSearchPath ) ) // try to load the normal .txt file first
	{
		if ( pICEKey )
		{
			Q_snprintf(szFullName,sizeof(szFullName), "%s.ctx", szFilenameWithoutExtension); // fall back to the .ctx file

			FileHandle_t f = filesystem->Open( szFullName, "rb", pSearchPath );

			if (!f)
			{
				pKV->deleteThis();
				return NULL;
			}
			// load file into a null-terminated buffer
			int fileSize = filesystem->Size(f);
			char *buffer = (char*)MemAllocScratch(fileSize + 1);
		
			Assert(buffer);
		
			filesystem->Read(buffer, fileSize, f); // read into local buffer
			buffer[fileSize] = 0; // null terminate file as EOF
			filesystem->Close( f );	// close file after reading

			UTIL_DecodeICE( (unsigned char*)buffer, fileSize, pICEKey );

			bool retOK = pKV->LoadFromBuffer( szFullName, buffer, filesystem );

			MemFreeScratch();

			if ( !retOK )
			{
				pKV->deleteThis();
				return NULL;
			}
		}
		else
		{
				pKV->deleteThis();
				return NULL;
		}
	}

	return pKV;
}


//=========================================================
//=========================================================
bool ReadWeaponDataFromFileForSlot( IFileSystem* filesystem, const char *szWeaponName, WEAPON_FILE_INFO_HANDLE *phandle, const unsigned char *pICEKey )
{
	if ( !phandle )
	{
		Assert( 0 );
		return false;
	}
	
	*phandle = FindWeaponInfoSlot( szWeaponName );
	FileWeaponInfo_t *pFileInfo = GetFileWeaponInfoFromHandle( *phandle );
	Assert( pFileInfo );

	// Pongles [

	if( pFileInfo->bParsedScript || !pFileInfo->ShouldParse( ) )
		return true;

	// Pongles ]

	char sz[128];
	Q_snprintf( sz, sizeof( sz ), "scripts/weapons/%s", szWeaponName );
	KeyValues *pKV = ReadEncryptedKVFile( filesystem, sz, pICEKey );
	if ( !pKV )
		return false;

	pFileInfo->Parse( pKV, szWeaponName );

	pKV->deleteThis();

	return true;
}

//=========================================================
//=========================================================
FileWeaponInfo_t::FileWeaponInfo_t()
{
	bParsedScript = false;

	szClassName[0] = 0;
	szViewModel[0] = 0;
	szWorldModel[0] = 0;
	szAnimationSuffix[0] = 0;

	flWeight = 0.0f;

	memset(aShootSounds, 0, sizeof(aShootSounds));
}

//=========================================================
//=========================================================
void FileWeaponInfo_t::Parse( KeyValues *pKeyValuesData, const char *szWeaponName )
{
	// okay, we tried at least once to look this up...
	bParsedScript = true;

	Q_strncpy( szClassName, szWeaponName, MAX_WEAPON_STRING );
	Q_strncpy( szViewModel, pKeyValuesData->GetString( "viewmodel" ), MAX_WEAPON_STRING );
	Q_strncpy( szWorldModel, pKeyValuesData->GetString( "playermodel" ), MAX_WEAPON_STRING );
	Q_strncpy( szAnimationSuffix, pKeyValuesData->GetString( "anim_suffix" ), MAX_WEAPON_PREFIX );

	// find weight
	flWeight = pKeyValuesData->GetFloat( "weight", NULL );

	// now read the weapon sounds
	memset( aShootSounds, 0, sizeof( aShootSounds ) );
	KeyValues *pSoundData = pKeyValuesData->FindKey( "SoundData" );
	if ( pSoundData )
	{
		for ( int i = EMPTY; i < NUM_SHOOT_SOUND_TYPES; i++ )
		{
			const char *soundname = pSoundData->GetString( pWeaponSoundCategories[i] );
			if ( soundname && soundname[0] )
			{
				Q_strncpy( aShootSounds[i], soundname, MAX_WEAPON_STRING );
			}
		}
	}
}