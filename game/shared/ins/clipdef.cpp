//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Base combat character with no AI
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "clipdef.h"
#include "keyvalues.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define PATH_CLIPDATA "scripts/weapons/clipdata"

//=========================================================
//=========================================================
DEFINE_STRING_LOOKUP_CONSTANTS( int, cliptypeids )

	ADD_LOOKUP( CLIP_M16 )
	ADD_LOOKUP( CLIP_M9 )
	ADD_LOOKUP( CLIP_M14 )
	ADD_LOOKUP( CLIP_M249 )
	ADD_LOOKUP( CLIP_AK47 )
	ADD_LOOKUP( CLIP_AKS74U )
	ADD_LOOKUP( CLIP_RPK )
	ADD_LOOKUP( CLIP_UZI )
	ADD_LOOKUP( CLIP_MAKAROV )
	ADD_LOOKUP( CLIP_FNFAL )
	ADD_LOOKUP( CLIP_SKS )
	ADD_LOOKUP( CLIP_L42A1 )
	ADD_LOOKUP( CLIP_M1014 )
	ADD_LOOKUP( CLIP_TOZ )

END_STRING_LOOKUP_CONSTANTS( )

//=========================================================
//=========================================================
CClipDef CClipDef::m_ClipDef;

//=========================================================
//=========================================================
CClipDef::CClipDef( )
{
}

//=========================================================
//=========================================================
bool CClipDef::Init( void )
{
	if( !Load( ) )
	{
		Assert( false );
		Warning( "CClipDef::GetClipDef, Unable to Load Clip Definitions\n" );
	}

	return true;
}

//=========================================================
//=========================================================
bool CClipDef::Load( void )
{
	KeyValues *pClipDef = ReadEncryptedKVFile(filesystem, PATH_CLIPDATA, GetEncryptionKey( ) );

	if( !pClipDef )
		return false;

	for( KeyValues *pClipType = pClipDef->GetFirstSubKey( ); pClipType; pClipType = pClipType->GetNextKey( ) )
	{
		const char *pszClip = pClipType->GetName( );

		if( !pszClip || *pszClip == '\0' )
			continue;

		int iClipID;

		// find cliptype
		if( !LookupClipTypeID( pszClip, iClipID ) )
			continue;

		Clip_t &Clip = m_ClipData[ iClipID ];

		// get weapontype
		const char *pszWeaponType = pClipType->GetString( "weapon" );

		if( !pszWeaponType || *pszWeaponType == '\0' )
			continue;

		if( !LookupWeaponTypeID( pszWeaponType, Clip.m_iWeaponType ) )
			continue;

		// get ammo type
		const char *pszBulletType = pClipType->GetString( "bullet" );

		if( !pszBulletType || *pszBulletType == '\0' )
			continue;

		if( !LookupBulletType( pszBulletType, Clip.m_iBulletType ) )
			continue;

		// get bullets
		Clip.m_iBullets = clamp( pClipType->GetInt( "size", 1 ), 1, MAX_CLIPSIZE );
	}

	pClipDef->deleteThis( );

	return true;
}

//=========================================================
//=========================================================
CClipDef *CClipDef::GetClipDef( void )
{
	return &CClipDef::m_ClipDef;
}

//=========================================================
//=========================================================
CClipDef *GetClipDef( void )
{
	return CClipDef::GetClipDef( );
}