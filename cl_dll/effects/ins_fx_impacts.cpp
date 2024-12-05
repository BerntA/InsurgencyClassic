//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific impact effect hooks
//
//=============================================================================//
#include "cbase.h"
#include "fx_impact.h"
#include "fx_quad.h"
#include "engine/ienginesound.h"
#include "decals.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef TESTING

ConVar ignoreimpacts( "cl_ignoreimpacts", "0" );

#endif

//=========================================================
//=========================================================
void ImpactCallback( const CEffectData &data )
{
#ifdef TESTING

	if( ignoreimpacts.GetBool( ) )
		return;

#endif

	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;

	C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

	if ( !pEntity )
		return;

	// if we hit, perform our custom effects and play the sound
	if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr ) )
	{
		bool bIsRicochet = ( data.m_nDamageType & DMG_RICOCHET );

		// check for custom effects based on the decal index
		PerformCustomEffects( vecOrigin, tr, vecShotDir, iMaterial, bIsRicochet ? 0.75f : 1.0f, 0 );

		//Play a ricochet sound some of the time
		if( ( iDamageType == DMG_BULLET ) && ( bIsRicochet || ( random->RandomInt( 1, 10 ) <= 3 ) ) )
		{
			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound( filter, SOUND_FROM_WORLD, "Bounce.Shrapnel", &vecOrigin );
		}
	}

	PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
}

DECLARE_CLIENT_EFFECT( "Impact", ImpactCallback );

//=========================================================
//=========================================================
void AR2ImpactCallback( const CEffectData &data )
{
	FX_AddQuad( data.m_vOrigin, 
				data.m_vNormal, 
				random->RandomFloat( 24, 32 ),
				0,
				0.75f, 
				1.0f,
				0.0f,
				0.4f,
				random->RandomInt( 0, 360 ), 
				0,
				Vector( 1.0f, 1.0f, 1.0f ), 
				0.25f, 
				"effects/combinemuzzle2_nocull",
				(FXQUAD_BIAS_SCALE|FXQUAD_BIAS_ALPHA) );
}

DECLARE_CLIENT_EFFECT( "AR2Impact", AR2ImpactCallback );