//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "grenadedef.h"
#include "keyvalues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
DEFINE_STRING_LOOKUP_CONSTANTS( int, grenadetypes )

	ADD_LOOKUP( GRENADE_M67 )
	ADD_LOOKUP( GRENADE_M18 )
	ADD_LOOKUP( GRENADE_RGD5 )
	ADD_LOOKUP( GRENADE_C4 )

END_STRING_LOOKUP_CONSTANTS( )

//=========================================================
//=========================================================
const char *g_pszGrenadeList[ GRENADE_COUNT ];

//=========================================================
//=========================================================
CGrenadeData::CGrenadeData( )
{
	m_flFuse = 0.0f;
}

//=========================================================
//=========================================================
void CGrenadeData::Parse( KeyValues *pData )
{
	BaseClass::Parse( pData );

	m_flFuse = pData->GetFloat( "fuse", 1.0f );
}

//=========================================================
//=========================================================
DECLARE_AMMOTYPE( AMMOTYPE_GRENADE, CGrenadeDef );

//=========================================================
//=========================================================
const CGrenadeData &CGrenadeDef::GetGrenadeData( int iType )
{
	CGrenadeDef *pGrenadeDef = ( CGrenadeDef* )( GetAmmoDef( )->GetAmmoType( AMMOTYPE_GRENADE ) );
	return pGrenadeDef->m_Data[ iType ];
}

//=========================================================
//=========================================================
const char *CGrenadeDef::GetHeader( void ) const
{
	return "GrenadeData";
}

//=========================================================
//=========================================================
int CGrenadeDef::ConvertID( const char *pszID ) const
{
	int iGrenadeID;

	if( !LookupGrenadeType( pszID, iGrenadeID ) )
	{
		Warning( "CGrenadeDef::ConvertID, Unknown Grenade" );

		return INVALID_AMMODATA;
	}

	return iGrenadeID;
}

//=========================================================
//=========================================================
int CGrenadeDef::GetDataCount( void ) const
{
	return GRENADE_COUNT;
}

//=========================================================
//=========================================================
IAmmoData *CGrenadeDef::GetData( int iID )
{
	return &m_Data[ iID ];
}