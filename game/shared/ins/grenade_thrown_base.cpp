//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "grenade_thrown_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define BODYGROUP_PIN "pin"

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( GrenadeThrownBase, DT_GrenadeThrownBase )
BEGIN_NETWORK_TABLE( CGrenadeThrownBase, DT_GrenadeThrownBase )

#ifdef GAME_DLL

	SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_LocalActiveWeaponData", "m_flTimeWeaponIdle" ),

	SendPropVector( SENDINFO( m_vecInitialVelocity ), 20, 0, -3000, 3000 ),
	SendPropVector( SENDINFO( m_vecVelocity ), 0, SPROP_NOSCALE ),

#else

	RecvPropVector( RECVINFO( m_vecInitialVelocity ) ),
	RecvPropVector( RECVINFO( m_vecVelocity ), 0, RecvProxy_LocalVelocity ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CGrenadeThrownBase )

	DEFINE_PRED_FIELD_TOL( m_vecVelocity, FIELD_VECTOR, FTYPEDESC_INSENDTABLE, 0.5f ),

END_PREDICTION_DATA( )

#endif

//=========================================================
//=========================================================
CGrenadeThrownBase::CGrenadeThrownBase( void )
{
	SetSimulatedEveryTick( true );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CGrenadeThrownBase::CreateThrownGrenade( CBasePlayer *pOwner, int iAmmoID, const Vector &vecPosition, const Vector &vecVelocity, const AngularImpulse &angVelocity, float flCookThreshold )
{
	CGrenadeThrownBase *pGrenadeThrown = ( CGrenadeThrownBase* )CBaseDetonator::CreateDetonator( pOwner, AMMOTYPE_GRENADE, iAmmoID, vecPosition, vec3_angle );

	if( !pGrenadeThrown )
		return;

	pGrenadeThrown->Configure( vecVelocity, angVelocity, flCookThreshold );
	pGrenadeThrown->Setup( );
}

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

//=========================================================
//=========================================================
void CGrenadeThrownBase::Setup( void )
{
	BaseClass::Setup( );

	// remove the pin
	int iPinGroup = FindBodygroupByName( BODYGROUP_PIN );

	if( iPinGroup != -1 ) 
		SetBodygroup( iPinGroup, 1 );
}

//=========================================================
//=========================================================
#define PROJECTILE_GRAVITY 0.85f
#define PROJECTILE_FRICTION 0.05f
#define PROJECTILE_ELASTICITY 0.4f

void CGrenadeThrownBase::Configure( const Vector &vecVelocity, const AngularImpulse &angVelocity, float flCookThreshold )
{
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );

	m_vecInitialVelocity = vecVelocity;
	SetAbsVelocity( vecVelocity );

	SetGravity( PROJECTILE_GRAVITY );
	SetFriction( PROJECTILE_ELASTICITY );
	SetElasticity( PROJECTILE_ELASTICITY );

	ApplyLocalAngularVelocityImpulse( angVelocity );	

	m_flCookThreshold = flCookThreshold;
}

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

void CGrenadeThrownBase::PostDataUpdate( DataUpdateType_t type )
{
	BaseClass::PostDataUpdate( type );

	if( type == DATA_UPDATE_CREATED )
	{
		// now stick our initial velocity into the interpolation history 
		CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator( );

		interpolator.ClearHistory( );
		float changeTime = GetLastChangeTime( LATCH_SIMULATION_VAR );

		// add a sample 1 second back.
		Vector vCurOrigin = GetLocalOrigin( ) - m_vecInitialVelocity;
		interpolator.AddToHead( changeTime - 1.0, &vCurOrigin, false );

		// add the current sample.
		vCurOrigin = GetLocalOrigin( );
		interpolator.AddToHead( changeTime, &vCurOrigin, false );
	}
}

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

