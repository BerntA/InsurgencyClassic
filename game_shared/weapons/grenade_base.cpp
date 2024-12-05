//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "grenade_base.h"
#include "grenadedef.h"
#include "ins_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CBaseGrenade::CBaseGrenade( )
{
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

//=========================================================
//=========================================================
void CBaseGrenade::Setup( void )
{
	BaseClass::Setup( );

	float flDetonateThreshold = GetDetonateThreshold( );
	flDetonateThreshold = max( gpGlobals->curtime, flDetonateThreshold );

	SetThink( &ThisClass::Detonate );
	SetNextThink( flDetonateThreshold );
}

//=========================================================
//=========================================================
float CBaseGrenade::GetDetonateThreshold( void )
{
	const CGrenadeData &GrenadeData = CGrenadeDef::GetGrenadeData( m_iAmmoID );
	return gpGlobals->curtime + GrenadeData.m_flFuse;
}

//=========================================================
//=========================================================
const char *CBaseGrenade::GetDetonatorModel( void ) const
{
	const CGrenadeData &GrenadeData = CGrenadeDef::GetGrenadeData( m_iAmmoID );
	return GrenadeData.m_szModel;
}

//=========================================================
//=========================================================
const char *CBaseGrenade::GetDetonatorSound( void ) const
{
	const CGrenadeData &GrenadeData = CGrenadeDef::GetGrenadeData( m_iAmmoID );
	return GrenadeData.m_szDetonationSound;
}

//=========================================================
//=========================================================
void CBaseGrenade::GetExplosionDamage( int &iDamage, int &iDamageRadius )
{
	const CGrenadeData &GrenadeData = CGrenadeDef::GetGrenadeData( m_iAmmoID );
	iDamage = GrenadeData.m_iDamage;
	iDamageRadius = GrenadeData.m_iDamageRadius;
}

//=========================================================
//=========================================================
int CBaseGrenade::GetInflictorType( void ) const
{
	return INFLICTORTYPE_GRENADE;
}

#endif