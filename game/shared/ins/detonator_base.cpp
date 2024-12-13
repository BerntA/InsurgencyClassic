//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "detonator_base.h"

#ifdef GAME_DLL

#include "ins_utils.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef GAME_DLL

BEGIN_DATADESC( CBaseDetonator )

	DEFINE_THINKFUNC( Detonate ),

END_DATADESC( )

#endif

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( BaseDetonator, DT_BaseDetonator )
BEGIN_NETWORK_TABLE( CBaseDetonator, DT_BaseDetonator )

#ifdef GAME_DLL

SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),
SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
CBaseDetonator::CBaseDetonator( )
{
#ifdef GAME_DLL

	m_iAmmoID = INVALID_AMMODATA;

#endif
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

CBaseDetonator *CBaseDetonator::CreateDetonator( CBasePlayer *pOwner, int iAmmoType, int iAmmoID, const Vector &vecPosition, const QAngle &angAngles )
{
	if( iAmmoID < 0 )
	{
		AssertMsg( false, "CBaseDetonator::CreateDetonator, Invalid AmmoID" );
		return false;
	}

	const char *pszClass = NULL;

	switch( iAmmoType )
	{
	case AMMOTYPE_GRENADE:
		pszClass = g_pszGrenadeList[ iAmmoID ];
		break;

	case AMMOTYPE_MISSILE:
		pszClass = g_pszMissileList[ iAmmoID ];
		break;
	}

	if( !pszClass )
	{
		AssertMsg( false, "CBaseDetonator::CreateDetonator, Unregistered Detonator" );
		return false;
	}
	
	CBaseDetonator *pBaseDetonator = static_cast< CBaseDetonator* >( CBaseEntity::Create( pszClass, vecPosition, angAngles, pOwner ) );

	if( !pBaseDetonator )
		return NULL;

	pBaseDetonator->SetAmmoID( iAmmoID );

	return pBaseDetonator;
}

#endif

//=========================================================
//=========================================================
void CBaseDetonator::Spawn( void )
{
#ifdef GAME_DLL

	SetSolidFlags( FSOLID_NOT_STANDABLE );
	SetSolid( SOLID_BBOX );

	Vector vecMins, vecMaxs;
	GetDetonatorBounds( vecMins, vecMaxs );

	SetSize( vecMins, vecMaxs );

#endif
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

//=========================================================
//=========================================================
CBasePlayer *CBaseDetonator::GetPlayerOwner( void ) const
{
	return ToBasePlayer( GetOwnerEntity( ) );
}

//=========================================================
//=========================================================
void CBaseDetonator::Setup( void )
{
	const char *pszModel = GetDetonatorModel( );
	Assert( pszModel );

	SetModel( pszModel );
}

//=========================================================
//=========================================================
void CBaseDetonator::SetAmmoID( int iAmmoID )
{
	m_iAmmoID = iAmmoID;
}

//=========================================================
//=========================================================
const char *CBaseDetonator::GetDetonatorModel( void ) const
{
	return NULL;
}

//=========================================================
//=========================================================
void CBaseDetonator::GetDetonatorBounds( Vector &vecMins, Vector &vecMaxs ) const
{
	vecMins = vecMaxs = vec3_origin;
}

//=========================================================
//=========================================================
const char *CBaseDetonator::GetDetonatorSound( void ) const
{
	return NULL;
}

//=========================================================
//=========================================================
void CBaseDetonator::Explode( trace_t *pCheckTrace )
{
	int iDamage, iDamageRadius;
	GetExplosionDamage( iDamage, iDamageRadius );

	CBasePlayer *pOwner = GetPlayerOwner( );

	if( !pOwner )
		return;

	Vector vecReported = pOwner->GetAbsOrigin( );

	UTIL_CreateExplosion( GetAbsOrigin( ), pOwner, this, iDamage, iDamageRadius, GetExplosionDamageFlags( ), GetDetonatorSound( ), pCheckTrace, &vecReported );

	Destroy( );
}

//=========================================================
//=========================================================
void CBaseDetonator::GetExplosionDamage( int &iDamage, int &iDamageRadius )
{
	iDamage = iDamageRadius = 0;
}

//=========================================================
//=========================================================
int CBaseDetonator::GetExplosionDamageFlags( void ) const
{
	return 0;
}

//=========================================================
//=========================================================
void CBaseDetonator::Destroy( void )
{
	SetModelName( NULL_STRING );
	m_takedamage = DAMAGE_NO;

	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetTouch( NULL );

	SetThink( &CBaseDetonator::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 0.001f );
}

//=========================================================
//=========================================================
void CBaseDetonator::Event_Killed( const CTakeDamageInfo &info )
{
	Detonate( );

	if( IsEffectActive( EF_NODRAW ) )
		Destroy( );
}

//=========================================================
//=========================================================
CBasePlayer *CBaseDetonator::GetScorer( void ) const
{
	return ToBasePlayer( GetOwnerEntity( ) );
}

//=========================================================
//=========================================================
int CBaseDetonator::GetInflictorID( void ) const
{
	return m_iAmmoID;
}

#endif
