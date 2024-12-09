//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef BASE_MISSILE_LAUNCHED_H
#define BASE_MISSILE_LAUNCHED_H
#ifdef _WIN32
#pragma once
#endif

#include "missile_base.h"

//=========================================================
//=========================================================
#define MISSILE_LAUNCHED_LENGTH 8

//=========================================================
//=========================================================
class CBaseLaunchedMissile : public CBaseMissile
{
public:
	DECLARE_CLASS( CBaseLaunchedMissile, CBaseMissile );
	DECLARE_DATADESC( );

public:
	CBaseLaunchedMissile( );

	static void CreateLaunchedMissile( CBasePlayer *pOwner, int iAmmoID, const Vector &vecPosition, const QAngle &angAngles, const Vector &vecDirection );

private:
	void Setup( void );

	void VelocityCheck( void );

	void OnCollision( void );
	void OnSafetyHit( void );
};

#endif // BASE_MISSILE_LAUNCHED_H
