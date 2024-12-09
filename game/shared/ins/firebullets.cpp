//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "firebullets.h"
#include "bulletdef.h"
#include "weapondef.h"
#include "shot_manipulator.h"
#include "takedamageinfo.h"
#include "debugoverlay_shared.h"
#include "ai_debug_shared.h"
#include "decals.h"
#include "effect_dispatch_data.h"

#ifdef GAME_DLL

#include "te_effect_dispatch.h"
#include "te_nmr_firebullets.h"
#include "ilagcompensationmanager.h"

#else

#include "c_te_effect_dispatch.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
ConVar showimpacts( "sv_showimpacts", "0", FCVAR_REPLICATED, "shows client (red) and server (blue) bullet impact point" );
ConVar showimpacts_time( "sv_showimpacts_time", "10", FCVAR_REPLICATED, "time that impacts are shown for" );
ConVar showpenetration( "sv_showpenetration", "1", FCVAR_REPLICATED, "shows penetration" );
ConVar allowpenetration( "sv_allowpenetration", "1", FCVAR_REPLICATED, "toggle for penetration" );

//=========================================================
//=========================================================
struct FireBulletsData_t
{
	FireBulletsData_t( const Vector &vecOrigin, const Vector &vecDir, int iTracerType )
	{
		m_vecOrigin = vecOrigin;
		m_vecDir = vecDir;
		m_iTracerType = iTracerType;
	}

	Vector m_vecOrigin;
	Vector m_vecDir;
	int m_iTracerType;
};

//=========================================================
//=========================================================
bool UTIL_VisibleTracer( int iTracerType )
{
	return ( iTracerType != TRACERTYPE_NONE && iTracerType != TRACERTYPE_DEFAULT );
}

//=========================================================
//=========================================================
void UTIL_ShowBullet( trace_t &tr, bool bHit )
{
	if ( !showimpacts.GetBool( ) || ( !bHit && !showpenetration.GetBool( ) ) )
		return;

#ifdef CLIENT_DLL

	if( bHit )
		debugoverlay->AddBoxOverlay( tr.endpos, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), QAngle( 0, 0, 0 ), 255, 0, 0, 127, showimpacts_time.GetFloat( ) );

	int iRed, iGreen, iBlue;
	iRed = iGreen = iBlue = 255;

	if( bHit )
	{
		iGreen = 0;
		iBlue = 0;
	}

	debugoverlay->AddLineOverlay( tr.startpos , tr.endpos, iRed, iGreen, iBlue, !bHit, showimpacts_time.GetFloat( ) );

#else

	if( bHit )
		NDebugOverlay::Box( tr.endpos, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 0, 0, 255, 127, showimpacts_time.GetFloat( ) );

	int iRed, iGreen, iBlue;
	iRed = iGreen = iBlue = 255;

	if( bHit )
	{
		iRed = 0;
		iGreen = 0;
	}
	
	NDebugOverlay::Line( tr.startpos, tr.endpos, iRed, iGreen, iBlue, !bHit, showimpacts_time.GetFloat( ) );

#endif
}

//=========================================================
//=========================================================
#define INCH_PER_CM 0.393700787f

