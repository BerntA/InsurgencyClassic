//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "basetempentity.h"

//=========================================================
//=========================================================
#define NUM_BULLET_SEED_BITS 8

//=========================================================
//=========================================================
class CTEFireBullets : public CBaseTempEntity
{
	DECLARE_CLASS( CTEFireBullets, CBaseTempEntity );
	DECLARE_SERVERCLASS( );

public:
	CTEFireBullets( const char *pszName );
	~CTEFireBullets( void );

public:
	CNetworkVar( int, m_iPlayer );		// player who fired
	CNetworkVector( m_vecOrigin );		// firing origin
	CNetworkVector( m_vecDirection );	// firing direction
	CNetworkVar( int, m_iSeed );		// shared random seed
};

//=========================================================
//=========================================================
CTEFireBullets::CTEFireBullets( const char *pszName ) :
	CBaseTempEntity( pszName )
{
}

//=========================================================
//=========================================================
CTEFireBullets::~CTEFireBullets( )
{
}

//=========================================================
//=========================================================
IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEFireBullets, DT_TEFireBullets )

	SendPropInt( SENDINFO( m_iPlayer ), 5, SPROP_UNSIGNED ), 	// max 32 players, see MAX_PLAYERS
	SendPropVector( SENDINFO( m_vecOrigin ), -1, SPROP_COORD ),
	SendPropVector( SENDINFO( m_vecDirection ), -1 ),
	SendPropInt( SENDINFO( m_iSeed ), NUM_BULLET_SEED_BITS, SPROP_UNSIGNED ),

END_SEND_TABLE( )

//=========================================================
//=========================================================
static CTEFireBullets g_TEFireBullets( "FireBullets" );

//=========================================================
//=========================================================
void TE_FireBullets( int iPlayerIndex, const Vector &vecOrigin, const Vector &vecDir, int iSeed )
{
	CPASFilter filter( vecOrigin );
	filter.UsePredictionRules( );

	g_TEFireBullets.m_iPlayer = iPlayerIndex;
	g_TEFireBullets.m_vecOrigin = vecOrigin;
	g_TEFireBullets.m_vecDirection = vecDir;
	g_TEFireBullets.m_iSeed = iSeed;
	
	Assert( iSeed < ( 1 << NUM_BULLET_SEED_BITS ) );
	
	g_TEFireBullets.Create( filter, 0 );
}