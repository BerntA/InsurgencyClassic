//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "explosivedef.h"
#include "keyvalues.h"
#include "ins_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CExplosiveData::CExplosiveData( )
{
	m_szModel[ 0 ] = '\0';

#ifdef GAME_DLL

	Q_strncpy( m_szDetonationSound, INS_EXPLOSION_DEFAULTSOUND, sizeof( m_szDetonationSound ) );
	m_bPrecacheDetonateSound = false;

	m_iDamage = 0;
	m_iDamageRadius = 0;

#endif
}

//=========================================================
//=========================================================
void CExplosiveData::Parse( KeyValues *pData )
{
	const char *pszModel = pData->GetString( "model", NULL );

	if( pszModel )
		Q_strncpy( m_szModel, pszModel, MAX_WEAPON_STRING );

#ifdef GAME_DLL

	const char *pszDetonationSound = pData->GetString( "detonate_sound", NULL );

	if( pszDetonationSound )
	{
		Q_strncpy( m_szDetonationSound, pszDetonationSound, sizeof( m_szDetonationSound ) );
		m_bPrecacheDetonateSound = true;
	}

	m_iDamage = pData->GetInt( "damage", 1 );
	m_iDamageRadius = pData->GetFloat( "damage_radius", 1 ) * INCHS_PER_METER;

#endif
}

//=========================================================
//=========================================================
void CExplosiveData::Precache( void )
{
	if( m_szModel[ 0 ] != '\0' )
		CBaseEntity::PrecacheModel( m_szModel );

#ifdef GAME_DLL

	if( m_bPrecacheDetonateSound )
		CBaseEntity::PrecacheScriptSound( m_szDetonationSound );

#endif
}