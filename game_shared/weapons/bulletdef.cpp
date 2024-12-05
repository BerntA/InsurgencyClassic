//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "bulletdef.h"
#include "keyvalues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
DEFINE_STRING_LOOKUP_CONSTANTS( int, bullettypes )

	ADD_LOOKUP( BULLET_556NATO )
	ADD_LOOKUP( BULLET_762NATO )
	ADD_LOOKUP( BULLET_9MMNATO )
	ADD_LOOKUP( BULLET_762RUSSIAN )
	ADD_LOOKUP( BULLET_545RUSSIAN )
	ADD_LOOKUP( BULLET_45 )
	ADD_LOOKUP( BULLET_303 )
	ADD_LOOKUP( BULLET_BUCKSHOT )

END_STRING_LOOKUP_CONSTANTS( )


//=========================================================
//=========================================================
float g_flBulletFormFactors[ BULLETFT_COUNT ] = {
	0.8f,	// BULLETFT_FLATNOSELEAD
	0.9f,	// BULLETFT_ROUNDNOSELEAD
	1.0f,	// BULLETFT_ROUNDNOSEJACKETED
	1.0f,	// BULLETFT_SEMIPOINTEDSOFTPOINT
	1.4f,	// BULLETFT_POINTEDSOFTPOINT
	1.75f,	// BULLETFT_POINTEDFULLJACKET
	2.0f,	// BULLETFT_POINTEDFULLJACKETBOATTAILED
};

//=========================================================
//=========================================================
CBulletData::CBulletData( )
{
	m_iMinSplashSize = 4;
	m_iMaxSplashSize = 6;

	m_iWeight = 0;
	m_flXArea = 0;
	m_iMuzzleVelocity = 0;
	m_flFormFactor = 0.0f;

	m_flPartPhysicsImpulse = 0.0f;

#ifdef CLIENT_DLL

	m_szShellModel[ 0 ] = '\0';
	m_pShellModel = NULL;

#endif
}

//=========================================================
//=========================================================
#define BULLET_IMPULSE_EXAGGERATION 3.5f
#define BULLET_IMPULSE( grams, mpersec ) ( 12.0f * ( grams * 0.001f ) * BULLET_IMPULSE_EXAGGERATION )

void CBulletData::Parse( KeyValues *pData )
{
	m_iWeight = pData->GetInt( "weight", 1 );
	m_flXArea = M_PI * pow( pData->GetFloat( "caliber", 1 ), 2 );
	m_iMuzzleVelocity = pData->GetInt( "vmuzzle", 1 );

	int iFormType;

	if( GetBulletFormType( pData->GetString( "formtype", "" ), iFormType ) )
		m_flFormFactor = g_flBulletFormFactors[ iFormType ];

	m_flPartPhysicsImpulse = BULLET_IMPULSE( m_iWeight, iMuzzleVelocity );

#ifdef CLIENT_DLL

	const char *pszShellModel = pData->GetString( "shell", NULL );

	if( pszShellModel )
		Q_strncpy( m_szShellModel, pszShellModel, MAX_WEAPON_STRING );

#endif
}

//=========================================================
//=========================================================
void CBulletData::LevelInit( void )
{
#ifdef CLIENT_DLL

	m_pShellModel = NULL;

	if( m_szShellModel && *m_szShellModel != '\0' )
		m_pShellModel = ( model_t* )engine->LoadModel( m_szShellModel );

#endif
}

//=========================================================
//=========================================================
DECLARE_AMMOTYPE( AMMOTYPE_BULLET, CBulletDef );

//=========================================================
//=========================================================
const CBulletData &CBulletDef::GetBulletData( int iType )
{
	CBulletDef *pBulletDef = ( CBulletDef* )( GetAmmoDef( )->GetAmmoType( AMMOTYPE_BULLET ) );
	return pBulletDef->m_Data[ iType ];
}

//=========================================================
//=========================================================
const char *CBulletDef::GetHeader( void ) const
{
	return "BulletData";
}

//=========================================================
//=========================================================
int CBulletDef::ConvertID( const char *pszID ) const
{
	int iBulletID;

	if( !LookupBulletType( pszID, iBulletID ) )
	{
		Warning( "CBulletDef::ConvertID, Unknown Bullet" );

		return INVALID_AMMODATA;
	}

	return iBulletID;
}

//=========================================================
//=========================================================
int CBulletDef::GetDataCount( void ) const
{
	return BULLET_COUNT;
}

//=========================================================
//=========================================================
IAmmoData *CBulletDef::GetData( int iID )
{
	return &m_Data[ iID ];
}
