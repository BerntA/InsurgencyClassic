//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h" 

#include "weapon_melee_base.h"
#include "in_buttons.h" 
#include "takedamageinfo.h"
#include "keyvalues.h"

#ifdef GAME_DLL

#include "ilagcompensationmanager.h"
#include "te_effect_dispatch.h"

#endif

// TODO: maybe revisit this one day in terms of animations

//=========================================================
//=========================================================
#define MELEE_FIRERATE 0.9f
#define MELEE_TIME2IDLE 3.0f
#define MELEE_RANGE 50.0f

#define MELEE_HULL_DIM 16 
static const Vector g_MeleeMins( -MELEE_HULL_DIM, -MELEE_HULL_DIM, -MELEE_HULL_DIM ); 
static const Vector g_MeleeMaxs( MELEE_HULL_DIM, MELEE_HULL_DIM, MELEE_HULL_DIM ); 

#define MASK_MELEE ( MASK_SHOT | CONTENTS_GRATE )

//=========================================================
//=========================================================
MeleeWeaponInfo_t::MeleeWeaponInfo_t( )
{
	flDamage = 0.0f;
}

void MeleeWeaponInfo_t::Parse( KeyValues *pKeyValuesData, const char *pszWeaponName )
{
	BaseClass::Parse( pKeyValuesData, pszWeaponName );

	flDamage = pKeyValuesData->GetFloat( "damage" );
}

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMeleeBase, DT_WeaponMeleeBase )

BEGIN_NETWORK_TABLE( CWeaponMeleeBase, DT_WeaponMeleeBase )
END_NETWORK_TABLE( )

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponMeleeBase ) 

	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ), 

END_PREDICTION_DATA()

#endif

LINK_ENTITY_TO_CLASS( weapon_melee_base, CWeaponMeleeBase );

//=========================================================
//=========================================================
CWeaponMeleeBase::CWeaponMeleeBase( )
{ 
}

//=========================================================
//=========================================================
void CWeaponMeleeBase::PrimaryAttack( void )
{ 
#ifdef GAME_DLL

	CBasePlayer *pPlayer = GetOwner( ); 
	
	if ( !pPlayer ) 
		return;

	// move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, pPlayer->GetCurrentCommand( ) );

#endif

	Swing( );

#ifdef GAME_DLL

	// move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( pPlayer );

#endif
}

//=========================================================
//=========================================================
void CWeaponMeleeBase::AddViewKick( void )
{ 
	// Do we have a valid owner holding the weapon? 
	CBasePlayer *pPlayer = GetOwner( ); 
	
	if ( !pPlayer ) 
		return; 
	
	// Shake 'em!
	pPlayer->ViewPunch( QAngle( random->RandomFloat( -1.0f, 1.0f ), random->RandomFloat( -1.0f, 1.0f ), 0.0f ) ); 
}

//=========================================================
//=========================================================
void CWeaponMeleeBase::Swing( void ) 
{ 
	CBasePlayer *pPlayer = GetOwner( ); 
	
	if ( !pPlayer ) 
		return; 
	
	WeaponSound( SHOT_SINGLE ); 
	
	SendWeaponAnim( ACT_VM_HITCENTER ); 

	m_flNextPrimaryAttack = gpGlobals->curtime + MELEE_FIRERATE; 
	m_flTimeWeaponIdle = gpGlobals->curtime + MELEE_TIME2IDLE; 
}