float UTIL_PenetrationAmount( int iBulletType, float flVelocity, csurface_t &surf )
{
	surfacedata_t *s_data = physprops->GetSurfaceData( surf.surfaceProps );

	if ( !s_data )
		return 0.0f;

	const CBulletData &BulletData = CBulletDef::GetBulletData( iBulletType );
	
	float flMass = BulletData.m_iWeight; // grams
	float flXArea = BulletData.m_flXArea; // mm ^ 2

	if( flMass <= 0.0f || flXArea <= 0.0f )
		return 0.0f;
		
	float flDensity = s_data->physics.density * 0.001f; // g/cu mm 
	float flPascals = 1.0f;

	switch( s_data->game.material )
	{
		case CHAR_TEX_CONCRETE:
			flPascals = 185.0f;
			break;

		case CHAR_TEX_METAL:
		case CHAR_TEX_COMPUTER:
			flPascals = 500.0f;
			break;

		case CHAR_TEX_VENT:
		case CHAR_TEX_GRATE:
		case CHAR_TEX_TILE:
		case CHAR_TEX_PLASTIC:
			flPascals = 25.0f;
			break;

		case CHAR_TEX_DIRT:
		case CHAR_TEX_WOOD:
			flPascals = 15.0f;
			break;

		case CHAR_TEX_GLASS:
			flPascals = 50.0f;
			break;

		case CHAR_TEX_FLESH:
		case CHAR_TEX_BLOODYFLESH:
		case CHAR_TEX_ALIENFLESH:
			flPascals = 4.0f;
			break;
	}

	// PNOTE: I've lost where I got this forumula from
	// but it wants input in ^ 10-3

	return ( ( flMass / ( 2.0f * flDensity * flXArea ) ) * logf( ( flDensity * ( flVelocity * flVelocity ) + flPascals ) / flPascals ) * ( 100 * INCH_PER_CM ) );
}

//=========================================================
//=========================================================
#define RICOCHET_MASS_MAX 8
#define RICOCHET_VELOCITY_MIN 100
#define RICOCHET_VELOCITY_MAX 1200

bool UTIL_BulletRicochet( int iBulletType, float flVelocity, Vector &vecPlaneNormal, csurface_t &surf, Vector &vecDir )
{
	surfacedata_t *s_data = physprops->GetSurfaceData( surf.surfaceProps );

	if ( !s_data )
		return false;

	// only allow on concrete and metal (and computer)
	if( s_data->game.material != CHAR_TEX_CONCRETE && s_data->game.material != CHAR_TEX_METAL && s_data->game.material != CHAR_TEX_COMPUTER )
		return false;

	// don't allow a velocity too high or low
	if( flVelocity < RICOCHET_VELOCITY_MIN || flVelocity > RICOCHET_VELOCITY_MAX )
		return false;

	// don't allow a mass too high
	const CBulletData &BulletData = CBulletDef::GetBulletData( iBulletType );

	float flMass = BulletData.m_iWeight;

	if( flMass > RICOCHET_MASS_MAX )
		return false;

	// work out the angle
	float flAngle = M_PI - acos( DotProduct( vecPlaneNormal, vecDir ) );

	// ... don't allow reflex angles
	if( flAngle > M_PI )
		return false;

	// don't allow angles less than 30degrees
	if( flAngle <= 0.5f )
		return false;

	// fudge a result
	bool bPerfectRicochet = ( s_data->game.material != CHAR_TEX_CONCRETE );

	if( bPerfectRicochet || ( !bPerfectRicochet && random->RandomInt( 0, 1 ) != 0 ) )
	{
		// ... work out the new direction
		Vector vecOldDir = vecDir;

		vecDir = vecOldDir - ( 2.0f * vecPlaneNormal * DotProduct( vecOldDir, vecPlaneNormal ) );

		QAngle angNewDir;
		angNewDir.Init( );

		VectorAngles( vecDir, angNewDir );

		if( angNewDir.x > 270 )
			angNewDir.x = 355.0f;

		AngleVectors( angNewDir, &vecDir );

		// ... tweak the angle if it's not a perfect ricochet
		if( !bPerfectRicochet )
			vecDir[ 2 ] += vecDir[ 1 ] * random->RandomFloat( -0.2f, 0.2f );

		return true;
	}

	return false;
}

//=========================================================
//=========================================================
void UTIL_Tracer( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pEntity, int iTracerType, bool bFindStart )
{
	if( iTracerType == TRACERTYPE_NONE )
		return;

	CEffectData data;
	data.m_vStart = vecStart;
	data.m_vOrigin = vecEnd;

#ifdef GAME_DLL

	data.m_nEntIndex = pEntity->entindex( );

#else

	data.m_hEntity = pEntity;

#endif

	data.m_nDamageType = iTracerType;
	data.m_nHitBox = bFindStart ? 1 : 0;

	if( UTIL_VisibleTracer( iTracerType ) )
		DispatchEffect( "Tracer", data );
	else
		DispatchEffect( "TracerSound", data );
}

