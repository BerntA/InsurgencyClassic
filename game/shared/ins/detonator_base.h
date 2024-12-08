//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASE_DETONATOR_H
#define BASE_DETONATOR_H
#ifdef _WIN32
#pragma once
#endif

#include "ammodef.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CBaseDetonator C_BaseDetonator

#endif

//=========================================================
//=========================================================
class CBaseDetonator : public CBaseAnimating
{
	DECLARE_CLASS( CBaseDetonator, CBaseAnimating );
	DECLARE_NETWORKCLASS( );

#ifdef GAME_DLL

	DECLARE_DATADESC( );

#endif

public:
	CBaseDetonator( );

#ifdef GAME_DLL

	static CBaseDetonator *CreateDetonator( CBasePlayer *pOwner, int iAmmoType, int iAmmoID, const Vector &vecPosition, const QAngle &angAngles );

#endif

	void Spawn( void );

#ifdef GAME_DLL

	virtual void Setup( void );

#endif

protected:

#ifdef GAME_DLL

	CBasePlayer *GetPlayerOwner( void ) const;

	void SetAmmoID( int iAmmoID );

	virtual const char *GetDetonatorModel( void ) const;
	virtual void GetDetonatorBounds( Vector &vecMins, Vector &vecMaxs ) const;
	virtual const char *GetDetonatorSound( void ) const;

	virtual void Detonate( void ) { }

	void Destroy( void );

	void Explode( trace_t *pCheckTrace = NULL );
	virtual void GetExplosionDamage( int &iDamage, int &iDamageRadius );
	virtual int GetExplosionDamageFlags( void ) const;

#endif

private:

#ifdef GAME_DLL

	void Event_Killed( const CTakeDamageInfo &info );

	CBasePlayer *GetScorer( void ) const;
	int GetInflictorID( void ) const;

#endif

private:
	CBaseDetonator( const CBaseDetonator & );

protected:

#ifdef GAME_DLL

	int m_iAmmoID;

#endif
};

#endif // BASE_DETONATOR_H