//=========================================================
//=========================================================
void CWeaponMeleeBase::FindHullIntersection( trace_t &tr, const Vector &vecMins, const Vector &vecMaxs, CBasePlayer *pPlayer ) 
{ 
	int i, j, k;
	float flDistance;
	const float	*pMinMaxs[ 2 ] = { vecMins.Base( ), vecMaxs.Base( ) };
	trace_t tmpTrace;
	Vector vecHullEnd = tr.endpos;
	Vector vecEnd;

	flDistance = 1e6f;
	Vector vecSrc = tr.startpos;

	vecHullEnd = vecSrc + ( ( vecHullEnd - vecSrc ) * 2 );
	UTIL_TraceLine( vecSrc, vecHullEnd, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tmpTrace );

	if( tmpTrace.fraction == 1.0f )
	{
		for( i = 0; i < 2; i++ )
		{
			for( j = 0; j < 2; j++ )
			{
				for( k = 0; k < 2; k++ )
				{
					vecEnd.x = vecHullEnd.x + pMinMaxs[ i ][ 0 ];
					vecEnd.y = vecHullEnd.y + pMinMaxs[ j ][ 1 ];
					vecEnd.z = vecHullEnd.z + pMinMaxs[ k ][ 2 ];

					UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tmpTrace );

					if( tmpTrace.fraction < 1.0 )
					{
						float flThisDistance = ( tmpTrace.endpos - vecSrc ).Length( );

						if( flThisDistance < flDistance )
						{
							tr = tmpTrace;
							flDistance = flThisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		tr = tmpTrace;
	}
}

//=========================================================
//=========================================================
void CWeaponMeleeBase::Hit( trace_t &tr ) 
{ 
	CBasePlayer *pPlayer = GetOwner( ); 
	
	if ( !pPlayer ) 
		return; 
	
	// shake the screen a little 
	AddViewKick( ); 
	
	// true if we hit a target 
	if( tr.m_pEnt != NULL ) 
	{ 
		Vector vForward; pPlayer->EyeVectors( &vForward, NULL, NULL ); 
		VectorNormalize( vForward ); 

		// process the damage and send it to the entity we just hit 
		CTakeDamageInfo dmgInfo( this, GetOwner( ), GET_WEAPON_DATA_CUSTOM( MeleeWeaponInfo_t, flDamage ), DMG_CLUB );
		CalculateMeleeDamageForce( &dmgInfo, vForward, tr.endpos ); 
		tr.m_pEnt->DispatchTraceAttack( dmgInfo, vForward, &tr ); 
		ApplyMultiDamage( );

	#ifdef GAME_DLL

		// now hit all triggers along the ray
		TraceAttackToTriggers( dmgInfo, tr.startpos, tr.endpos, vForward );

	#endif 
	}

	// apply an impact effect 
	ImpactEffect( tr ); 
}

//=========================================================
//=========================================================
bool CWeaponMeleeBase::ImpactWater( const Vector &vStart, const Vector &vEnd ) 
{ 
	// FIXME: this doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//        right now anyway...
	
	// We must start outside the water
	if( UTIL_PointContents( vStart ) & ( CONTENTS_WATER | CONTENTS_SLIME ) )
		return false;

	// We must end inside of water
	if ( !( UTIL_PointContents( vEnd ) & ( CONTENTS_WATER | CONTENTS_SLIME ) ) )
		return false;

#ifdef GAME_DLL

	trace_t	waterTrace;

	UTIL_TraceLine( vStart, vEnd, ( CONTENTS_WATER | CONTENTS_SLIME ), GetOwner( ), COLLISION_GROUP_NONE, &waterTrace );

	if ( waterTrace.fraction < 1.0f )
	{
		CEffectData	data;

		data.m_fFlags  = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if( waterTrace.contents & CONTENTS_SLIME )
			data.m_fFlags |= FX_WATER_IN_SLIME;

		DispatchEffect( "watersplash", data );			
	}

#endif

	return true;
}

//=========================================================
//=========================================================
void CWeaponMeleeBase::ImpactEffect( trace_t &tr )
{ 
	// see if we hit water (we don't do the other impact effects in this case)
	if ( ImpactWater( tr.startpos, tr.endpos ) )
		return;

	UTIL_ImpactTrace( &tr, DMG_SLASH ); 
}

//=========================================================
//=========================================================
void CWeaponMeleeBase::FindHit( void )
{
	CBasePlayer *pPlayer = GetOwner( );
	
	if ( !pPlayer )
		return; 

	// get the player's position 
	Vector vecSwingStart = pPlayer->Weapon_ShootPosition( ); 
	Vector vecForward; 

	pPlayer->EyeVectors( &vecForward, NULL, NULL ); 
	
	// get attack end position using the weapon's range 
	Vector vecSwingEnd = vecSwingStart + vecForward * MELEE_RANGE;

	trace_t tr; 
	UTIL_TraceLine( vecSwingStart, vecSwingEnd, MASK_MELEE, pPlayer, COLLISION_GROUP_NONE, &tr ); 
	
	// didn't hit jack shit - so let's check for hull trace now 
	if ( tr.fraction == 1.0f ) 
	{ 
		// hull is +/- 16, so use cube-root of 2 to determine how big the hull is from center to the corner point
		static float flMeleeHullRadius = 1.732f * MELEE_HULL_DIM; 
		
		// pull back from the end by "hull radius" amount
		vecSwingEnd -= vecForward * flMeleeHullRadius; 
		UTIL_TraceHull( vecSwingStart, vecSwingEnd, g_MeleeMins, g_MeleeMaxs, MASK_MELEE, pPlayer, COLLISION_GROUP_NONE, &tr ); 
		
		// see if we hit a wall,etc or an Entity
		if( tr.fraction < 1.0 && tr.m_pEnt ) 
		{ 
			// get the position of the entity we just hit 
			Vector vecToTarget = tr.m_pEnt->GetAbsOrigin( ) - vecSwingStart;
			VectorNormalize( vecToTarget ); 
			float flDot = vecToTarget.Dot( vecForward ); // angle between us and entity
			
			// see if our angle is enough to see this entity
			if( flDot < 0.70721f ) 
			{ 
				// force a miss 
				tr.fraction = 1.0f; 
			} 
			else 
			{ 
				// see if we should impact based on the melee swing
				FindHullIntersection( tr, g_MeleeMins, g_MeleeMaxs, pPlayer ); 
			} 
		} 
	} 

	// see if we hit water along the way 
	ImpactWater( vecSwingStart, tr.endpos ); 

	if( tr.fraction != 1.0f ) 
		Hit( tr ); 
}

//=========================================================
//=========================================================
bool CWeaponMeleeBase::HasAmmo(void) const
{
	return true;
}

#ifdef GAME_DLL

//=========================================================
//=========================================================
void CWeaponMeleeBase::HandleAnimEvent(animevent_t *pEvent)
{
	if( pEvent->event == MELEE_EVENT )
		FindHit( );
	else
		BaseClass::HandleAnimEvent( pEvent );
}

#else

//=========================================================
//=========================================================
bool C_WeaponMeleeBase::OnFireEvent( C_BaseViewModel *pViewModel, const Vector& origin, const QAngle& angles, int event, const char *options )
{
	if( event == MELEE_EVENT )
	{
		FindHit( );

		return true;
	}

	return OnFireEvent( pViewModel, origin, angles, event, options );
}

#endif
