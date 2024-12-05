//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the player specific data that is sent only to the player
//			to whom it belongs.
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_PLAYERLOCALDATA_H
#define C_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "basetypes.h"
#include "vector.h"
#include "playernet_vars.h"

//=========================================================
//=========================================================
class CPlayerLocalData
{
public:
	DECLARE_PREDICTABLE( );
	DECLARE_CLASS_NOBASE( CPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR( );

public:
	CPlayerLocalData( ) :
		m_iv_vecPunchAngle( "CPlayerLocalData::m_iv_vecPunchAngle" ),
		m_iv_vecRecoilPunchAngle( "CPlayerLocalData::m_iv_vecRecoilPunchAngle" )
	{
		m_iv_vecPunchAngle.Setup( &m_vecPunchAngle.m_Value, LATCH_SIMULATION_VAR );
		m_iv_vecRecoilPunchAngle.Setup( &m_vecRecoilPunchAngle.m_Value, LATCH_SIMULATION_VAR );

		m_iDefaultFOV = 0;

		m_flFOVRate = m_flViewmodelFOVRate = 0.0f;
	}

public:

	unsigned char m_chAreaBits[ 32 ];
	unsigned char m_chAreaPortalBits[ MAX_AREA_PORTAL_STATE_BYTES ];

	int m_iDefaultFOV;

    float m_flFOVRate;
	float m_flViewmodelFOVRate;

	int m_iScopeFOV;

	float m_flJumpTime;
	int m_nStepside;
	float m_flFallVelocity;
	int m_nOldButtons;
	bool m_bDrawViewmodel;

	float m_flStepSize;
	bool m_bAllowAutoMovement;

    CNetworkQAngle( m_vecPunchAngle );
    CInterpolatedVar<QAngle> m_iv_vecPunchAngle;
    CNetworkQAngle( m_vecPunchAngleVel );

    CNetworkQAngle( m_vecRecoilPunchAngle );
    CInterpolatedVar<QAngle> m_iv_vecRecoilPunchAngle;
    CNetworkQAngle( m_vecRecoilPunchAngleVel );

	// 3d skybox
	sky3dparams_t m_skybox3d;

	// wold fog
	fogparams_t m_fog;

	// audio environment
	audioparams_t m_audio;
};

#endif // C_PLAYERLOCALDATA_H