//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "ins_utils.h"
#include "firebullets.h"
#include "ins_gamerules.h"
#include "play_team_shared.h"
#include "ins_recipientfilter.h"
#include "ins_area.h"
#include "decals.h"
#include "soundemittersystem/isoundemittersystembase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern short g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion
extern short g_sModelIndexSmoke;		// (in combatweapon.cpp) holds the index for the smoke cloud

#define EXPLOSION_SHAKE_AMPLITUDE 25.0f
#define EXPLOSION_SHAKE_RADIUS 750.0f

//=========================================================
//=========================================================
void UTIL_CreateExplosion( const Vector &vecOrigin, CBaseEntity *pAttacker, CBaseEntity *pInflictor, int iDamage, int iDamageRadius, int iExtraDamageFlags, const char *pszSound, trace_t *pCheckTrace, Vector *pReported )
{
	if( !INSRules( )->DetonationsAllowed( ) )
		return;

	Vector vecAdjustedOrigin = vecOrigin;

	// create a checktrace if not passed one
	trace_t tr;

	if( !pCheckTrace )
	{
		Vector vecSpot;

		vecSpot = vecOrigin + Vector( 0, 0, 8 );
		UTIL_TraceLine( vecSpot, vecSpot + Vector( 0, 0, -32 ), MASK_SHOT_HULL, pInflictor, COLLISION_GROUP_NONE, &tr );

		pCheckTrace = &tr;
	}

	// pull out of the wall a bit and work out if a big explosion or not
	bool bLargeExplosion = true;

	if( pCheckTrace->fraction != 1.0f )
	{
		vecAdjustedOrigin = pCheckTrace->endpos + ( pCheckTrace->plane.normal * 0.6f );

		surfacedata_t *s_data = physprops->GetSurfaceData( pCheckTrace->surface.surfaceProps );

		if( s_data && s_data->game.material == CHAR_TEX_METAL )
			bLargeExplosion = false;
	}

	// make the effect
	int iContents = UTIL_PointContents( vecAdjustedOrigin );
	int iEffectsDamage = iDamage * ( bLargeExplosion ? 4 : 1 );

	if( pCheckTrace->fraction != 1.0 )
	{
		Vector vecNormal = pCheckTrace->plane.normal;

		surfacedata_t *pdata = physprops->GetSurfaceData( pCheckTrace->surface.surfaceProps );	

		CPASFilter filter( vecAdjustedOrigin );

		te->Explosion( filter, -1.0, // don't apply cl_interp delay
			&vecAdjustedOrigin,
			!( iContents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			iDamageRadius * 0.1f, 
			25,
			TE_EXPLFLAG_NONE,
			iDamageRadius,
			iEffectsDamage,
			&vecNormal,
			( char )pdata->game.material );
	}
	else
	{
		CPASFilter filter( vecAdjustedOrigin );

		te->Explosion( filter, -1.0, // don't apply cl_interp delay
			&vecAdjustedOrigin, 
			!( iContents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			iDamageRadius * 0.03f, 
			25,
			TE_EXPLFLAG_NONE,
			iDamageRadius,
			iEffectsDamage );
	}

	// do the damage
	Vector vecReported = ( pReported ) ? *pReported : vec3_origin;
	CTakeDamageInfo info( pInflictor, pAttacker, vec3_origin, vecOrigin, iDamage, DMG_BLAST | iExtraDamageFlags, 0, &vecReported );

	RadiusDamage( info, vecOrigin, iDamageRadius, CLASS_NONE, pInflictor, true );

	// add a decal
	UTIL_DecalTrace( pCheckTrace, "Scorch" );

	// make the sound
	if( pInflictor )
	{
		pInflictor->EmitSound( pszSound ? pszSound : INS_EXPLOSION_DEFAULTSOUND );
	}
	else
	{
		// TODO: emit sound from origin
	}

	// shake the screen
	UTIL_ScreenShake( vecAdjustedOrigin, EXPLOSION_SHAKE_AMPLITUDE, 150.0, 1.0, EXPLOSION_SHAKE_RADIUS, SHAKE_START );
}

//=========================================================
//=========================================================
void UTIL_SendHint( CINSPlayer *pPlayer, int iHintID )
{
	g_HintHelper.SendHint( pPlayer, iHintID );
}

//=========================================================
//=========================================================
const char *UTIL_FindPlaceName( const Vector &vecPos )
{
	for( int i = 0; i < g_INSAreas.Count( ); i++ )
	{
		CINSArea *pArea = g_INSAreas[ i ];
		Assert( pArea );

		if( !pArea )
			continue;

		Vector vecMins, vecMaxs;
		vecMins = pArea->WorldAlignMins( );
		vecMaxs = pArea->WorldAlignMaxs( );

		if( vecPos.x > vecMaxs.x || vecPos.y > vecMaxs.y || vecPos.z > vecMaxs.z )
			continue;

		if( vecPos.x < vecMins.x || vecPos.y < vecMins.y || vecPos.z < vecMins.z )
			continue;

		return pArea->GetTitle( );
	}

	return NULL;
}

//=========================================================
//=========================================================
bool IsExplosionTraceBlocked( trace_t *ptr )
{
	if( ptr->DidHitWorld( ) )
		return true;

	if( ptr->m_pEnt == NULL )
		return false;

	if( ptr->m_pEnt->GetMoveType( ) == MOVETYPE_PUSH )
	{
		// all doors are push, but not all things that push are doors. This 
		// narrows the search before we start to do classname compares.
		if( FClassnameIs( ptr->m_pEnt, "prop_door_rotating" ) ||
        FClassnameIs( ptr->m_pEnt, "func_door" ) ||
        FClassnameIs( ptr->m_pEnt, "func_door_rotating" ) )
			return true;
	}

	return false;
}

//=========================================================
//=========================================================
void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore )
{
	RadiusDamage( info, vecSrcIn, flRadius, iClassIgnore, pEntityIgnore, ( info.GetDamageType( ) & DMG_BLAST ) ? true : false );
}

//=========================================================
//=========================================================

// TODO: the way in which this works out if it can 'see' the player
// is by doing a trace and then using that as the blocking entity
// however this raises the question - what if it finds, lets say a barrel
// but there is a thick wall behind it?? probarly needs todo two traces - one 
// for entities one for walls

#define ROBUST_RADIUS_PROBE_DIST 16.0f // if a solid surface blocks the explosion, this is how far to creep along the surface looking for another way to the target
#define RADIUS_CONCUSSION 1.75f

#ifdef _DEBUG

ConVar showradiusdmg( "sv_showradiusdmg", "1", FCVAR_GAMEDLL, "Show Radius Damage", true, 0, true, 1 );

#endif

void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore, bool bSendConcussion )
{
	const int MASK_RADIUS_DAMAGE = MASK_SHOT & ( ~CONTENTS_HITBOX );
	CBaseEntity *pEntity = NULL;
	trace_t tr;
	float flAdjustedDamage, falloff;
	Vector vecSpot;

	Vector vecSrc = vecSrcIn;

	if ( flRadius )
		falloff = info.GetDamage( ) / flRadius;
	else
		falloff = 1.0;

	int bInWater = ( UTIL_PointContents ( vecSrc ) & MASK_WATER ) ? true : false;

	// in case grenade is lying on the ground
	vecSrc.z += 1;

	float flHalfRadiusSqr = Square( flRadius / 2.0f );
	float flFullRadius = flRadius;

	if( bSendConcussion )
		flFullRadius *= RADIUS_CONCUSSION;

#ifdef _DEBUG

	if( showradiusdmg.GetBool( ) )
	{
		float flAdjustedRadius = flRadius * ( ( M_PI * flRadius * flRadius ) / ( flRadius * flRadius * 4 ) );
		NDebugOverlay::Box( vecSrcIn, Vector( -flAdjustedRadius, -flAdjustedRadius, -flAdjustedRadius ), Vector( flAdjustedRadius, flAdjustedRadius, flAdjustedRadius ), 255, 0, 0, 127, 5.0f );

		if( bSendConcussion )
		{
			float flAdjustedFullRadius = flFullRadius * ( ( M_PI * flFullRadius * flFullRadius ) / (flFullRadius * flFullRadius * 4));
			NDebugOverlay::Box( vecSrcIn, Vector( -flAdjustedFullRadius, -flAdjustedFullRadius, -flAdjustedFullRadius ), Vector( flAdjustedFullRadius, flAdjustedFullRadius, flAdjustedFullRadius ), 130, 255, 130, 55, 5.0f );
		}
	}

#endif

	// iterate on all entities in the vicinity
	for( CEntitySphereQuery sphere( vecSrc, flFullRadius );
		( pEntity = sphere.GetCurrentEntity( ) ) != NULL;
		sphere.NextEntity( ) )
	{
		// This value is used to scale damage when the explosion is blocked by some other object.
		float flBlockedDamagePercent = 0.0f;

		if( pEntity == pEntityIgnore )
			continue;

		if( pEntity->m_takedamage == DAMAGE_NO )
			continue;

		// UNDONE: this should check a damage mask, not an ignore
		// houndeyes don't hurt other houndeyes with their attack
		if( iClassIgnore != CLASS_NONE && pEntity->Classify( ) == iClassIgnore )
			continue;

		// blasts don't travel into or out of water
		if( bInWater && pEntity->GetWaterLevel( ) == 0 )
			continue;

		if( !bInWater && pEntity->GetWaterLevel( ) == 3 )
			continue;

		// check that the explosion can 'see' this entity.
		vecSpot = pEntity->BodyTarget( vecSrc, true );
		UTIL_TraceLine( vecSrc, vecSpot, MASK_RADIUS_DAMAGE, pEntityIgnore, COLLISION_GROUP_NONE, &tr );

#ifdef _DEBUG

		if( showradiusdmg.GetBool( ) )
		{
			if( tr.m_pEnt )
				NDebugOverlay::Line( vecSrc, tr.endpos, 255, 0, 0, false, 15.0f );
			else
				NDebugOverlay::Line( vecSrc, tr.endpos, 255, 255, 255, false, 15.0f );

			if( tr.m_pEnt && tr.m_pEnt->IsPlayer( ) )
			{
				CBasePlayer *player = ToBasePlayer( tr.m_pEnt );
				player->DrawServerHitboxes( 4, true );
			}
		}

#endif

		bool bMinorConcussion = false;

		if( tr.fraction != 1.0 )
		{
			if( IsExplosionTraceBlocked( &tr ) )
			{
				// only use robust model on a target within one-half of the explosion's radius.
				if( vecSpot.DistToSqr( vecSrc ) <= flHalfRadiusSqr )
				{
					Vector vecToTarget = vecSpot - tr.endpos;
					VectorNormalize( vecToTarget );

					// we're going to deflect the blast along the surface that 
					// interrupted a trace from explosion to this target.
					Vector vecUp, vecDeflect;
					CrossProduct( vecToTarget, tr.plane.normal, vecUp );
					CrossProduct( tr.plane.normal, vecUp, vecDeflect );
					VectorNormalize( vecDeflect );

					// trace along the surface that intercepted the blast...
					UTIL_TraceLine( tr.endpos, tr.endpos + vecDeflect * ROBUST_RADIUS_PROBE_DIST, MASK_SHOT, info.GetInflictor( ), COLLISION_GROUP_NONE, &tr );

					// ...to see if there's a nearby edge that the explosion would 'spill over' if the blast were fully simulated.
					UTIL_TraceLine( tr.endpos, vecSpot, MASK_SHOT, info.GetInflictor( ), COLLISION_GROUP_NONE, &tr );

					// ensure we can reach the target
					if( tr.fraction != 1.0 && tr.DidHitWorld( ) )
						bMinorConcussion = true;
				}
				else
				{
					bMinorConcussion = true;
				}
			}

			// UNDONE: probably shouldn't let children block parents either? or maybe those guys should set their owner if they want this behavior?
			if( !bMinorConcussion && tr.m_pEnt && tr.m_pEnt != pEntity && tr.m_pEnt->GetOwnerEntity( ) != pEntity )
			{
				// some entity was hit by the trace, meaning the explosion does not have clear
				// line of sight to the entity that it's trying to hurt. if the world is also
				// blocking, we do no damage
				CBaseEntity *pBlockingEntity = tr.m_pEnt;

				UTIL_TraceLine( vecSrc, vecSpot, CONTENTS_SOLID, pEntity, COLLISION_GROUP_NONE, &tr );

				if( tr.fraction == 1.0 )
				{
					// now, if the interposing object is physics, block some explosion force based on its mass
					if( pBlockingEntity->VPhysicsGetObject( ) )
					{
						const float MASS_ABSORB_ALL_DAMAGE = 350.0f;
						float flMass = pBlockingEntity->VPhysicsGetObject( )->GetMass( );
						float flScale = flMass / MASS_ABSORB_ALL_DAMAGE;

						// check if its absorbed all the damage
						if( flScale >= 1.0f )
							continue;

						Assert( flScale > 0.0f );
						flBlockedDamagePercent = flScale;
					}
					else
					{
						// some object that's not the world and not physics. generically block 25% damage.
						flBlockedDamagePercent = 0.25f;
					}
				}
			}
		}

		float flDistance = ( vecSrc - tr.endpos ).Length( );

		if( flDistance > flRadius )
			bMinorConcussion = true;

		if( !bMinorConcussion )
		{
			// decrease damage for an ent that's farther from the bomb
			flAdjustedDamage = flDistance * falloff;
			flAdjustedDamage = info.GetDamage( ) - flAdjustedDamage;

			if( flAdjustedDamage > 0 )
			{	
				// the explosion can 'see' this entity, so hurt them!
				if( tr.startsolid )
				{
					// if we're stuck inside them, fixup the position and distance
					tr.endpos = vecSrc;
					tr.fraction = 0.0;
				}
		
				CTakeDamageInfo adjustedInfo = info;
				adjustedInfo.SetDamage( flAdjustedDamage - ( flAdjustedDamage * flBlockedDamagePercent ) );

				Vector dir = vecSpot - vecSrc;
				VectorNormalize( dir );

				// if we don't have a damage force, manufacture one
				if( adjustedInfo.GetDamagePosition( ) == vec3_origin || adjustedInfo.GetDamageForce( ) == vec3_origin )
				{
					CalculateExplosiveDamageForce( &adjustedInfo, dir, vecSrc );
				}
				else
				{
					// assume the force passed in is the maximum force. Decay it based on falloff.
					float flForce = adjustedInfo.GetDamageForce( ).Length( ) * falloff;
					adjustedInfo.SetDamageForce( dir * flForce );
					adjustedInfo.SetDamagePosition( vecSrc );
				}

				if ( tr.fraction != 1.0 && pEntity == tr.m_pEnt )
				{
					ClearMultiDamage( );
					pEntity->DispatchTraceAttack( adjustedInfo, dir, &tr );
					ApplyMultiDamage( );
				}
				else
				{
					pEntity->TakeDamage( adjustedInfo );
				}

				// now hit all triggers along the way that respond to damage... 
				pEntity->TraceAttackToTriggers( adjustedInfo, vecSrc, tr.endpos, dir );
			}
		}

		if( bSendConcussion && pEntity->IsPlayer( ) )
		{
			CINSPlayer *pPlayer = ToINSPlayer( pEntity );
			pPlayer->ConcussionEffect( bMinorConcussion );
		}
	}
}

