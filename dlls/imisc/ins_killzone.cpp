//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_killzone.h"
#include "ins_player_shared.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// TODO: add in the whole "might not die" functionailty

//=========================================================
//=========================================================
BEGIN_DATADESC( CKillZone )

	DEFINE_FUNCTION( PlayerThink ),

END_DATADESC( )

//=========================================================
//=========================================================
bool CKillZone::Init( void )
{
	// TODO: work out think length

	return true;
}

//=========================================================
//=========================================================
void CKillZone::PlayerThink( void )
{
	for( int i = 0; i < m_Players.Count(); i++ )
	{
		CINSPlayer *pPlayer = m_Players[ i ];

		if( !pPlayer )
			continue;

		for( int j = 0; j < m_PlaneData.Count(); j++ )
		{
			ZonedPlaneData_t &PlaneData = m_PlaneData[ j ];
			
			float flDistance = GetInternalPlaneDistance( PlaneData, pPlayer->GetAbsOrigin() );

			if( PlaneData.m_flPerpLength == 0 || flDistance > PlaneData.m_flPerpLength )
				continue;

			// kill player if too close
			float flDistFrac = flDistance / PlaneData.m_flPerpLength;

			if( flDistFrac > random->RandomFloat( 0.75f, 0.1f ) )
				continue;

			KillPlayer( pPlayer, PlaneData.m_vecCentre );
		}
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//=========================================================
//=========================================================
void CKillZone::PlayerStartTouch( CINSPlayer *pPlayer )
{
	BaseClass::PlayerStartTouch( pPlayer );

	m_Players.AddToTail( pPlayer );

	if( !m_pfnThink )
	{
		SetThink( &CKillZone::PlayerThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

//=========================================================
//=========================================================
void CKillZone::PlayerEndTouch( CINSPlayer *pPlayer )
{
	m_Players.FindAndRemove( pPlayer );

	if( m_Players.Count() <= 0 )
		SetThink( NULL );
}

//=========================================================
//=========================================================
void CKillZone::FoundProtectedSurface( Vector &vecNormal, float flPerpLength, Vector &vecCentre )
{
	int iPlaneDataID = m_PlaneData.AddToTail( );

	if( !m_PlaneData.IsValidIndex( iPlaneDataID ) )
		return;

	ZonedPlaneData_t &PlaneData = m_PlaneData[ iPlaneDataID ];

	PlaneData.m_vecNormal = vecNormal;
	PlaneData.m_vecCentre = vecCentre;
	PlaneData.m_flPerpLength = flPerpLength;
}
