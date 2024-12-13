//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: combine ball
//
//=============================================================================//

#include "cbase.h"
#include "ins_combineball.h"
#include "props.h"
#include "explode.h"
#include "materialsystem/imaterial.h"
#include "beam_flags.h"
#include "physics_prop_ragdoll.h"
#include "soundent.h"
#include "soundenvelope.h"
#include "te_effect_dispatch.h"
#include "spritetrail.h"
#include "decals.h"
#include "eventqueue.h"
#include "ins_player.h"
#include "ins_pbrules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define INS_COMBINEBALL_SPRITE_TRAIL "sprites/combineball_trail_black_1.vmt" 
#define INS_COMBINEBALL_LIFETIME 4.0f
#define INS_COMBINEBALL_HOLD_DISSOLVE_TIME 8.0f
#define	MAX_COMBINEBALL_RADIUS 12

#define INS_COMBINEBALL_GUIDEFACTOR 0.5
#define INS_COMBINEBALL_SEARCHRADIUS 512
#define INS_COMBINEBALL_SEEKANGLE 15

#define INS_COMBINEBALL_RADIUS 10
#define INS_COMBINEBALL_DURATION 2
#define INS_COMBINEBALL_MASS 150

//=========================================================
//=========================================================
static const char *g_pszExplodeTimerContext = "ExplodeTimerContext";
static const char *g_pszAnimThinkContext = "AnimThinkContext";
static const char *g_pszRemoveContext = "RemoveContext";

int g_iExplosionTexture = -1;

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_combineball, CINSCombineBall );

//=========================================================
//=========================================================
BEGIN_DATADESC( CINSCombineBall )

	DEFINE_THINKFUNC( ExplodeThink ),
	DEFINE_THINKFUNC( DieThink ),
	DEFINE_THINKFUNC( AnimThink ),

END_DATADESC( )

//=========================================================
//=========================================================
IMPLEMENT_SERVERCLASS_ST( CINSCombineBall, DT_INSCombineBall )

	SendPropBool( SENDINFO( m_bEmit ) ),
	SendPropFloat( SENDINFO( m_flRadius ), 0, SPROP_NOSCALE ),

END_SEND_TABLE()

//=========================================================
//=========================================================
void CINSCombineBall::CreateBall( const Vector &vecOrigin, const Vector &vecVelocity, CBaseEntity *pOwner )
{
	CINSCombineBall *pBall = static_cast< CINSCombineBall* >( CreateEntityByName( "ins_combineball" ) );
	Assert( pBall );

	pBall->SetRadius( INS_COMBINEBALL_RADIUS );

	pBall->SetAbsOrigin( vecOrigin );
	pBall->SetOwnerEntity( pOwner );

	pBall->SetAbsVelocity( vecVelocity );
	pBall->Spawn( );

	pBall->SetSpeed( vecVelocity.Length( ) );

	pBall->EmitSound( "NPC_CombineBall.Launch" );

	PhysSetGameFlags( pBall->VPhysicsGetObject( ), FVPHYSICS_WAS_THROWN );

	pBall->SetMass( INS_COMBINEBALL_MASS );
	pBall->StartLifetime( INS_COMBINEBALL_DURATION );
}

//=========================================================
//=========================================================
void CINSCombineBall::Precache( void )
{
	PrecacheModel( INS_COMBINEBALL_SPRITE_TRAIL );

	g_iExplosionTexture = PrecacheModel( "sprites/lgtning.vmt" );

	PrecacheScriptSound( "INS_CombineBall.Launch" );
	PrecacheScriptSound( "INS_CombineBall.KillImpact" );
	PrecacheScriptSound( "INS_CombineBall.Explosion" );
	PrecacheScriptSound( "INS_CombineBall.Impact" );
}

//=========================================================
//=========================================================
bool CINSCombineBall::OverridePropdata( void ) 
{ 
	return true; 
}

//=========================================================
//=========================================================
void CINSCombineBall::SetRadius( float flRadius )
{
	m_flRadius = clamp( flRadius, 1, MAX_COMBINEBALL_RADIUS );
}