#ifdef TESTING

//=========================================================
//=========================================================
class CGunConsole : public IFireBullets
{
public:
	CGunConsole( )
	{
		m_pAttacker = GetContainingEntity( INDEXENT( 0 ) );
	}

	CBaseEntity *GetAttacker( void ) { return m_pAttacker; }
	CBaseEntity *GetInflictor( void ) { return m_pAttacker; }
	int GetBulletType( void ) const { return BULLET_556NATO; }
	int GetMuzzleVelocity( void ) const { return 600; }
	int GetShots( void ) const { return 1; }
	Vector GetSpread( void ) const { return vec3_origin; }
	int GetRange( void ) const { return 4096; }

private:
	CBaseEntity *m_pAttacker;
};

#ifdef _DEBUG

#define INS_VIRTUALGUN_FLAGS ( FCVAR_GAMEDLL )

#else

#define INS_VIRTUALGUN_FLAGS ( FCVAR_GAMEDLL | FCVAR_CHEAT )

#endif

#define INS_VIRTUALGUN_USAGE "Console Command Gun - Shoot People\n" \
	"Usage: ins_virtualgun <target id> [bone]\n"

CON_COMMAND_F( ins_virtualgun, INS_VIRTUALGUN_USAGE, FCVAR_GAMEDLL )
{
	if( engine->Cmd_Argc( ) < 2 )
	{
		Msg( "%s\n", INS_VIRTUALGUN_USAGE );
		return;
	}

	// find target
	CBasePlayer *pPlayer = UTIL_PlayerByIndex( atoi( engine->Cmd_Argv( 1 ) ) );

	if( !pPlayer )
	{
		Msg( "Invalid Player\n" );
		return;
	}

	// find bone
	const char *pszBone = NULL;

	if( engine->Cmd_Argc( ) > 2 )
		pszBone = engine->Cmd_Argv( 2 );

	if( !pszBone )
		pszBone = "Spine";

	int iBoneID = pPlayer->LookupBone( pszBone );

	if( iBoneID < 0 )
	{
		Msg( "Invalid Bone\n" );
		return;
	}

	// execute gun
	Vector vecEye, vecEyeDir, vecOrigin, vecTarget;
	
	vecEye = pPlayer->EyePosition( );

	QAngle angTmp = pPlayer->EyeAngles( );
	angTmp[ PITCH ] = 0;

	AngleVectors( angTmp, &vecEyeDir );
	vecEyeDir.NormalizeInPlace( );

	VectorMA( vecEye, 100.0f, vecEyeDir, vecOrigin );

	pPlayer->GetBonePosition( iBoneID, vecTarget, angTmp );

	CGunConsole GunConsole;
	UTIL_FireBullets( &GunConsole, vecOrigin, vecTarget - vecOrigin, TRACERTYPE_NONE );
}

