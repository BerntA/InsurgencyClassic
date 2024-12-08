//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_grenade_smoke_base.h"

#ifdef GAME_DLL

#include "smoke_trail.h"
#include "particle_smokegrenade.h"
#include "soundemittersystem/isoundemittersystembase.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CSmokeGrenade::Spawn( void )
{
	BaseClass::Spawn( );


}

//=========================================================
//=========================================================
void CSmokeGrenade::UpdateOnRemove( void )
{
	RemoveSmokeTrail( );

	BaseClass::UpdateOnRemove( );
}

//=========================================================
//=========================================================
void CSmokeGrenade::CreateSmokeTrail( void )
{
	m_hSmokeTrail = SmokeTrail::CreateSmokeTrail( );

	SmokeTrail *pSmokeTrail = m_hSmokeTrail;

	if( !pSmokeTrail )
		return;

	pSmokeTrail->m_SpawnRate = 48;
	pSmokeTrail->m_ParticleLifetime = 1.5f;
	pSmokeTrail->m_StartColor.Init( 0.9f, 0.9f, 0.9f );
	pSmokeTrail->m_EndColor.Init( 0.9f,0.9f,0.9f );
	pSmokeTrail->m_StartSize = 12;
	pSmokeTrail->m_EndSize = 24;
	pSmokeTrail->m_SpawnRadius = 4;
	pSmokeTrail->m_MinSpeed = 4;
	pSmokeTrail->m_MaxSpeed = 12;
	pSmokeTrail->m_Opacity = 0.2f;

	pSmokeTrail->SetLifetime( 10.0f );
	pSmokeTrail->FollowEntity( this );
}

//=========================================================
//=========================================================
void CSmokeGrenade::RemoveSmokeTrail( void )
{
	SmokeTrail *pSmokeTrail = m_hSmokeTrail;

	if( pSmokeTrail )
		UTIL_Remove( pSmokeTrail );
}

//=========================================================
//=========================================================

// FIXME: put in ammodefs!!
#define SMOKEGRENADE_REMAIN 3.5f
#define SMOKEGRENADE_FADE_START 15
#define SMOKEGRENADE_FADE_END 30

void CSmokeGrenade::Detonate( void )
{
	CBasePlayer *pOwner = GetPlayerOwner( );

	if( !pOwner )
		return;

	Vector vecOrigin = GetAbsOrigin( );

	if( UTIL_PointContents( vecOrigin ) & MASK_WATER )
	{
		UTIL_Bubbles( vecOrigin - Vector( 64, 64, 64 ), vecOrigin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		ParticleSmokeGrenade *pSmoke = dynamic_cast< ParticleSmokeGrenade* >( CreateEntityByName( PARTICLESMOKEGRENADE_ENTITYNAME ) );

		if( pSmoke )
		{
			Vector vecForward;
			AngleVectors( GetLocalAngles( ), &vecForward );

			vecForward.z = 0;
			VectorNormalize( vecForward );

			pSmoke->SetLocalOrigin( vecOrigin + vecForward * 65 );
			pSmoke->SetFadeTime( SMOKEGRENADE_FADE_START, SMOKEGRENADE_FADE_END );
			pSmoke->Activate( );
			pSmoke->SetLifetime( SMOKEGRENADE_FADE_END );
			pSmoke->FillVolume( );
		}
	}

	const char *pszSound = GetDetonatorSound( );

	if( pszSound )
		EmitSound( pszSound );

	SetThink( &CBaseGrenade::SUB_Remove );
	SetNextThink( gpGlobals->curtime + SMOKEGRENADE_REMAIN );
}

#endif

//=========================================================
//=========================================================
bool CWeaponGrenadeSmokeBase::ValidAttack( int iAttackType ) const
{
	return ( iAttackType != ATTACKTYPE_THROW_COOK );
}