//=========================================================
//=========================================================
bool CINSCombineBall::CreateVPhysics( void )
{
	SetSolid( SOLID_BBOX );

	float flSize = m_flRadius;

	SetCollisionBounds( Vector( -flSize, -flSize, -flSize ), Vector( flSize, flSize, flSize ) );
	objectparams_t params = g_PhysDefaultObjectParams;
	params.pGameData = static_cast< void* >( this );
	int nMaterialIndex = physprops->GetSurfaceIndex( "metal_bouncy" );
	IPhysicsObject *pPhysicsObject = physenv->CreateSphereObject( flSize, nMaterialIndex, GetAbsOrigin( ), GetAbsAngles( ), &params, false );

	if ( !pPhysicsObject )
		return false;

	VPhysicsSetObject( pPhysicsObject );
	SetMoveType( MOVETYPE_VPHYSICS );
	pPhysicsObject->Wake( );

	pPhysicsObject->SetMass( 750.0f );
	pPhysicsObject->EnableGravity( false );
	pPhysicsObject->EnableDrag( false );

	float flDamping = 0.0f;
	float flAngDamping = 0.5f;
	pPhysicsObject->SetDamping( &flDamping, &flAngDamping );
	pPhysicsObject->SetInertia( Vector( 1e30, 1e30, 1e30 ) );

	PhysSetGameFlags( pPhysicsObject, FVPHYSICS_DMG_DISSOLVE | FVPHYSICS_HEAVY_OBJECT );

	return true;
}

//=========================================================
//=========================================================
void CINSCombineBall::Spawn( void )
{
	BaseClass::Spawn( );

	SetModel( POWERBALL_MODEL );

	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );

	CreateVPhysics( );

	Vector vecAbsVelocity = GetAbsVelocity( );
	VPhysicsGetObject( )->SetVelocity( &vecAbsVelocity, NULL );

	// no shadow
	AddEffects( EF_NOSHADOW );

	// start up the eye trail
	m_pGlowTrail = CSpriteTrail::SpriteTrailCreate( INS_COMBINEBALL_SPRITE_TRAIL, GetAbsOrigin( ), false );
	
	if ( m_pGlowTrail != NULL )
	{
		m_pGlowTrail->FollowEntity( this );
		m_pGlowTrail->SetTransparency( kRenderTransAdd, 0, 0, 0, 255, kRenderFxNone );
		m_pGlowTrail->SetStartWidth( m_flRadius );
		m_pGlowTrail->SetEndWidth( 0 );
		m_pGlowTrail->SetLifeTime( 0.1f );
		m_pGlowTrail->TurnOff();
	}

	m_bEmit = true;

	m_flNextDamageTime = gpGlobals->curtime;
}

//=========================================================
//=========================================================
void CINSCombineBall::StartAnimating( void )
{
	// start our animation cycle - use the random to avoid everything thinking the same frame
	SetContextThink( &CINSCombineBall::AnimThink, gpGlobals->curtime + random->RandomFloat( 0.0f, 0.1f ), g_pszAnimThinkContext );

	int nSequence = LookupSequence( "idle" );

	SetCycle( 0 );
	m_flAnimTime = gpGlobals->curtime;
	ResetSequence( nSequence );
	ResetClientsideFrame( );
}

//=========================================================
//=========================================================
void CINSCombineBall::StopAnimating( void )
{
	SetContextThink( NULL, gpGlobals->curtime, g_pszAnimThinkContext );
}

//=========================================================
//=========================================================
void CINSCombineBall::StartLifetime( float flDuration )
{
	SetContextThink( &CINSCombineBall::ExplodeThink, gpGlobals->curtime + flDuration, g_pszExplodeTimerContext );
}

//=========================================================
//=========================================================
void CINSCombineBall::SetMass( float flMass )
{
	IPhysicsObject *pObj = VPhysicsGetObject( );

	if ( pObj != NULL )
	{
		pObj->SetMass( flMass );
		pObj->SetInertia( Vector( 500, 500, 500 ) );
	}
}

//=========================================================
//=========================================================
CBasePlayer *CINSCombineBall::HasPhysicsAttacker( float flDT )
{
	// must have an owner
	if( GetOwnerEntity( ) == NULL || GetOwnerEntity( )->IsPlayer( ) )
		return false;

	// we don't care about the time passed in
	return static_cast< CBasePlayer* >( GetOwnerEntity( ) );
}

