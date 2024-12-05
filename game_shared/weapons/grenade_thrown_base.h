//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASE_GRENADE_THROWN_SHARED_H
#define BASE_GRENADE_THROWN_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "grenade_base.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CGrenadeThrownBase C_GrenadeThrownBase

#endif

//=========================================================
//=========================================================
class CGrenadeThrownBase : public CBaseGrenade
{
	DECLARE_CLASS( CGrenadeThrownBase, CBaseGrenade );
	DECLARE_PREDICTABLE( );
	DECLARE_NETWORKCLASS( );

public:
	CGrenadeThrownBase( );

#ifdef GAME_DLL

	static void CreateThrownGrenade( CBasePlayer *pOwner, int iAmmoID, const Vector &vecPosition, const Vector &vecVelocity, const AngularImpulse &angVelocity, float flCookThreshold );

#endif

#ifdef CLIENT_DLL

	void PostDataUpdate( DataUpdateType_t type );

#endif

private:
	CGrenadeThrownBase( const CBaseGrenade & );

#ifdef GAME_DLL

	void Configure( const Vector &vecVelocity, const AngularImpulse &angVelocity, float flCookThreshold );

	void Setup( void );

	void GetDetonatorBounds( Vector &vecMins, Vector &vecMaxs ) const;
	float GetDetonateThreshold( void );

	void ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );

#endif

private:
	CNetworkVector( m_vecInitialVelocity );

	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_vecVelocity );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_fFlags );

#ifdef GAME_DLL

	float m_flCookThreshold;

#endif
};

#endif // BASE_GRENADE_THROWN_SHARED_H
