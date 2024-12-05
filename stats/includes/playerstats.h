//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//=============================================================================//

#ifndef PLAYERSTATS_H
#define PLAYERSTATS_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_defines.h"

//=========================================================
//=========================================================
typedef unsigned int StatVar_t;

//=========================================================
//=========================================================
#define INVALID_STATSTYPE -1

//=========================================================
//=========================================================
enum PlayerStatsTypes_t
{
	PLAYERSTATS_INVALID = -1,

	// important
	PLAYERSTATS_GAMEPTS = 0,
	PLAYERSTATS_KILLPTS,

	// intresting
	PLAYERSTATS_KILLS,
	PLAYERSTATS_FKILLS,
	PLAYERSTATS_DEATHS,
	PLAYERSTATS_SUICIDES,
	PLAYERSTATS_OBJCAPS,
	PLAYERSTATS_MINUTES,

	// misc
	PLAYERSTATS_TALKED,

	PLAYERSTATS_COUNT
};

typedef StatVar_t PlayerStatsData_t[ PLAYERSTATS_COUNT ];

//=========================================================
//=========================================================
enum PlayerWeaponStatsTypes_t
{
	WEAPONSTATS_INVALID = -1,

	WEAPONSTATS_SHOTS = 0,
	WEAPONSTATS_FRAGS,

	WEAPONSTATS_HITS_HEAD,
	WEAPONSTATS_HITS_BODY,
	WEAPONSTATS_HITS_LARM,
	WEAPONSTATS_HITS_RARM,
	WEAPONSTATS_HITS_LLEG,
	WEAPONSTATS_HITS_RLEG,

	WEAPONSTATS_COUNT
};

typedef StatVar_t WeaponStatsData_t[ WEAPONSTATS_COUNT ];

//=========================================================
//=========================================================
class IStatsBase
{
public:
	virtual void Reset( void ) = 0;
	virtual void Reset( int iType ) = 0;

	// data management
	virtual int CountTypes( void ) const = 0;

	// data access
	virtual void SetType( int iType, int iValue ) = 0;
	virtual void Increment( int iType, int iIncrement ) = 0;
	virtual int GetValue( int iType ) const = 0;

	// typename management
	virtual const char *GetTypeName( int iType ) const = 0;
	int GetTypeID( const char *pszTypeName ) const;
};

//=========================================================
//=========================================================
template< class T >
class CStatsBase : public IStatsBase
{
public:
	CStatsBase( );

	// data management
	void Reset( void );
	void Reset( int iType );

	// data access
	void SetType( int iType, int iValue );
	void Increment( int iType, int iIncrement );
	int GetValue( int iType ) const;

	// helpers
	virtual void Copy( const CStatsBase &Stats );
	virtual void Combine( const CStatsBase &Stats );

protected:
	T m_Data;
};

template< class T >
CStatsBase< T >::CStatsBase( )
{
	Reset( );
}

template< class T >
void CStatsBase< T >::Reset( void )
{
	memset( &m_Data, 0, sizeof( m_Data ) );
}

template< class T >
void CStatsBase< T >::Reset( int iType )
{
	m_Data[ iType ] = 0;
}

template< class T >
void CStatsBase< T >::SetType( int iType, int iValue )
{
	m_Data[ iType ] = iValue;
}

template< class T >
void CStatsBase< T >::Increment( int iType, int iIncrement )
{
	m_Data[ iType ] += iIncrement;
}

template< class T >
int CStatsBase< T >::GetValue( int iType ) const
{
	return m_Data[ iType ];
}

template< class T >
void CStatsBase< T >::Copy( const CStatsBase &Stats )
{
	memcpy( &m_Data, Stats.m_Data, sizeof( m_Data ) );
}

template< class T >
void CStatsBase< T >::Combine( const CStatsBase &Stats )
{
	for( int i = 0; i < CountTypes( ); i++ )
		m_Data[ i ] += Stats.m_Data[ i ];
}

//=========================================================
//=========================================================
class CWeaponStats : public CStatsBase< WeaponStatsData_t >
{
public:
	int CountTypes( void ) const;

	const char *GetTypeName( int iType ) const;
};

//=========================================================
//=========================================================
class CPlayerStats : public CStatsBase< PlayerStatsData_t >
{
public:
	int CountTypes( void ) const;

	void Copy( const CPlayerStats &Stats );
	void Combine( const CPlayerStats &Stats );
	void Combine( const CWeaponStats &WeaponStats, int iWeaponID );

	 const char *GetTypeName( int iType ) const;

	 CWeaponStats &GetWeaponStats( int iWeaponID ) { return m_WeaponStats[ iWeaponID ]; }
	 const CWeaponStats &GetWeaponStats( int iWeaponID ) const { return m_WeaponStats[ iWeaponID ]; }

private:
	CWeaponStats m_WeaponStats[ MAX_WEAPONS ];
};

#endif // PLAYERSTATS_H
