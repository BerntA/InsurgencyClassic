//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef PLAYERLOCALDATA_H
#define PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "playernet_vars.h"
#include "networkvar.h"

//=========================================================
//=========================================================
class CSkyCamera;

//=========================================================
//=========================================================
class CPlayerLocalData
{
public:
	DECLARE_SIMPLE_DATADESC( );
	DECLARE_CLASS_NOBASE( CPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR( );

public:
	CPlayerLocalData( );

	void UpdateAreaBits( CBasePlayer *pl, unsigned char chAreaPortalBits[ MAX_AREA_PORTAL_STATE_BYTES ] );

public:

	// which areas are potentially visible to the client?
	CNetworkArray( unsigned char, m_chAreaBits, MAX_AREA_STATE_BYTES );

	// which area portals are open?
	CNetworkArray( unsigned char, m_chAreaPortalBits, MAX_AREA_PORTAL_STATE_BYTES );

	// player FOV
	CNetworkVar( int, m_iDefaultFOV );
	CNetworkVar( float, m_flFOVRate );
	CNetworkVar( float, m_flViewmodelFOVRate );
	CNetworkVar( int, m_iScopeFOV );

	// punch angle
    CNetworkQAngle( m_vecPunchAngle );
    CNetworkQAngle( m_vecPunchAngleVel );

	CNetworkQAngle( m_vecRecoilPunchAngle );		
	CNetworkQAngle( m_vecRecoilPunchAngleVel );

	// jump time
	CNetworkVar( float, m_flJumpTime );

	// step sound side flip / flip
	int m_nStepside;

	// velocity at time when we hit ground
	CNetworkVar( float, m_flFallVelocity );

	// previous button state
	int m_nOldButtons;
	CSkyCamera *m_pOldSkyCamera;

	// draw view model for the player
	CNetworkVar( bool, m_bDrawViewmodel );

	CNetworkVar( float, m_flStepSize );
	CNetworkVar( bool, m_bAllowAutoMovement );

	// 3d skybox
	CNetworkVarEmbedded( sky3dparams_t, m_skybox3d );

	// wold fog
	CNetworkVarEmbedded( fogparams_t, m_fog );

	// audio environment
	CNetworkVarEmbedded( audioparams_t, m_audio );
};

//=========================================================
//=========================================================
EXTERN_SEND_TABLE( DT_Local );

#endif // PLAYERLOCALDATA_H