void UTIL_Tracer( const Vector &vecEnd, CBaseEntity *pEntity, int iTracerType )
{
	UTIL_Tracer( vec3_origin, vecEnd, pEntity, iTracerType, true );
}

void UTIL_Tracer( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pEntity, int iTracerType )
{
	UTIL_Tracer( vecStart, vecEnd, pEntity, iTracerType, false );
}

//=========================================================
//=========================================================
#define BULLET_POINTBLANK_DISTANCE 36.0f
#define BULLET_VELOCITY_LIMIT 100.0f
#define BULLET_VELOCITY_SCALE 4.0f

float UTIL_BulletImpact( int iBulletType, float flVelocity,  float flTravelled, CTakeDamageInfo *pDamage )
{
	const CBulletData &BulletData = CBulletDef::GetBulletData( iBulletType );

	float flBulletWeight, flBulletXArea, flBulletFormFactor;
	flBulletWeight = BulletData.m_iWeight;
	flBulletXArea = BulletData.m_flXArea;
	flBulletFormFactor = BulletData.m_flFormFactor;

	float flNewVelocity = flVelocity;

	// only play with velocities that have traveled further than point-blank
	if( flTravelled > BULLET_POINTBLANK_DISTANCE )
	{
		// work out how much to retard the velocity
		float flVelocityRetardation = 0.0f;
		Assert( flBulletFormFactor != 0.0f );

		if( flBulletFormFactor > 0.0f )
			flVelocityRetardation = flBulletXArea * ( 1 / flBulletFormFactor );

		// subtract the retard amount over a fraction of the distance traveled and
		// scale the velocity retardation to ensure weapons with a larger
		// value are more exaggerated
		flNewVelocity -= ( ( flTravelled - BULLET_POINTBLANK_DISTANCE ) * 0.001f ) * pow( flVelocityRetardation, 1.125f );

		// determine the min velocity by the formfactor - use powers
		// to ensure that the larger ones are more exaggerated
		flNewVelocity = max( flNewVelocity, pow( flBulletFormFactor, 4.5f ) * 25 );
	}

	// use the old velocity if smaller
	if( flNewVelocity > flVelocity )
		flNewVelocity = flVelocity;

	// now work out hatcher's formula
	if( pDamage )
		pDamage->SetDamage( GetWeaponDef( )->flDamageScale * flNewVelocity * flBulletWeight );

	return flNewVelocity;
}

//=========================================================
//=========================================================
class CTraceFilterBullet : public CTraceFilterSimple
{
public:
	CTraceFilterBullet( CBaseEntity *pAttacker )
		: CTraceFilterSimple( pAttacker, COLLISION_GROUP_NONE )
	{
	}

#ifdef CLIENT_DLL

	bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		if( !CTraceFilterSimple::ShouldHitEntity( pServerEntity, contentsMask ) )
			return false;

		C_BaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );

		if( !pEntity )
			return false;

		if( dynamic_cast< C_BaseViewModel* >( pEntity ) != NULL )
			return false;

		return true;
	}

#endif
};

//=========================================================
//=========================================================

// the base trace dist for starting in water
#define WATERTRAVEL_BASE 80

// the minimum velocity for penetration to occur
#define PENETRATION_VELOCITY_MIN 200

