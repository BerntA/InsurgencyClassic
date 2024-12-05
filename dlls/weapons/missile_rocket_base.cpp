//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "missile_rocket_base.h"
#include "missiledef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define BODYGROUP_PROJECTILE "projectile"
#define ATTACHMENT_TRAIL "trail"

#define INITIAL_ROCKET_SPEED 3000.0f

//=========================================================
//=========================================================
CBaseRocketMissile::CBaseRocketMissile( )
{
	m_pController = NULL;

	m_hRocketTrail = NULL;
}

//=========================================================
//=========================================================
void CBaseRocketMissile::CreateRocketMissile( CBasePlayer *pOwner, int iAmmoID, const Vector &vecPosition, const QAngle &angAngles, const Vector &vecDirection )
{
	CBaseRocketMissile *pRocketMissile = ( CBaseRocketMissile* )CBaseDetonator::CreateDetonator( pOwner, AMMOTYPE_MISSILE, iAmmoID, vecPosition, angAngles );

	if( !pRocketMissile )
		return;

	pRocketMissile->Configure( vecDirection );
	pRocketMissile->Setup( );
}

//=========================================================
//=========================================================
void CBaseRocketMissile::Setup( void )
{
	BaseClass::Setup( );

	// setup entity
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	AddEffects( EF_NOSHADOW );

	// setup projectile
	int iProjectileGroupID = FindBodygroupByName( BODYGROUP_PROJECTILE );

	if( iProjectileGroupID != -1 )
		SetBodygroup( iProjectileGroupID, 1 );

	// start the smoke
	CreateSmokeTrail( );

	// setup the forces
	IPhysicsObject *pPhysicsObject = VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags( ) | FSOLID_TRIGGER, false );

	if( !pPhysicsObject )
		return;

	Vector vecVelocity = m_vecInitialDirection * INITIAL_ROCKET_SPEED;

	Vector thrustOrigin, thrustDirection;
	GetAttachment( LookupAttachment( ATTACHMENT_TRAIL ), thrustOrigin );

	VectorNormalize( thrustDirection );

	m_Thruster.m_thrust = 1000.0f;
	m_Thruster.CalcThrust( thrustOrigin, m_vecInitialDirection, VPhysicsGetObject( ) );

	m_pController = physenv->CreateMotionController( &m_Thruster );
	m_pController->AttachObject( pPhysicsObject, true );

	pPhysicsObject->EnableGravity( false );
	pPhysicsObject->EnableDrag( false );

	float flDamping = 0.0f;
	float flAngDamping = 0.0f;
	pPhysicsObject->SetDamping( &flDamping, &flAngDamping );
	pPhysicsObject->SetInertia( Vector( 1e30, 1e30, 1e30 ) );

	PhysSetGameFlags( pPhysicsObject, FVPHYSICS_HEAVY_OBJECT );

	pPhysicsObject->SetVelocity( &vecVelocity, NULL );
}

//=========================================================
//=========================================================
void CBaseRocketMissile::OnCollision( void )
{
}

//=========================================================
//=========================================================
void CBaseRocketMissile::OnSafetyHit( void )
{
	SetThink( &CBaseRocketMissile::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 0.1f );

	// TODO: impact effect
}

//=========================================================
//=========================================================
void CBaseRocketMissile::UpdateOnRemove( void )
{
	RemoveSmokeTrail( );

	BaseClass::UpdateOnRemove( );
}

//=========================================================
//=========================================================
void CBaseRocketMissile::CreateSmokeTrail( void )
{
	if( m_hRocketTrail )
		return;

	m_hRocketTrail = RocketTrail::CreateRocketTrail( );

	RocketTrail *pRocketTrail = m_hRocketTrail;

	if( !pRocketTrail )
		return;

	pRocketTrail->m_Opacity = 0.1f;
	pRocketTrail->m_SpawnRate = 75;
	pRocketTrail->m_ParticleLifetime = 0.15f;
	pRocketTrail->m_StartColor.Init( 0.65f, 0.65f , 0.65f );
	pRocketTrail->m_EndColor.Init( 0.0, 0.0, 0.0 );
	pRocketTrail->m_StartSize = 16;
	pRocketTrail->m_EndSize = 8;
	pRocketTrail->m_SpawnRadius = 3;
	pRocketTrail->m_MinSpeed = 2;
	pRocketTrail->m_MaxSpeed = 16;
	pRocketTrail->SetLifetime( -1 );
	pRocketTrail->FollowEntity( this, ATTACHMENT_TRAIL );
}

//=========================================================
//=========================================================
void CBaseRocketMissile::RemoveSmokeTrail( void )
{
	RocketTrail *pRocketTrail = m_hRocketTrail;

	if( pRocketTrail )
		UTIL_Remove( pRocketTrail );
}
