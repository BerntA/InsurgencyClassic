//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "missiledef.h"
#include "keyvalues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
DEFINE_STRING_LOOKUP_CONSTANTS( int, missileids )

	ADD_LOOKUP( MISSILE_OG7V )
	ADD_LOOKUP( MISSILE_M406 )

END_STRING_LOOKUP_CONSTANTS( )

//=========================================================
//=========================================================
const char *g_pszMissileList[ MISSILE_COUNT ];

//=========================================================
//=========================================================
CMissileData::CMissileData( )
{
	m_iSafetyRange = 0;
}

//=========================================================
//=========================================================
void CMissileData::Parse( KeyValues *pData )
{
	BaseClass::Parse( pData );

	// convert to meters
	m_iSafetyRange = RoundFloatToInt( pData->GetFloat( "safety_range", 1 ) * INCHS_PER_METER );
}

//=========================================================
//=========================================================
const char *CMissileDef::GetHeader( void ) const
{
	return "MissileData";
}

//=========================================================
//=========================================================
DECLARE_AMMOTYPE( AMMOTYPE_MISSILE, CMissileDef );

//=========================================================
//=========================================================
const CMissileData &CMissileDef::GetMissileData( int iType )
{
	CMissileDef *pMissileDef = ( CMissileDef* )( GetAmmoDef( )->GetAmmoType( AMMOTYPE_MISSILE ) );
	return pMissileDef->m_Data[ iType ];
}

//=========================================================
//=========================================================
int CMissileDef::ConvertID( const char *pszID ) const
{
	int iMissileID;

	if( !LookupMissileType( pszID, iMissileID ) )
	{
		Warning( "CMissileDef::ConvertID, Unknown Missile" );

		return INVALID_AMMODATA;
	}

	return iMissileID;
}

//=========================================================
//=========================================================
int CMissileDef::GetDataCount( void ) const
{
	return MISSILE_COUNT;
}

//=========================================================
//=========================================================
IAmmoData *CMissileDef::GetData( int iID )
{
	return &m_Data[ iID ];
}