void UTIL_FireBullet( IFireBullets *pWeapon, FireBulletsData_t &FireBulletsData, CTakeDamageInfo *pDamage, ITraceFilter &filter, trace_t &tr, bool bUseHull )
{
	float flDistTrace, flDistTotal, flDistPenetration;
	flDistTrace = flDistTotal = flDistPenetration = 0.0f;

	// setup effects
	bool bHitSurface = false;
	bool bHadRicochet = false;

	// setup manipulator
	CShotManipulator manip( FireBulletsData.m_vecDir );

	Vector vecOrigin, vecDir, vecEnd, vecTracerEnd;
	vecTracerEnd.Init( );

	vecOrigin = FireBulletsData.m_vecOrigin;
	vecDir = manip.ApplySpread(pWeapon->GetSpread());

	CBaseEntity *pAttacker, *pInflictor;

	pAttacker = pWeapon->GetAttacker( );
	pInflictor = pWeapon->GetInflictor( );

	if( !pAttacker || !pInflictor )
		return;

	float flVelocity;
	int iBulletType, iDamageType, iRange;

	iBulletType = pWeapon->GetBulletType( );
	iDamageType = pWeapon->GetDamageType( );
	flVelocity = pWeapon->GetMuzzleVelocity( );
	iRange = pWeapon->GetRange( );

	// get bullet data
	const CBulletData &BulletData = CBulletDef::GetBulletData( iBulletType );

	// loop through until we hit something we can't penetrate
	bool bHitWater = false;

	while( true )
	{
		// check if we're starting in water
		bool bStartedInWater;
		bStartedInWater = bHitWater;

		if( !bStartedInWater )
			bStartedInWater = bHitWater = ( enginetrace->GetPointContents( vecOrigin ) & ( CONTENTS_WATER | CONTENTS_SLIME ) ) != 0;

		// work out trace distance
		flDistTrace = 0.0f;

		if( bStartedInWater )
			flDistTrace = WATERTRAVEL_BASE * BulletData.m_flFormFactor;
		else
			flDistTrace = iRange - flDistTotal;
		
		// don't trace really small amounts
		if( flDistTrace < 0.1f )
			break;

		vecEnd = vecOrigin + ( vecDir * flDistTrace );

		// do a trace
		int iCollisionGroup = MASK_SHOT;
		
		if( !bStartedInWater )
			iCollisionGroup |= CONTENTS_WATER | CONTENTS_SLIME;

		if( !bUseHull )
			AI_TraceLine( vecOrigin, vecEnd, iCollisionGroup, &filter, &tr );
		else
			AI_TraceHull( vecOrigin, vecEnd, Vector( -3, -3, -3 ), Vector( 3, 3, 3 ), iCollisionGroup, &filter, &tr );
			
		// show the trace if needed
		UTIL_ShowBullet( tr, true );

		// show a tracer if started out in water (when no other tracer is specified)
	#ifdef CLIENT_DLL

		if( bStartedInWater && !UTIL_VisibleTracer( FireBulletsData.m_iTracerType ) )
		{
			if( bHitSurface )
				UTIL_Tracer( tr.startpos, tr.endpos, pAttacker, TRACERTYPE_WATER );
			else
				UTIL_Tracer( tr.endpos, pAttacker, TRACERTYPE_WATER );
		}

	#endif

		// up the total trace
		flDistTotal += tr.fraction * flDistTrace;

		// set tracer end
		if( !bHadRicochet )
			vecTracerEnd = tr.endpos;

		// if we didn't hit anything or we've hot the sky, don't continue
		if( ( tr.surface.flags & ( SURF_SKY ) ) || !tr.DidHit( ) )
			break;

		// work out water stuff
	#ifdef CLIENT_DLL

		bHitWater = ( ( tr.surface.flags & SURF_NODECALS && 
			tr.surface.flags & SURF_WARP &&
			tr.surface.flags & SURF_TRANS ) || 
			( enginetrace->GetPointContents( tr.endpos ) & ( CONTENTS_WATER | CONTENTS_SLIME ) ) );

		if( bHitWater && !bStartedInWater )
		{
			int nMinSplashSize = BulletData.m_iMinSplashSize;
			int nMaxSplashSize = BulletData.m_iMaxSplashSize;

			CEffectData data;
			data.m_vOrigin = tr.endpos;
			data.m_vNormal = tr.plane.normal;
			data.m_flScale = random->RandomFloat( nMinSplashSize, nMaxSplashSize );

			if ( tr.contents & CONTENTS_SLIME )
				data.m_fFlags |= FX_WATER_IN_SLIME;

			DispatchEffect( "gunshotsplash", data );
		}

	#endif

		// find info on impact
		flVelocity = UTIL_BulletImpact( iBulletType, flVelocity, flDistTotal, pDamage );

		// add ricochet damage when occurred
		if( pDamage && bHadRicochet )
			pDamage->AddDamageType( DMG_RICOCHET );

		// trace attack
		if( pDamage )
		{
			CBaseEntity *pHitEntity = tr.m_pEnt;

			if( !bHitWater && pHitEntity )
			{
				CalculateBulletDamageForce( pDamage, iBulletType, flVelocity, vecDir, tr.endpos );

				pHitEntity->DispatchTraceAttack( *pDamage, vecDir, &tr );

			#ifdef GAME_DLL

				surfacedata_t *psurf = physprops->GetSurfaceData( tr.surface.surfaceProps );

				if( ( psurf != NULL ) && ( psurf->game.material == CHAR_TEX_GLASS ) && ( tr.m_pEnt->ClassMatches( "func_breakable" ) ) )
				{
					CEffectData	data;

					data.m_vNormal = tr.plane.normal;
					data.m_vOrigin = tr.endpos;

					DispatchEffect( "GlassImpact", data );
				}

			#endif
			}

		#ifdef GAME_DLL

			CBaseEntity::TraceAttackToTriggers( *pDamage, tr.startpos, tr.endpos, vecDir );

		#endif
		}

		// if we've started in water, don't ever penetrate or ricochet etc
		// or if the server doesn't allow it
		if( bStartedInWater || !allowpenetration.GetBool( ) )
			break;

		bHitSurface = true;

		// our origin is now the end position
		vecOrigin = tr.endpos;

		if( bHitWater )
			continue;

		// shall we richocet?
		bool bOldHadRicochet = bHadRicochet;

		if( !bOldHadRicochet && UTIL_BulletRicochet( iBulletType, flVelocity, tr.plane.normal, tr.surface, vecDir ) )
			bHadRicochet = true;

		// make impact (and make slash damage effect if ricochet)
	#ifdef CLIENT_DLL

		pInflictor->DoImpactEffect( tr, ( !bOldHadRicochet && bHadRicochet ) ? ( iDamageType | DMG_RICOCHET ) : iDamageType );

	#endif

		// don't do anything more when we've already had a richocet
		if( bOldHadRicochet && bHadRicochet )
			break;

		// continue to next bullet when had a ricochet
		if( bHadRicochet )
			continue;

		// don't penetrate when our velocity is too low
		if( flVelocity <= PENETRATION_VELOCITY_MIN )
			break;

		// try and penetrate what we've hit
		flDistPenetration = UTIL_PenetrationAmount( iBulletType, flVelocity, tr.surface );

		// don't trace small amounts
		if( flDistPenetration < 0.1f )
			break;

		// do a trace from the outside inwards, only hit solids
		Vector vecPenetration = vecDir * flDistPenetration;

		UTIL_TraceLine( vecOrigin + vecPenetration, vecOrigin, MASK_SHOT, &filter, &tr );
		UTIL_ShowBullet( tr, false );

		if( tr.startsolid )
			break;

		flVelocity = max( flVelocity - ( 1000 / flDistPenetration ), PENETRATION_VELOCITY_MIN );

		float flPenetrationFraction = 1.0f - tr.fraction;
		flDistTotal += flDistTrace * flPenetrationFraction;

		// set new origin
		vecOrigin = vecOrigin + ( vecPenetration * flPenetrationFraction );

		// add an impact on the other side of the penetration
		// but only if hit the world
	#ifdef CLIENT_DLL

		if( tr.DidHitWorld( ) )
			pInflictor->DoImpactEffect( tr, iDamageType );

	#endif
	}

#ifdef CLIENT_DLL

	UTIL_Tracer( vecTracerEnd, pInflictor, FireBulletsData.m_iTracerType );

#endif
};