//=========================================================
//=========================================================
void CINSCombineBall::UpdateOnRemove( void )
{
	if ( m_pGlowTrail != NULL )
	{
		UTIL_Remove( m_pGlowTrail );
		m_pGlowTrail = NULL;
	}

	BaseClass::UpdateOnRemove( );
}

//=========================================================
//=========================================================
void CINSCombineBall::ExplodeThink( void )
{
	DoExplosion( );
}

//=========================================================
//=========================================================
void CINSCombineBall::DieThink( void )
{
	UTIL_Remove( this );
}

//=========================================================
//=========================================================
void CINSCombineBall::DoExplosion( void )
{
	CBroadcastRecipientFilter filter2;

	te->BeamRingPoint( filter2, 0, GetAbsOrigin( ),
		128,
		384,
		g_iExplosionTexture,
		0,
		0,
		2,
		0.25f,
		48,
		0,
		0,
		255,
		255,
		225,
		64,
		0,
		FBEAM_FADEOUT );

	// turn us off and wait because we need our trails to finish up properly
	SetAbsVelocity( vec3_origin );
	SetMoveType( MOVETYPE_NONE );
	AddSolidFlags( FSOLID_NOT_SOLID );

	m_bEmit = false;

	SetContextThink( &CINSCombineBall::SUB_Remove, gpGlobals->curtime + 0.5f, g_pszRemoveContext );
}

//=========================================================
//=========================================================
void CINSCombineBall::CollisionEventToTrace( int iIndex, gamevcollisionevent_t *pEvent, trace_t &tr )
{
	UTIL_ClearTrace( tr );
	pEvent->pInternalData->GetSurfaceNormal( tr.plane.normal );
	pEvent->pInternalData->GetContactPoint( tr.endpos );
	tr.plane.dist = DotProduct( tr.plane.normal, tr.endpos );
	VectorMA( tr.endpos, -1.0f, pEvent->preVelocity[ iIndex ], tr.startpos );
	tr.m_pEnt = pEvent->pEntities[ !iIndex ];
	tr.fraction = 0.01f;
}

//=========================================================
//=========================================================
bool CINSCombineBall::OnHitEntity( CBaseEntity *pHitEntity, float flSpeed, int iIndex, gamevcollisionevent_t *pEvent )
{
	if( pHitEntity && pHitEntity->IsPlayer( ) )
	{
		CINSPlayer *pPlayer = ToINSPlayer( pHitEntity );

		if( pPlayer->IsRunningAround( ) )
		{
			pPlayer->GivePowerball( );
			return true;
		}
	}

	Vector vecFinalVelocity = pEvent->postVelocity[ iIndex ];
	VectorNormalize( vecFinalVelocity );
	vecFinalVelocity *= GetSpeed( );

	PhysCallbackSetVelocity( pEvent->pObjects[ iIndex ], vecFinalVelocity ); 

	return false;
}

//=========================================================
//=========================================================
void CINSCombineBall::DoImpactEffect( const Vector &preVelocity, int iIndex, gamevcollisionevent_t *pEvent )
{
	// do that crazy impact effect!
	trace_t tr;
	CollisionEventToTrace( !iIndex, pEvent, tr );
	
	CBaseEntity *pTraceEntity = pEvent->pEntities[ iIndex ];
	UTIL_TraceLine( tr.startpos - preVelocity * 2.0f, tr.startpos + preVelocity * 2.0f, MASK_SOLID, pTraceEntity, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction < 1.0f )
	{
		// see if we hit the sky
		if ( tr.surface.flags & SURF_SKY )
		{
			DoExplosion( );
			return;
		}

		// send the effect over
		CEffectData	data;

		data.m_flRadius = 16;
		data.m_vNormal = tr.plane.normal;
		data.m_vOrigin = tr.endpos + tr.plane.normal * 1.0f;

		DispatchEffect( "cball_bounce", data );
	}

	EmitSound( "NPC_CombineBall.Impact" );
}