//=========================================================
//=========================================================
void CGrenadeThrownBase::GetDetonatorBounds( Vector &vecMins, Vector &vecMaxs ) const
{
	vecMins = Vector( -2, -2, -2 );
	vecMaxs = Vector(  2,  2,  2 );
}

//=========================================================
//=========================================================
float CGrenadeThrownBase::GetDetonateThreshold( void )
{
	if( m_flCookThreshold == 0.0f )
		return BaseClass::GetDetonateThreshold( );

	return m_flCookThreshold;
}

//=========================================================
//=========================================================
void CGrenadeThrownBase::ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity )
{
	//Assume all surfaces have the same elasticity
	float flSurfaceElasticity = 1.0;

	//Don't bounce off of players with perfect elasticity
	if( trace.m_pEnt && trace.m_pEnt->IsPlayer() )
	{
		flSurfaceElasticity = 0.3;
	}

	// if its breakable glass and we kill it, don't bounce.
	// give some damage to the glass, and if it breaks, pass 
	// through it.
	bool breakthrough = false;

	if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable" ) )
	{
		breakthrough = true;
	}

	if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable_surf" ) )
	{
		breakthrough = true;
	}

	if (breakthrough)
	{
		CTakeDamageInfo info( this, this, 10, DMG_CLUB );
		trace.m_pEnt->DispatchTraceAttack( info, GetAbsVelocity(), &trace );

		ApplyMultiDamage();

		if( trace.m_pEnt->m_iHealth <= 0 )
		{
			// slow our flight a little bit
			Vector vel = GetAbsVelocity();

			vel *= 0.4;

			SetAbsVelocity( vel );
			return;
		}
	}

	float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
	flTotalElasticity = clamp( flTotalElasticity, 0.0f, 0.9f );

	// NOTE: A backoff of 2.0f is a reflection
	Vector vecAbsVelocity;
	PhysicsClipVelocity( GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f );
	vecAbsVelocity *= flTotalElasticity;

	// Get the total velocity (player + conveyors, etc.)
	VectorAdd( vecAbsVelocity, GetBaseVelocity(), vecVelocity );
	float flSpeedSqr = DotProduct( vecVelocity, vecVelocity );

	// Stop if on ground.
	if ( trace.plane.normal.z > 0.7f )			// Floor
	{
		// Verify that we have an entity.
		CBaseEntity *pEntity = trace.m_pEnt;
		Assert( pEntity );

		SetAbsVelocity( vecAbsVelocity );

		if ( flSpeedSqr < ( 30 * 30 ) )
		{
			if ( pEntity->IsStandable() )
			{
				SetGroundEntity( pEntity );
			}

			// Reset velocities.
			SetAbsVelocity( vec3_origin );
			SetLocalAngularVelocity( vec3_angle );

			//align to the ground so we're not standing on end
			QAngle angle;
			VectorAngles( trace.plane.normal, angle );

			// rotate randomly in yaw
			angle[1] = random->RandomFloat( 0, 360 );

			// TODO: rotate around trace.plane.normal

			SetAbsAngles( angle );			
		}
		else
		{
			Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;	
			Vector vecBaseDir = GetBaseVelocity();
			VectorNormalize( vecBaseDir );
			float flScale = vecDelta.Dot( vecBaseDir );

			VectorScale( vecAbsVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, vecVelocity ); 
			VectorMA( vecVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, GetBaseVelocity() * flScale, vecVelocity );
			PhysicsPushEntity( vecVelocity, &trace );
		}
	}
	else
	{
		// If we get *too* slow, we'll stick without ever coming to rest because
		// we'll get pushed down by gravity faster than we can escape from the wall.
		if ( flSpeedSqr < ( 30 * 30 ) )
		{
			// Reset velocities.
			SetAbsVelocity( vec3_origin );
			SetLocalAngularVelocity( vec3_angle );
		}
		else
		{
			SetAbsVelocity( vecAbsVelocity );
		}
	}
}

#endif