#endif

//=========================================================
//=========================================================
int UTIL_WeaponStatisticType( int iHitGroup )
{
	switch( iHitGroup )
	{
		case HITGROUP_HEAD:
		case HITGROUP_NECK:
			return WEAPONSTATS_HITS_HEAD;

		case HITGROUP_LEFTUPPERARM:
		case HITGROUP_LEFTFOREARM:
			return WEAPONSTATS_HITS_LARM;

		case HITGROUP_RIGHTUPPERARM:
		case HITGROUP_RIGHTFOREARM:
			return WEAPONSTATS_HITS_RARM;

		case HITGROUP_LEFTTHIGH:
		case HITGROUP_LEFTCALF:
		case HITGROUP_LEFTFOOT:
			return WEAPONSTATS_HITS_LLEG;

		case HITGROUP_RIGHTTHIGH:
		case HITGROUP_RIGHTCALF:
		case HITGROUP_RIGHTFOOT:
			return WEAPONSTATS_HITS_RLEG;
	}

	return WEAPONSTATS_HITS_BODY;
}

//=========================================================
//=========================================================
void UTIL_SendKeyValues( KeyValues *pData )
{
	if( !pData )
	{
		WRITE_BYTE( 0 );
		return;
	}

	KeyValues *pSubData = pData->GetFirstSubKey( );

	if( !pSubData )
	{
		WRITE_BYTE( 0 );
		return;
	}

	// start writing
	WRITE_BYTE( 1 );
	
	// be careful not more than 192 bytes!
	bool bFirstData = true;

	for( ; pSubData; pSubData = pSubData->GetNextKey( ) )
	{
		if( pSubData->GetDataType( ) != KeyValues::TYPE_INT && pSubData->GetDataType( ) != KeyValues::TYPE_STRING )
		{
			if( bFirstData )
				return;

			continue;
		}

		if( !bFirstData )
			WRITE_BYTE( 1 );

		WRITE_BYTE( pSubData->GetDataType( ) );

		WRITE_STRING( pSubData->GetName( ) );

		if( pSubData->GetDataType( ) == KeyValues::TYPE_INT )
			WRITE_BYTE( pSubData->GetInt( ) );
		else
			WRITE_STRING( pSubData->GetString( ) );

		bFirstData = false;
	}

	WRITE_BYTE( 0 );
}

