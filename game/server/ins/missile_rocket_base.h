//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef BASE_MISSILE_ROCKET_H
#define BASE_MISSILE_ROCKET_H
#ifdef _WIN32
#pragma once
#endif

#include "missile_base.h"
#include "physics_cannister.h"
#include "smoke_trail.h"

//=========================================================
//=========================================================
#define MISSILE_ROCKET_LENGTH 25

//=========================================================
//=========================================================
class CBaseRocketMissile : public CBaseMissile
{
public:
	DECLARE_CLASS( CBaseRocketMissile, CBaseMissile );

public:
	CBaseRocketMissile( );

	static void CreateRocketMissile( CBasePlayer *pOwner, int iAmmoID, const Vector &vecPosition, const QAngle &angAngles, const Vector &vecDirection );

private:
	void Setup( void );

	void OnCollision( void );
	void OnSafetyHit( void );

	void UpdateOnRemove( void );

	void CreateSmokeTrail( void );
	void RemoveSmokeTrail( void );

private:
	CThrustController m_Thruster;
	IPhysicsMotionController *m_pController;

	CHandle< RocketTrail > m_hRocketTrail;
};

#endif // BASE_MISSILE_ROCKET_H
