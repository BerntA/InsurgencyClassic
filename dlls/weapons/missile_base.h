//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef BASE_MISSILE_H
#define BASE_MISSILE_H
#ifdef _WIN32
#pragma once
#endif

#include "detonator_base.h"

//=========================================================
//=========================================================
class CBaseMissile : public CBaseDetonator
{
	DECLARE_CLASS( CBaseMissile, CBaseDetonator );

public:
	CBaseMissile( );

protected:
	void Configure( const Vector &vecDirection );

	virtual void Setup( void );

	virtual int GetSafetyRange( void ) const;

	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual void OnCollision( void ) = 0;
	virtual void OnSafetyHit( void ) = 0;

private:
	void Detonate( void );
	const char *GetDetonatorModel( void ) const;
	const char *GetDetonatorSound( void ) const;

	void GetExplosionDamage( int &iDamage, int &iDamageRadius );

	int GetInflictorType( void ) const;

protected:
	Vector m_vecOriginalOrigin;
	Vector m_vecInitialDirection;

	bool m_bDetonated;
};

#endif // BASE_MISSILE_H