//=========================================================
//=========================================================
void UTIL_FireBullets( IFireBullets *pWeapon, FireBulletsData_t &FireBulletsData, CTakeDamageInfo *pDamage, int iSeed )
{
	// PNOTE: this function is shared between local and thirdperson firing
	// client-firing does not handle any damage

	CBaseEntity *pAttacker = pWeapon->GetAttacker( );

	if( !pAttacker )
		return;

#ifdef GAME_DLL

	// should always have valid damage when server
	Assert( pDamage );

#endif

	CTraceFilterBullet filter( pAttacker );
	trace_t tr;

	int iShots = pWeapon->GetShots( );
	int iStartingDamageType = 0;

	if( pDamage )
		iStartingDamageType = pDamage->GetDamageType( );

	for ( int i = 0; i < iShots; i++ )
	{
		// use new seed for players
		if( pAttacker->IsPlayer( ) )
			RandomSeed( iSeed );

		// reset damagetype
		if( pDamage )
			pDamage->SetDamageType( iStartingDamageType );

		// use hulls for every other shot (easier to hit targets)
		if ( pAttacker->IsPlayer( ) && iShots > 1 && i % 2 )
			UTIL_FireBullet( pWeapon, FireBulletsData, pDamage, filter, tr, true );
		else
			UTIL_FireBullet( pWeapon, FireBulletsData, pDamage, filter, tr, false );

		// get new seed
		iSeed++;
	}
}