//=========================================================
//=========================================================

#ifdef _DEBUG

struct LineElement_t
{
	Vector m_vecStart, m_vecEnd;
	Color m_LineColor;
};

static CUtlVector< LineElement_t > g_LineList;
static bool g_bDrawnLineList = false;

void UTIL_AddLineList( const Vector &vecStart, const Vector &vecEnd, const Color &LineColor )
{
	int iID = g_LineList.AddToTail( );
	LineElement_t &LineElement = g_LineList[ iID ];

	LineElement.m_vecStart = vecStart;
	LineElement.m_vecEnd = vecEnd;
	LineElement.m_LineColor = LineColor;
}

//=========================================================
//=========================================================
void UTIL_DrawLineList( void )
{
	if( g_bDrawnLineList )
		return;

	for( int i = 0; i < g_LineList.Count( ); i++ )
	{
		LineElement_t &LineElement = g_LineList[ i ];
		NDebugOverlay::Line( LineElement.m_vecStart, LineElement.m_vecEnd, LineElement.m_LineColor[ 0 ], LineElement.m_LineColor[ 1 ], LineElement.m_LineColor[ 2 ], false, FLT_MAX );
	}

	g_bDrawnLineList = true;
}

//=========================================================
//=========================================================
void UTIL_ClearLineList( void )
{
	g_LineList.RemoveAll( );
	g_bDrawnLineList = false;
}

#endif

//=========================================================
//=========================================================
Vector UTIL_SpawnPositionOffset( CBaseEntity *pEntity )
{
	return ( pEntity->GetAbsOrigin( ) + Vector( 0, 0, 1 ) );
}