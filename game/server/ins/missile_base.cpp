//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "missile_base.h"
#include "missiledef.h"
#include "ins_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CBaseMissile::CBaseMissile( )
{
	m_bDetonated = false;
}

//=========================================================
//=========================================================
void CBaseMissile::Configure( const Vector &vecDirection )
{
	m_vecInitialDirection = vecDirection;
}

//=========================================================
//=========================================================
void CBaseMissile::Setup( void )
{
	BaseClass::Setup( );

	// set the original origin
	CBasePlayer *pPlayer = GetPlayerOwner( );

	if( pPlayer )
		m_vecOriginalOrigin = pPlayer->GetAbsOrigin( );
}

//=========================================================
//=========================================================
const char *CBaseMissile::GetDetonatorModel( void ) const
{
	const CMissileData &MissileData = CMissileDef::GetMissileData( m_iAmmoID );
	return MissileData.m_szModel;
}

//=========================================================
//=========================================================
const char *CBaseMissile::GetDetonatorSound( void ) const
{
	const CMissileData &MissileData = CMissileDef::GetMissileData( m_iAmmoID );
	return MissileData.m_szDetonationSound;
}

//=========================================================
//=========================================================
int CBaseMissile::GetSafetyRange( void ) const
{
	const CMissileData &MissileData = CMissileDef::GetMissileData( m_iAmmoID );
	return MissileData.m_iSafetyRange;
}

//=========================================================
//=========================================================
void CBaseMissile::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );

	CBaseEntity *pHitEntity = pEvent->pEntities[ !index ];

	if( !pHitEntity || pHitEntity == GetOwnerEntity( ) )
		return;

	OnCollision( );

	if( m_bDetonated )
		return;

	if( pHitEntity->IsSolidFlagSet( FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS ) )
		return;

	if( pHitEntity->GetCollisionGroup( ) == COLLISION_GROUP_WEAPON || pHitEntity->GetCollisionGroup( ) == COLLISION_GROUP_BREAKABLE_GLASS )
		return;

	Vector vecOrigin = m_vecOriginalOrigin - GetAbsOrigin( );

	if( vecOrigin.Length( ) < GetSafetyRange( ) )
	{
		m_bDetonated = true;

		OnSafetyHit( );
	}
	else
	{
		Detonate( );
	}
}

//=========================================================
//=========================================================
void CBaseMissile::Detonate( void )
{
	Vector vecOrigin, vecForward;
	trace_t tr;

	vecOrigin = GetAbsOrigin( );
	GetVectors( &vecForward, NULL, NULL );

	UTIL_TraceLine( vecOrigin, vecOrigin + vecForward * 32, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );

	if( tr.fraction != 1.0f && tr.surface.flags & SURF_SKY )
	{
		Destroy( );

		return;
	}

	Explode( &tr );
}

//=========================================================
//=========================================================
void CBaseMissile::GetExplosionDamage( int &iDamage, int &iDamageRadius )
{
	const CMissileData &MissileData = CMissileDef::GetMissileData( m_iAmmoID );
	iDamage = MissileData.m_iDamage;
	iDamageRadius = MissileData.m_iDamageRadius;
}

//=========================================================
//=========================================================
int CBaseMissile::GetInflictorType( void ) const
{
	return INFLICTORTYPE_MISSILE;
}