//=========================================================
//=========================================================
bool CINSCombineBall::IsAttractiveTarget( CBaseEntity *pEntity )
{
	if ( !pEntity->IsPlayer( ) || !GetOwnerEntity( ) || !GetOwnerEntity( )->IsPlayer( ) ) 
		return false;

	CINSPlayer *pShooter, *pTarget;
	pShooter = ToINSPlayer( GetOwnerEntity( ) );
	pTarget = ToINSPlayer( pEntity );

	// can only hit alive players on the same team
	if( !pTarget->IsRunningAround( ) )
		return false;

	if( pShooter->GetTeamID( ) != pTarget->GetTeamID( ) )
		return false;
	
	// we must be able to hit them
	trace_t	tr;
	UTIL_TraceLine( WorldSpaceCenter( ), pEntity->BodyTarget( WorldSpaceCenter( ) ), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction < 1.0f && tr.m_pEnt != pEntity )
		return false;

	return true;
}

//=========================================================
//=========================================================
void CINSCombineBall::DeflectTowardEnemy( float flSpeed, int iIndex, gamevcollisionevent_t *pEvent )
{
	Vector vecVelDir = pEvent->postVelocity[ iIndex ];
	VectorNormalize( vecVelDir );

	CBaseEntity *pBestTarget = NULL;

	Vector vecStartPoint;
	pEvent->pInternalData->GetContactPoint( vecStartPoint );

	float flBestDist = MAX_COORD_FLOAT;

	CBaseEntity *pList[ 1024 ];

	Vector vecDelta;
	float flDistance;

	int iCount = UTIL_EntitiesInSphere( pList, 1024, GetAbsOrigin( ), INS_COMBINEBALL_SEARCHRADIUS, FL_NPC | FL_CLIENT );
		
	for( int i = 0; i < iCount; i++ )
	{
		if ( !IsAttractiveTarget( pList[ i ] ) )
			continue;

		VectorSubtract( pList[ i ]->WorldSpaceCenter( ), vecStartPoint, vecDelta );
		flDistance = VectorNormalize( vecDelta );

		if ( flDistance < flBestDist )
		{
			// check our direction
			if ( DotProduct( vecDelta, vecVelDir ) > 0.0f )
			{
				pBestTarget = pList[ i ];
				flBestDist = flDistance;
			}
		}
	}

	if ( pBestTarget )
	{
		Vector vecDelta;
		VectorSubtract( pBestTarget->WorldSpaceCenter( ), vecStartPoint, vecDelta );
		VectorNormalize( vecDelta );
		vecDelta *= GetSpeed( );
		PhysCallbackSetVelocity( pEvent->pObjects[ iIndex], vecDelta ); 
	}
}

//=========================================================
//=========================================================
void CINSCombineBall::VPhysicsCollision( int iIndex, gamevcollisionevent_t *pEvent )
{
	Vector preVelocity = pEvent->preVelocity[ iIndex ];
	float flSpeed = VectorNormalize( preVelocity );

	CBaseEntity *pHitEntity = pEvent->pEntities[ !iIndex ];
	
	if( OnHitEntity( pHitEntity, flSpeed, iIndex, pEvent ) )
	{
		// remove self without affecting the object that was hit. (Unless it was flesh)
		PhysCallbackRemove( this->NetworkProp( ) );

		// disable dissolve damage so we don't kill off the player when he's the one we hit
		PhysClearGameFlags( VPhysicsGetObject(), FVPHYSICS_DMG_DISSOLVE );
		return;
	}

	// if we've collided going faster than our desired, then up our desired
	if( flSpeed > GetSpeed( ) )
		SetSpeed( flSpeed );

	// make sure we don't slow down
	Vector vecFinalVelocity = pEvent->postVelocity[ iIndex ];
	VectorNormalize( vecFinalVelocity );
	vecFinalVelocity *= GetSpeed( );
	PhysCallbackSetVelocity( pEvent->pObjects[ iIndex ], vecFinalVelocity ); 

	// do that crazy impact effect
	DoImpactEffect( preVelocity, iIndex, pEvent );

	// only do the bounce so often
	if( gpGlobals->curtime - m_flLastBounceTime < 0.25f )
		return;

	// save off our last bounce time
	m_flLastBounceTime = gpGlobals->curtime;

	// deflect towards nearby enemies
	DeflectTowardEnemy( flSpeed, iIndex, pEvent );
}

//=========================================================
//=========================================================
void CINSCombineBall::AnimThink( void )
{
	StudioFrameAdvance( );
	SetContextThink( &CINSCombineBall::AnimThink, gpGlobals->curtime + 0.1f, g_pszAnimThinkContext );
}
