//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_weaponcache_shared.h"
#include "ins_obj.h"
#include "grenade_c4.h"
#include "vcollide_parse.h"
#include "imc_config.h"
#include "ins_player.h"
#include "ins_gamerules.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_weaponcache, CINSWeaponCache );

BEGIN_DATADESC( CINSWeaponCache )

	DEFINE_KEYFIELD( m_iID, FIELD_INTEGER, "crateid" )

END_DATADESC( )

//=========================================================
//=========================================================
CINSWeaponCache::CINSWeaponCache( )
{
	m_iState = WCACHESTATE_INVALID;
	m_pParent = NULL;
	m_pObjParent = NULL;

	m_takedamage = DAMAGE_NO;
}

//=========================================================
//=========================================================
void CINSWeaponCache::Spawn( void )
{
	BaseClass::Spawn( );

	UpdateState( );
}

//=========================================================
//=========================================================
bool CINSWeaponCache::CreateVPhysics( void )
{
	solid_t tmpSolid;
	PhysModelParseSolid( tmpSolid, this, GetModelIndex( ) );

	IPhysicsObject *pPhysicsObject = VPhysicsInitNormal( SOLID_VPHYSICS, 0, true, &tmpSolid );

	if( !pPhysicsObject )
		return false;

	pPhysicsObject->EnableMotion( false );

	return true;
}

//=========================================================
//=========================================================
void CINSWeaponCache::Setup( CIMCWeaponCache *pWeaponCache )
{
	if( m_iState != WCACHESTATE_INVALID )
		return;

	m_pParent = pWeaponCache;
	m_iState = WCACHESTATE_HIDDEN;
}

//=========================================================
//=========================================================
bool CINSWeaponCache::HasIMCParent( void ) const
{
	return ( m_pParent != NULL );
}

//=========================================================
//=========================================================
void CINSWeaponCache::SetObjParent( CINSObjective *pParent )
{
	m_pObjParent = pParent;
}

//=========================================================
//=========================================================
bool CINSWeaponCache::HasObjParent( void ) const
{
	return ( m_pParent != NULL );
}

//=========================================================
//=========================================================
int CINSWeaponCache::GetTeam( void )
{
	return ( m_pObjParent ? m_pObjParent->GetCapturedTeam( ) : TEAM_NEUTRAL );
}

//=========================================================
//=========================================================
int CINSWeaponCache::GetFlags( void ) const
{
	return ( m_pParent ? m_pParent->GetFlags( ) : 0 );
}

//=========================================================
//=========================================================
void CINSWeaponCache::Create( void )
{
	if( m_iState != WCACHESTATE_HIDDEN )
		return;

	SetState( WCACHESTATE_ACTIVE );
}

//=========================================================
//=========================================================
void CINSWeaponCache::Remove( void )
{
	if( m_iState != WCACHESTATE_ACTIVE )
		return;

	SetState( WCACHESTATE_HIDDEN );
}

//=========================================================
//=========================================================
void CINSWeaponCache::Restock( void )
{
	if( !HasIMCParent( ) )
		return;

	if( m_iState != WCACHESTATE_ACTIVE )
		return;

	// TODO: refill inventory
}

//=========================================================
//=========================================================
void CINSWeaponCache::SetState( int iState )
{
	if( !HasIMCParent( ) )
		return;

	m_iState = iState;
	UpdateState( );
}

//=========================================================
//=========================================================
void CINSWeaponCache::UpdateState( void )
{
	if( !HasIMCParent( ) )
		return;

	if( m_iState == WCACHESTATE_ACTIVE )
	{
		if( m_takedamage == DAMAGE_NO )
		{
			m_takedamage = DAMAGE_YES;
			RemoveEffects( EF_NODRAW );

			CreateVPhysics( );

			Restock( );
		}
	}
	else
	{
		if( m_takedamage == DAMAGE_YES )
		{
			AddEffects( EF_NODRAW );
			m_takedamage = DAMAGE_NO;

			SetSolid( SOLID_NONE );
			VPhysicsDestroyObject( );
		}
	}
}

//=========================================================
//=========================================================
void CINSWeaponCache::TraceAttack( const CTakeDamageInfo &info, const Vector &dir, trace_t *ptr )
{
	// ensure valid inflictor
	if( !info.GetInflictor( ) || !( info.GetDamageType( ) & ( DMG_BLAST | DMG_ENGINEER ) ) )
		return;

	// ensure valid attacker
	CINSPlayer *pAttacker = ToINSPlayer( info.GetAttacker( ) );

	if( !pAttacker || !pAttacker->IsPlayer( ) )
		return;

	// check for sabotage when disabled
	if( !INSRules( )->CanSabotageCaches( ) && pAttacker->GetTeamID( ) == GetTeam( ) )
		return;

	Destory( pAttacker );
}

//=========================================================
//=========================================================
void CINSWeaponCache::Destory( CINSPlayer *pDestoryer )
{
	// destory me
	Break( );

	// i've been destroyed!
	if( !m_pObjParent )
		m_pObjParent->CacheDestroyed( this, pDestoryer );

	// update its state
	m_iState = WCACHESTATE_HIDDEN;
	UpdateState( );
}

//=========================================================
//=========================================================
void CINSWeaponCache::Break( void )
{
	/*Vector velocity;
	AngularImpulse angVelocity;
	IPhysicsObject *pPhysics = VPhysicsGetObject();

	Vector origin;
	QAngle angles;
	AddSolidFlags( FSOLID_NOT_SOLID );

	if ( pPhysics )
	{
	pPhysics->GetVelocity( &velocity, &angVelocity );
	pPhysics->GetPosition( &origin, &angles );
	pPhysics->RecheckCollisionFilter();
	}
	else
	{
	velocity = GetAbsVelocity();
	QAngleToAngularImpulse( GetLocalAngularVelocity(), angVelocity );
	origin = GetAbsOrigin();
	angles = GetAbsAngles();
	}

	PhysBreakSound( this, VPhysicsGetObject(), GetAbsOrigin() );

	// in multiplayer spawn break models as clientside temp ents
	CPASFilter filter( WorldSpaceCenter() );

	Vector velocity; velocity.Init();

	if ( pPhysics )
	pPhysics->GetVelocity( &velocity, NULL );

	te->PhysicsProp( filter, -1, GetModelIndex(), GetAbsOrigin(), GetAbsAngles(), velocity, true );*/
}
