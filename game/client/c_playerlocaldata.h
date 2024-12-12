//========= Copyright Valve Corporation, All rights reserved. ============//
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
#include "mathlib/vector.h"
#include "playernet_vars.h"

//-----------------------------------------------------------------------------
// Purpose: Player specific data ( sent only to local player, too )
//-----------------------------------------------------------------------------
class CPlayerLocalData
{
public:
	DECLARE_PREDICTABLE();
	DECLARE_CLASS_NOBASE(CPlayerLocalData);
	DECLARE_EMBEDDED_NETWORKVAR();

	CPlayerLocalData() :
		m_iv_vecPunchAngle("CPlayerLocalData::m_iv_vecPunchAngle"),
		m_iv_vecPunchAngleVel("CPlayerLocalData::m_iv_vecPunchAngleVel"),
		m_iv_vecRecoilPunchAngle("CPlayerLocalData::m_iv_vecRecoilPunchAngle"),
		m_iv_vecRecoilPunchAngleVel("CPlayerLocalData::m_iv_vecRecoilPunchAngleVel")
	{
		m_iv_vecPunchAngle.Setup(&m_vecPunchAngle.m_Value, LATCH_SIMULATION_VAR);
		m_iv_vecPunchAngleVel.Setup(&m_vecPunchAngleVel.m_Value, LATCH_SIMULATION_VAR);

		m_iv_vecRecoilPunchAngle.Setup(&m_vecRecoilPunchAngle.m_Value, LATCH_SIMULATION_VAR);
		m_iv_vecRecoilPunchAngleVel.Setup(&m_vecRecoilPunchAngleVel.m_Value, LATCH_SIMULATION_VAR);

		m_iDefaultFOV = 0;
		m_iScopeFOV = 0;
		m_flFOVRate = m_flViewmodelFOVRate = 0.0f;
	}

	unsigned char			m_chAreaBits[MAX_AREA_STATE_BYTES];				// Area visibility flags.
	unsigned char			m_chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES];// Area portal visibility flags.

	int						m_iDefaultFOV;
	float					m_flFOVRate;
	float					m_flViewmodelFOVRate;
	int						m_iScopeFOV;

	float					m_flJumpTime;
	int						m_nStepside;
	float					m_flFallVelocity;
	int						m_nOldButtons;

	// Base velocity that was passed in to server physics so 
	//  client can predict conveyors correctly.  Server zeroes it, so we need to store here, too.
	Vector					m_vecClientBaseVelocity;

	CNetworkQAngle(m_vecPunchAngle);		// auto-decaying view angle adjustment
	CInterpolatedVar< QAngle >	m_iv_vecPunchAngle;

	CNetworkQAngle(m_vecPunchAngleVel);		// velocity of auto-decaying view angle adjustment
	CInterpolatedVar< QAngle >	m_iv_vecPunchAngleVel;

	CNetworkQAngle(m_vecRecoilPunchAngle);
	CInterpolatedVar<QAngle> m_iv_vecRecoilPunchAngle;

	CNetworkQAngle(m_vecRecoilPunchAngleVel);
	CInterpolatedVar< QAngle >	m_iv_vecRecoilPunchAngleVel;

	float					m_flStepSize;
	bool					m_bAllowAutoMovement;
	bool					m_bIsInOtherView;

	// 3d skybox
	sky3dparams_t			m_skybox3d;
	// fog params
	fogplayerparams_t		m_PlayerFog;
	// audio environment
	audioparams_t			m_audio;
};

#endif // C_PLAYERLOCALDATA_H