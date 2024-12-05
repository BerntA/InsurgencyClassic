//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "missile_launched_base.h"
#include "missiledef.h"
#include "ins_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
BEGIN_DATADESC( CBaseLaunchedMissile )

	DEFINE_THINKFUNC( VelocityCheck )

END_DATADESC()

//=========================================================
//=========================================================
CBaseLaunchedMissile::CBaseLaunchedMissile( )
{
}

//=========================================================
//=========================================================
void CBaseLaunchedMissile::CreateLaunchedMissile( CBasePlayer *pOwner, int iAmmoID, const Vector &vecPosition, const QAngle &angAngles, const Vector &vecDirection )
{
	CBaseLaunchedMissile *pLaunchedMissile = ( CBaseLaunchedMissile* )CBaseDetonator::CreateDetonator( pOwner, AMMOTYPE_MISSILE, iAmmoID, vecPosition, angAngles );

	if( !pLaunchedMissile )
		return;

	pLaunchedMissile->Configure( vecDirection );
	pLaunchedMissile->Setup( );
}

//=========================================================
//=========================================================
void CBaseLaunchedMissile::Setup( void )
{
	BaseClass::Setup( );

	// setup entity
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );

	// setup forces
	IPhysicsObject *pPhysicsObject = VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags( ) | FSOLID_TRIGGER, false );

	if( !pPhysicsObject )
		return;

	pPhysicsObject->EnableGravity( true );
	pPhysicsObject->EnableDrag( false );
	pPhysicsObject->SetVelocity( &m_vecInitialDirection, NULL );

	float flDamping, flAngDamping;
	flDamping = flAngDamping = 0.0f;

	pPhysicsObject->SetDamping( &flDamping, &flAngDamping );
	pPhysicsObject->SetInertia( Vector( 1e30, 1e30, 1e30 ) );

	Vector vecForce;
	VectorScale( m_vecInitialDirection, 8000.0f, vecForce );

	pPhysicsObject->ApplyForceCenter( vecForce );
}

//=========================================================
//=========================================================
void CBaseLaunchedMissile::VelocityCheck( void )
{
	if( UTIL_IsMoving( GetAbsVelocity( ) ) )
		return;

	SetThink( &CBaseLaunchedMissile::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 5.0f );
}

//=========================================================
//=========================================================
void CBaseLaunchedMissile::OnCollision( void )
{
	SetThink( &CBaseLaunchedMissile::VelocityCheck );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//=========================================================
//=========================================================
void CBaseLaunchedMissile::OnSafetyHit( void )
{
}
