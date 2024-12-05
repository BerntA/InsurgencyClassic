//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef BASE_GRENADE_H
#define BASE_GRENADE_H
#ifdef _WIN32
#pragma once
#endif

#include "detonator_base.h"

//=========================================================
//=========================================================
class CBaseGrenade : public CBaseDetonator
{
	DECLARE_CLASS( CBaseGrenade, CBaseDetonator );

public:
	CBaseGrenade( );

#ifdef GAME_DLL

	void Setup( void );

protected:
	virtual float GetDetonateThreshold( void );
	const char *GetDetonatorSound( void ) const;

	void GetExplosionDamage( int &iDamage, int &iDamageRadius );

private:
	const char *GetDetonatorModel( void ) const;

	int GetInflictorType( void ) const;

#endif
};

#endif // BASE_GRENADE_H