//=========================================================
//=========================================================
void UTIL_FireBullets( IFireBullets *pWeapon, const Vector &vecOrigin, const Vector &vecDir, int iTracerType )
{
	// PNOTE: this function is used when firing the weapon for the
	// local player or server-side entities doing firing

	CBaseEntity *pAttacker = pWeapon->GetAttacker( );

	if( !pAttacker )
		return;

	int iBulletType = pWeapon->GetBulletType( );
	Assert( iBulletType != INVALID_AMMODATA );

	if( iBulletType == INVALID_AMMODATA )
		return;

#ifdef GAME_DLL

	// move other players back to history positions based on local player's lag
	CBasePlayer *pPlayerAttacker = NULL;

	if( pAttacker->IsPlayer( ) )
	{
		pPlayerAttacker = ToBasePlayer( pAttacker );

		if( pPlayerAttacker )
			lagcompensation->StartLagCompensation( pPlayerAttacker, pPlayerAttacker->GetCurrentCommand( ) );
	}

#endif

	// make sure we don't have a dangling damage target from a recursive call
	if( g_MultiDamage.GetTarget( ) != NULL )
		ApplyMultiDamage( );
	  
	ClearMultiDamage( );

	// setup data
	FireBulletsData_t FireBulletsData( vecOrigin, vecDir, iTracerType );
	CTakeDamageInfo dmg( pWeapon->GetInflictor( ), pAttacker, 0.0f, pWeapon->GetDamageType( ) );

	// set damage type
	g_MultiDamage.SetDamageType( dmg.GetDamageType( ) );

	// setup seed
	int iSeed = 0;

	if( pAttacker->IsPlayer( ) )
		iSeed = CBaseEntity::GetPredictionRandomSeed( ) & 255;

	// now go through all the bullets
	UTIL_FireBullets( pWeapon, FireBulletsData, &dmg, iSeed );

#ifdef GAME_DLL

	// tell the other clients of the effect
	TE_FireBullets( pAttacker->entindex( ), vecOrigin, vecDir, iSeed ); // TODO !!!

	// apply damage
	ApplyMultiDamage( );

	// finish lag compression
	if( pPlayerAttacker )
		lagcompensation->FinishLagCompensation( pPlayerAttacker );

#endif
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

void UTIL_FireBulletsEffect( IFireBullets *pWeapon, const Vector &vecOrigin, const Vector &vecDir, int iSeed )
{
	// PNOTE: this function is used when doing client-side firing

	FireBulletsData_t FireBulletsData( vecOrigin, vecDir, TRACERTYPE_NONE );
	UTIL_FireBullets( pWeapon, FireBulletsData, NULL, iSeed );
}

#endif
