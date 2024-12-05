//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifdef INS_DLL

#include "cbase.h"

#endif

#include "playerstats.h"

#ifdef INS_DLL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#else

#pragma warning( disable : 4311 )
#pragma warning( disable : 4312 )

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define Q_strcmp( str1, str2 ) strcmp( str1, str2 )
#define Assert( exp ) assert( exp )

#endif

//=========================================================
//=========================================================
int IStatsBase::GetTypeID( const char *pszTypeName ) const
{
	for( int i = 0; i < CountTypes( ); i++ )
	{
		if( Q_strcmp( pszTypeName, GetTypeName( i ) ) == 0 )
			return i;			
	}

	return INVALID_STATSTYPE;
}

//=========================================================
//=========================================================
int CWeaponStats::CountTypes( void ) const
{
	return WEAPONSTATS_COUNT;
}

//=========================================================
//=========================================================
const char *CWeaponStats::GetTypeName( int iType ) const
{
	static const char *pszWeaponStatsNames[ WEAPONSTATS_COUNT ] = {
		"shots",		// WEAPONSTATS_SHOTS
		"frags",		// WEAPONSTATS_FRAGS
		"hits_head",	// WEAPONSTATS_HITS_HEAD
		"hits_body",	// WEAPONSTATS_HITS_BODY
		"hits_larm",	// WEAPONSTATS_HITS_LARM
		"hits_rarm",	// WEAPONSTATS_HITS_RARM
		"hits_lleg",	// WEAPONSTATS_HITS_LLEG
		"hits_rleg",	// WEAPONSTATS_HITS_RLEG
	};

	return pszWeaponStatsNames[ iType ];
}

//=========================================================
//=========================================================
int CPlayerStats::CountTypes( void ) const
{
	return PLAYERSTATS_COUNT;
}

//=========================================================
//=========================================================
void CPlayerStats::Copy( const CPlayerStats &Stats )
{
	CStatsBase< PlayerStatsData_t >::Copy( Stats );

	for( int i = 0; i < MAX_WEAPONS; i++ )
		m_WeaponStats[ i ].Copy( Stats.m_WeaponStats[ i ] );
}

//=========================================================
//=========================================================
void CPlayerStats::Combine( const CPlayerStats &Stats )
{
	CStatsBase< PlayerStatsData_t >::Combine( Stats );

	for( int i = 0; i < MAX_WEAPONS; i++ )
		m_WeaponStats[ i ].Combine( Stats.m_WeaponStats[ i ] );
}

//=========================================================
//=========================================================
void CPlayerStats::Combine( const CWeaponStats &WeaponStats, int iWeaponID )
{
	m_WeaponStats[ iWeaponID ].Combine( WeaponStats );
}

//=========================================================
//=========================================================
const char *CPlayerStats::GetTypeName( int iType ) const
{
	static const char *pszPlayerStatsNames[ PLAYERSTATS_COUNT ] = {
		"gamepts",		// PLAYERSTATS_GAMEPTS
		"killpts",		// PLAYERSTATS_KILLPTS
		"kills",		// PLAYERSTATS_KILLS
		"fkills",		// PLAYERSTATS_FKILLS
		"deaths",		// PLAYERSTATS_DEATHS
		"suicides",		// PLAYERSTATS_SUICIDES
		"objcaps",		// PLAYERSTATS_OBJCAPS
		"hours",		// PLAYERSTATS_HOURS
		"talked",		// PLAYERSTATS_TALKED
	};

	return pszPlayerStatsNames[ iType ];
}
