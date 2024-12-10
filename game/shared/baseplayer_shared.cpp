//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements shared baseplayer class functionality
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "movevars_shared.h"
#include "util_shared.h"
#include "datacache/imdlcache.h"

#if defined( CLIENT_DLL )

	#include "prediction.h"
	#include "c_basedoor.h"
	#include "c_world.h"
	#include "view.h"
    #include "c_te_effect_dispatch.h"
	#include "c_playerresource.h"
	#include <game/client/iviewport.h>

	#define CRecipientFilter C_RecipientFilter

#else

	#include "trains.h"
	#include "world.h"
	#include "doors.h"
	#include "particle_parse.h"

	extern int TrainSpeed(int iSpeed, int iMax);
	
#endif

#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "tier0/vprof.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "decals.h"
#include "obstacle_pushaway.h"
#include "gamemovement.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if defined(GAME_DLL) && !defined(_XBOX)
	extern ConVar sv_pushaway_max_force;
	extern ConVar sv_pushaway_force;
	extern ConVar sv_turbophysics;

	class CUsePushFilter : public CTraceFilterEntitiesOnly
	{
	public:
		bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
		{
			CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

			// Static prop case...
			if ( !pEntity )
				return false;

			// Only impact on physics objects
			if ( !pEntity->VPhysicsGetObject() )
				return false;

			return g_pGameRules->CanEntityBeUsePushed( pEntity );
		}
	};
#endif

void CopySoundNameWithModifierToken( char *pchDest, const char *pchSource, int nMaxLenInChars, const char *pchToken )
{
	// Copy the sound name
	int nSource = 0;
	int nDest = 0;
	bool bFoundPeriod = false;

	while ( pchSource[ nSource ] != '\0' && nDest < nMaxLenInChars - 2 )
	{
		pchDest[ nDest ] = pchSource[ nSource ];
		nDest++;
		nSource++;

		if ( !bFoundPeriod && pchSource[ nSource - 1 ] == '.' )
		{
			// Insert special token after the period
			bFoundPeriod = true;

			int nToken = 0;

			while ( pchToken[ nToken ] != '\0' && nDest < nMaxLenInChars - 2 )
			{
				pchDest[ nDest ] = pchToken[ nToken ];
				nDest++;
				nToken++;
			}
		}
	}

	pchDest[ nDest ] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CBasePlayer::GetTimeBase( void ) const
{
	return m_nTickBase * TICK_INTERVAL;
}

//-----------------------------------------------------------------------------
// Purpose: Called every usercmd by the player PreThink
//-----------------------------------------------------------------------------
void CBasePlayer::ItemPreFrame()
{
	// Handle use events
	PlayerUse();

    if ( gpGlobals->curtime < m_flNextAttack )
		return;

	CBaseCombatWeapon* pActive = GetActiveWeapon();
	if (!pActive)
		return;

#if defined( CLIENT_DLL )
	// Not predicting this weapon
	if ( !pActive->IsPredicted() )
		return;
#endif

	pActive->ItemPreFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Called every usercmd by the player PostThink
//-----------------------------------------------------------------------------
void CBasePlayer::ItemPostFrame()
{
	VPROF( "CBasePlayer::ItemPostFrame" );

	// Put viewmodels into basically correct place based on new player origin
	CalcViewModelView(EyePosition(), EyeAngles());

	// switch weapon when needed
	if (m_flNextActiveWeapon != 0.0f && gpGlobals->curtime >= m_flNextActiveWeapon)
	{
		CBaseCombatWeapon* pNextWeapon = m_hNextActiveWeapon;

		if (pNextWeapon)
		{
			Weapon_SwitchToNext();
		}
		else
		{
			m_flNextActiveWeapon = 0.0f;

			CBaseViewModel* pViewModel = GetViewModel();
			if (pViewModel)
				pViewModel->RemoveEffects(EF_NODRAW);
		}
	}

	CBaseCombatWeapon* pWeapon = GetActiveWeapon();
	if (pWeapon)
	{
		if (gpGlobals->curtime >= m_flNextAttack)
			pWeapon->ItemPostFrame();
		else
			pWeapon->ItemBusyFrame();
	}
}

//-----------------------------------------------------------------------------
// Eye angles
//-----------------------------------------------------------------------------
const QAngle &CBasePlayer::EyeAngles( )
{
	// NOTE: Viewangles are measured *relative* to the parent's coordinate system
	CBaseEntity *pMoveParent = const_cast<CBasePlayer*>(this)->GetMoveParent();

	if ( !pMoveParent )
	{
		return pl.v_angle;
	}

	// FIXME: Cache off the angles?
	matrix3x4_t eyesToParent, eyesToWorld;
	AngleMatrix( pl.v_angle, eyesToParent );
	ConcatTransforms( pMoveParent->EntityToWorldTransform(), eyesToParent, eyesToWorld );

	static QAngle angEyeWorld;
	MatrixAngles( eyesToWorld, angEyeWorld );
	return angEyeWorld;
}


const QAngle &CBasePlayer::LocalEyeAngles()
{
	return pl.v_angle;
}

//-----------------------------------------------------------------------------
// Actual Eye position + angles
//-----------------------------------------------------------------------------
Vector CBasePlayer::EyePosition( )
{
#ifdef CLIENT_DLL
	if (IsObserver())
	{
		if (GetObserverMode() == OBS_MODE_CHASE)
		{
			if (IsLocalPlayer())
			{
				return MainViewOrigin();
			}
		}
	}
#endif
	return BaseClass::EyePosition();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector CBasePlayer::GetPlayerMins( void ) const
{
	return VEC_OBS_HULL_MIN_SCALED(this);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector CBasePlayer::GetPlayerMaxs( void ) const
{	
	return VEC_OBS_HULL_MAX_SCALED(this);
}

//-----------------------------------------------------------------------------
// Returns eye vectors
//-----------------------------------------------------------------------------
void CBasePlayer::EyeVectors( Vector *pForward, Vector *pRight, Vector *pUp )
{
	AngleVectors(EyeAngles(), pForward, pRight, pUp);
}

//-----------------------------------------------------------------------------
// Purpose: Returns the eye position and angle vectors.
//-----------------------------------------------------------------------------
void CBasePlayer::EyePositionAndVectors( Vector *pPosition, Vector *pForward,
										 Vector *pRight, Vector *pUp )
{
	VectorCopy(BaseClass::EyePosition(), *pPosition);
	AngleVectors(EyeAngles(), pForward, pRight, pUp);
}

#ifdef CLIENT_DLL
surfacedata_t * CBasePlayer::GetFootstepSurface( const Vector &origin, const char *surfaceName )
{
	return physprops->GetSurfaceData( physprops->GetSurfaceIndex( surfaceName ) );
}
#endif

surfacedata_t *CBasePlayer::GetLadderSurface( const Vector &origin )
{
#ifdef CLIENT_DLL
	return GetFootstepSurface( origin, "ladder" );
#else
	return physprops->GetSurfaceData( physprops->GetSurfaceIndex( "ladder" ) );
#endif
}

void CBasePlayer::UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity )
{
	// moved to INS player code
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : step - 
//			fvol - 
//			force - force sound to play
//-----------------------------------------------------------------------------
void CBasePlayer::PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force )
{
#if defined( CLIENT_DLL )
	// during prediction play footstep sounds only once
	if ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		return;
#endif

	if ( !psurface )
		return;

	int nSide = m_Local.m_nStepside;
	unsigned short stepSoundName = nSide ? psurface->sounds.stepleft : psurface->sounds.stepright;
	if ( !stepSoundName )
		return;

	m_Local.m_nStepside = !nSide;

	CSoundParameters params;

	Assert( nSide == 0 || nSide == 1 );

	if ( m_StepSoundCache[ nSide ].m_usSoundNameIndex == stepSoundName )
	{
		params = m_StepSoundCache[ nSide ].m_SoundParameters;
	}
	else
	{
		IPhysicsSurfaceProps *physprops = MoveHelper()->GetSurfaceProps();
		const char *pSoundName = physprops->GetString( stepSoundName );

		// Give child classes an opportunity to override.
		pSoundName = GetOverrideStepSound( pSoundName );

		if ( !CBaseEntity::GetParametersForSound( pSoundName, params, NULL ) )
			return;

		// Only cache if there's one option.  Otherwise we'd never here any other sounds
		if ( params.count == 1 )
		{
			m_StepSoundCache[ nSide ].m_usSoundNameIndex = stepSoundName;
			m_StepSoundCache[ nSide ].m_SoundParameters = params;
		}
	}

	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

#ifndef CLIENT_DLL
	// in MP, server removes all players in the vecOrigin's PVS, these players generate the footsteps client side
	if ( gpGlobals->maxClients > 1 )
	{
		filter.RemoveRecipientsByPVS( vecOrigin );
	}
#endif

	EmitSound_t ep;
	ep.m_nChannel = CHAN_AUTO;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = fvol;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
	OnEmitFootstepSound( params, vecOrigin, fvol );
}

void CBasePlayer::UpdateButtonState( int nUserCmdButtonMask )
{
#ifdef CLIENT_DLL // INS WARN HACK
	if (gViewPortInterface->GetActivePanel() && gViewPortInterface->GetActivePanel()->HasInputElements())
		nUserCmdButtonMask = 0;
#endif

	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	m_afButtonLast = m_nButtons;

	// Get button states
	m_nButtons = nUserCmdButtonMask;
 	int buttonsChanged = m_afButtonLast ^ m_nButtons;
	
	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_afButtonPressed =  buttonsChanged & m_nButtons;		// The changed ones still down are "pressed"
	m_afButtonReleased = buttonsChanged & (~m_nButtons);	// The ones not down are "released"
}

Vector CBasePlayer::Weapon_ShootPosition( )
{
	return EyePosition();
}

Vector CBasePlayer::Weapon_ShootDirection()
{
	Vector	forward;
#ifdef CLIENT_DLL
	AngleVectors(GetAbsAngles() + m_Local.m_vecRecoilPunchAngle, &forward);
#else
	AngleVectors(EyeAngles() + m_Local.m_vecRecoilPunchAngle, &forward);
#endif

	return	forward;
}

//-----------------------------------------------------------------------------
// Purpose: Set the weapon to switch to when the player uses the 'lastinv' command
//-----------------------------------------------------------------------------
void CBasePlayer::Weapon_SetLast( CBaseCombatWeapon *pWeapon )
{
	m_hLastWeapon = pWeapon;
}

//-----------------------------------------------------------------------------
// Purpose: Override base class so player can reset autoaim
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CBasePlayer::Weapon_Switch(CBaseCombatWeapon *pWeapon, bool bForce)
{
	MDLCACHE_CRITICAL_SECTION();

	CBaseCombatWeapon* pLastWeapon = GetActiveWeapon();

	// quit out if invalid
	if (pWeapon == NULL)
		return false;

	// ensure we are able to switch
	if (!Weapon_CanSwitchTo(pWeapon))
		return false;

	// already have it out?
	if (pLastWeapon == pWeapon)
	{
		// deploy again if invisible
		if (!pLastWeapon->IsWeaponVisible())
			pLastWeapon->Deploy();

		return false;
	}

	// find the deploy threshold of the lastweapon
	m_flNextActiveWeapon = 0.0f;

	if (pLastWeapon && pLastWeapon->GetOwner())
	{
		if (!pLastWeapon->Holster(pWeapon/*, bForce*/))
			return false;

		if (!bForce)
		{
			m_flNextActiveWeapon = pLastWeapon->GetHolsterTime();

			if (m_flNextActiveWeapon == 0.0f)
				bForce = true;
		}
	}
	else
	{
		bForce = true;
	}

	// set the next active weapon
	m_hNextActiveWeapon = pWeapon;

	// don't delay when forcing
	if (bForce)
		Weapon_SwitchToNext();

	// set the last weapon
	if (pLastWeapon)
		Weapon_SetLast(pLastWeapon);

	// don't hide the viewmodel anymore
	CBaseViewModel* pViewModel = GetViewModel();
	Assert(pViewModel);

	if (pViewModel)
		pViewModel->RemoveEffects(EF_NODRAW);

	return true;
}

void CBasePlayer::Weapon_SwitchToNext(void)
{
	CBaseCombatWeapon* pNewWeapon = m_hNextActiveWeapon;
	if (!pNewWeapon)
		return;

	// set as active weapon and deploy
	m_hActiveWeapon = pNewWeapon;
	pNewWeapon->Deploy();

	// reset
	m_flNextActiveWeapon = 0.0f;
	m_hNextActiveWeapon = NULL;

	// reset blends
#ifdef CLIENT_DLL
	CBaseViewModel* pVM = GetViewModel();
	if (pVM)
		pVM->m_SequenceTransitioner.m_animationQueue.RemoveAll();
#endif
}

void CBasePlayer::SelectLastItem(void)
{
	if (m_hLastWeapon.Get() == NULL)
		return;

	if (GetActiveWeapon() && !GetActiveWeapon()->CanHolster())
		return;

	SelectItem(m_hLastWeapon.Get()->GetWeaponID());
}

//-----------------------------------------------------------------------------
// Purpose: Abort any reloads we're in
//-----------------------------------------------------------------------------
void CBasePlayer::AbortReload( void )
{
	CBaseCombatWeapon* pWeapon = GetActiveWeapon();
	if (pWeapon)
		pWeapon->AbortReload();
}

#if !defined( NO_ENTITY_PREDICTION )
void CBasePlayer::AddToPlayerSimulationList( CBaseEntity *other )
{
	CHandle< CBaseEntity > h;
	h = other;
	// Already in list
	if ( m_SimulatedByThisPlayer.Find( h ) != m_SimulatedByThisPlayer.InvalidIndex() )
		return;

	Assert( other->IsPlayerSimulated() );

	m_SimulatedByThisPlayer.AddToTail( h );
}

//-----------------------------------------------------------------------------
// Purpose: Fixme, this should occur if the player fails to drive simulation
//  often enough!!!
// Input  : *other - 
//-----------------------------------------------------------------------------
void CBasePlayer::RemoveFromPlayerSimulationList( CBaseEntity *other )
{
	if ( !other )
		return;

	Assert( other->IsPlayerSimulated() );
	Assert( other->GetSimulatingPlayer() == this );

	CHandle< CBaseEntity > h;
	h = other;

	m_SimulatedByThisPlayer.FindAndRemove( h );
}

void CBasePlayer::SimulatePlayerSimulatedEntities( void )
{
	int c = m_SimulatedByThisPlayer.Count();
	int i;

	for ( i = c - 1; i >= 0; i-- )
	{
		CHandle< CBaseEntity > h;
		
		h = m_SimulatedByThisPlayer[ i ];
		CBaseEntity *e = h;

		if ( !e || !e->IsPlayerSimulated() )
		{
			m_SimulatedByThisPlayer.Remove( i );
			continue;
		}

#if defined( CLIENT_DLL )
		if ( e->IsClientCreated() && prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		{
			continue;
		}
#endif
		Assert( e->IsPlayerSimulated() );
		Assert( e->GetSimulatingPlayer() == this );

		e->PhysicsSimulate();
	}

	// Loop through all entities again, checking their untouch if flagged to do so
	c = m_SimulatedByThisPlayer.Count();

	for ( i = c - 1; i >= 0; i-- )
	{
		CHandle< CBaseEntity > h;
		
		h = m_SimulatedByThisPlayer[ i ];

		CBaseEntity *e = h;
		if ( !e || !e->IsPlayerSimulated() )
		{
			m_SimulatedByThisPlayer.Remove( i );
			continue;
		}

#if defined( CLIENT_DLL )
		if ( e->IsClientCreated() && prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		{
			continue;
		}
#endif

		Assert( e->IsPlayerSimulated() );
		Assert( e->GetSimulatingPlayer() == this );

		if ( !e->GetCheckUntouch() )
			continue;

		e->PhysicsCheckForEntityUntouch();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::ClearPlayerSimulationList( void )
{
	int c = m_SimulatedByThisPlayer.Size();
	int i;

	for ( i = c - 1; i >= 0; i-- )
	{
		CHandle< CBaseEntity > h;
		
		h = m_SimulatedByThisPlayer[ i ];
		CBaseEntity *e = h;
		if ( e )
		{
			e->UnsetPlayerSimulated();
		}
	}

	m_SimulatedByThisPlayer.RemoveAll();
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Return true if we should allow selection of the specified item
//-----------------------------------------------------------------------------
bool CBasePlayer::Weapon_ShouldSelectItem( CBaseCombatWeapon *pWeapon )
{
	return (pWeapon != GetActiveWeapon());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::SelectItem(int iWeaponID)
{
	if (GetObserverMode() != OBS_MODE_NONE)
		return;

	CBaseCombatWeapon *pItem = Weapon_OwnsThisType(iWeaponID);
	if (!pItem || !Weapon_ShouldSelectItem(pItem))
		return;

	// FIX, this needs to queue them up and delay
	// Make sure the current weapon can be holstered
	if (GetActiveWeapon() && !GetActiveWeapon()->CanHolster())
		return;

	Weapon_Switch(pItem);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ConVar sv_debug_player_use( "sv_debug_player_use", "0", FCVAR_REPLICATED, "Visualizes +use logic. Green cross=trace success, Red cross=trace too far, Green box=radius success" );
float IntervalDistance( float x, float x0, float x1 )
{
	// swap so x0 < x1
	if ( x0 > x1 )
	{
		float tmp = x0;
		x0 = x1;
		x1 = tmp;
	}

	if ( x < x0 )
		return x0-x;
	else if ( x > x1 )
		return x - x1;
	return 0;
}

class CTraceFilterPlayerUse : public CTraceFilterSimple
{
public:
	DECLARE_CLASS(CTraceFilterPlayerUse, CTraceFilterSimple);

	CTraceFilterPlayerUse(IHandleEntity *pHandleEntity, int collisionGroup, int team) : BaseClass(pHandleEntity, collisionGroup)
	{ 
		iWantedTeam = team;
	}

	virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
	{
		CBaseEntity *pEntity = EntityFromEntityHandle(pHandleEntity);
		if (pEntity)
		{
			if (pEntity->IsPlayer() && (pEntity->GetTeamNumber() == iWantedTeam))
				return false;
		}

		return BaseClass::ShouldHitEntity(pHandleEntity, contentsMask);
	}

protected:
	int iWantedTeam;
};

#ifndef CLIENT_DLL
class CTraceFilterWeaponsAndItems : public CTraceFilterSimple
{
public:
	DECLARE_CLASS(CTraceFilterWeaponsAndItems, CTraceFilterSimple);

	CTraceFilterWeaponsAndItems(IHandleEntity *pHandleEntity, int collisionGroup, int priority) : BaseClass(pHandleEntity, collisionGroup)
	{
		m_iItemPriority = priority;
	}

	virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
	{
		CBaseEntity *pEntity = EntityFromEntityHandle(pHandleEntity);
		if (pEntity)
		{
			if (pEntity->IsCombatCharacter())
				return false;

			if (m_iItemPriority > 0)
			{
				if (pEntity->IsBaseCombatWeapon() || (pEntity->GetItemPrio() && (pEntity->GetItemPrio() != m_iItemPriority)))
					return false; // Don't care.

				if (pEntity->GetItemPrio() == m_iItemPriority)
					return true;
			}
			else if (pEntity->IsBaseCombatWeapon() || pEntity->GetItemPrio())
				return true;
		}

		return BaseClass::ShouldHitEntity(pHandleEntity, contentsMask);
	}

private:
	int m_iItemPriority;
};
#endif

CBaseEntity *CBasePlayer::FindUseEntity()
{
	CTraceFilterPlayerUse trFiltr(this, COLLISION_GROUP_NONE, GetTeamNumber());

	Vector forward, up;
	EyeVectors( &forward, NULL, &up );

	trace_t tr;
	// Search for objects in a sphere (tests for entities that are not solid, yet still useable)
	Vector searchCenter = EyePosition();

	// NOTE: Some debris objects are useable too, so hit those as well
	// A button, etc. can be made out of clip brushes, make sure it's +useable via a traceline, too.
	int useableContents = MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_PLAYERCLIP;

#ifndef CLIENT_DLL
	CBaseEntity *pFoundByTrace = NULL;
#endif

	// UNDONE: Might be faster to just fold this range into the sphere query
	CBaseEntity *pObject = NULL;

	float nearestDist = FLT_MAX;
	// try the hit entity if there is one, or the ground entity if there isn't.
	CBaseEntity *pNearest = NULL;

#ifndef CLIENT_DLL
	{ // Quick inv. item scan. Prevent pickup bugs!
		trace_t trHighPrioItems;
		CTraceFilterWeaponsAndItems invItemFilter(this, COLLISION_GROUP_NONE, ITEM_PRIORITY_OBJECTIVE);
		UTIL_TraceHull(searchCenter, searchCenter + forward * PLAYER_USE_RADIUS, -Vector(2, 2, 2), Vector(2, 2, 2), MASK_SHOT_HULL, &invItemFilter, &trHighPrioItems);
		if (trHighPrioItems.m_pEnt && IsUseableEntity(trHighPrioItems.m_pEnt, 0) && (trHighPrioItems.m_pEnt->GetItemPrio() == ITEM_PRIORITY_OBJECTIVE))
			return trHighPrioItems.m_pEnt;
	}
#endif

	const int NUM_TANGENTS = 8;
	// trace a box at successive angles down
	//							forward, 45 deg, 30 deg, 20 deg, 15 deg, 10 deg, -10, -15
	const float tangents[NUM_TANGENTS] = { 0, 1, 0.57735026919f, 0.3639702342f, 0.267949192431f, 0.1763269807f, -0.1763269807f, -0.267949192431f };
	for ( int i = 0; i < NUM_TANGENTS; i++ )
	{
		if ( i == 0 )
		{
			UTIL_TraceLine(searchCenter, searchCenter + forward * 1024, useableContents, &trFiltr, &tr);
		}
		else
		{
			Vector down = forward - tangents[i]*up;
			VectorNormalize(down);
			UTIL_TraceHull(searchCenter, searchCenter + down * 72, -Vector(16, 16, 16), Vector(16, 16, 16), useableContents, &trFiltr, &tr);
		}
		pObject = tr.m_pEnt;

#ifndef CLIENT_DLL
		pFoundByTrace = pObject;
#endif
		bool bUsable = IsUseableEntity(pObject, 0);
		while ( pObject && !bUsable && pObject->GetMoveParent() )
		{
			pObject = pObject->GetMoveParent();
			bUsable = IsUseableEntity(pObject, 0);
		}

		if ( bUsable )
		{
			Vector delta = tr.endpos - tr.startpos;
			float centerZ = CollisionProp()->WorldSpaceCenter().z;
			delta.z = IntervalDistance( tr.endpos.z, centerZ + CollisionProp()->OBBMins().z, centerZ + CollisionProp()->OBBMaxs().z );
			float dist = delta.Length();
			if ( dist < PLAYER_USE_RADIUS )
			{
#ifndef CLIENT_DLL
				if ( sv_debug_player_use.GetBool() )
				{
					NDebugOverlay::Line( searchCenter, tr.endpos, 0, 255, 0, true, 30 );
					NDebugOverlay::Cross3D( tr.endpos, 16, 0, 255, 0, true, 30 );
				}
#endif
				if ( sv_debug_player_use.GetBool() )
				{
					Msg( "Trace using: %s\n", pObject ? pObject->GetDebugName() : "no usable entity found" );
				}

				pNearest = pObject;
				
				// if this is directly under the cursor just return it now
				if (i == 0)
					return pObject;
			}
		}
	}

	// check ground entity first
	// if you've got a useable ground entity, then shrink the cone of this search to 45 degrees
	// otherwise, search out in a 90 degree cone (hemisphere)
	if ( GetGroundEntity() && IsUseableEntity(GetGroundEntity(), FCAP_USE_ONGROUND) )
	{
		pNearest = GetGroundEntity();
	}
	if ( pNearest )
	{
		// estimate nearest object by distance from the view vector
		Vector point;
		pNearest->CollisionProp()->CalcNearestPoint( searchCenter, &point );
		nearestDist = CalcDistanceToLine( point, searchCenter, forward );
		if ( sv_debug_player_use.GetBool() )
		{
			Msg("Trace found %s, dist %.2f\n", pNearest->GetClassname(), nearestDist );
		}
	}

	for ( CEntitySphereQuery sphere( searchCenter, PLAYER_USE_RADIUS ); ( pObject = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if ( !pObject )
			continue;

		if ( !IsUseableEntity( pObject, FCAP_USE_IN_RADIUS ) )
			continue;

		// see if it's more roughly in front of the player than previous guess
		Vector point;
		pObject->CollisionProp()->CalcNearestPoint( searchCenter, &point );

		Vector dir = point - searchCenter;
		VectorNormalize(dir);
		float dot = DotProduct( dir, forward );

		// Need to be looking at the object more or less
		if ( dot < 0.8 )
			continue;

		float dist = CalcDistanceToLine( point, searchCenter, forward );

		if ( sv_debug_player_use.GetBool() )
		{
			Msg("Radius found %s, dist %.2f\n", pObject->GetClassname(), dist );
		}

		if ( dist < nearestDist )
		{
			// Since this has purely been a radius search to this point, we now
			// make sure the object isn't behind glass or a grate.
			trace_t trCheckOccluded;
			UTIL_TraceLine(searchCenter, point, useableContents, &trFiltr, &trCheckOccluded);

			if ( trCheckOccluded.fraction == 1.0 || trCheckOccluded.m_pEnt == pObject )
			{
				pNearest = pObject;
				nearestDist = dist;
			}
		}
	}

#ifndef CLIENT_DLL
	if ( !pNearest )
	{
		// Haven't found anything near the player to use, nor any NPC's at distance.
		// Check to see if the player is trying to select an NPC through a rail, fence, or other 'see-though' volume.
		trace_t trAllies;
		UTIL_TraceLine(searchCenter, searchCenter + forward * PLAYER_USE_RADIUS, MASK_OPAQUE_AND_NPCS, &trFiltr, &trAllies);
	}

	// Still nothing? Try a simplified weapon/item based trace.
	if (!pNearest)
	{
		trace_t trWeaponsAndItems;
		CTraceFilterWeaponsAndItems wepItemFilter(this, COLLISION_GROUP_NONE, ITEM_PRIORITY_NO);
		UTIL_TraceLine(searchCenter, searchCenter + forward * PLAYER_USE_RADIUS, MASK_SHOT, &wepItemFilter, &trWeaponsAndItems);
		if (trWeaponsAndItems.m_pEnt && IsUseableEntity(trWeaponsAndItems.m_pEnt, 0))
			pNearest = trWeaponsAndItems.m_pEnt;
	}

	if ( sv_debug_player_use.GetBool() )
	{
		if ( !pNearest )
		{
			NDebugOverlay::Line( searchCenter, tr.endpos, 255, 0, 0, true, 30 );
			NDebugOverlay::Cross3D( tr.endpos, 16, 255, 0, 0, true, 30 );
		}
		else if ( pNearest == pFoundByTrace )
		{
			NDebugOverlay::Line( searchCenter, tr.endpos, 0, 255, 0, true, 30 );
			NDebugOverlay::Cross3D( tr.endpos, 16, 0, 255, 0, true, 30 );
		}
		else
		{
			NDebugOverlay::Box( pNearest->WorldSpaceCenter(), Vector(-8, -8, -8), Vector(8, 8, 8), 0, 255, 0, true, 30 );
		}
	}
#endif

	if ( sv_debug_player_use.GetBool() )
	{
		Msg( "Radial using: %s\n", pNearest ? pNearest->GetDebugName() : "no usable entity found" );
	}

	return pNearest;
}

//-----------------------------------------------------------------------------
// Purpose: Handles USE keypress
//-----------------------------------------------------------------------------
void CBasePlayer::PlayerUse ( void )
{
#ifdef GAME_DLL
	// Was use pressed or released?
	if ( ! ((m_nButtons | m_afButtonPressed | m_afButtonReleased) & IN_USE) )
		return;

	if ( IsObserver() )
	{
		// do special use operation in oberserver mode
		if ( m_afButtonPressed & IN_USE )
			ObserverUse( true );
		else if ( m_afButtonReleased & IN_USE )
			ObserverUse( false );
		
		return;
	}

#if !defined(_XBOX)
	// push objects in turbo physics mode
	if ( (m_nButtons & IN_USE) && sv_turbophysics.GetBool() )
	{
		Vector forward, up;
		EyeVectors( &forward, NULL, &up );

		trace_t tr;
		// Search for objects in a sphere (tests for entities that are not solid, yet still useable)
		Vector searchCenter = EyePosition();

		CUsePushFilter filter;

		UTIL_TraceLine( searchCenter, searchCenter + forward * 96.0f, MASK_SOLID, &filter, &tr );

		// try the hit entity if there is one, or the ground entity if there isn't.
		CBaseEntity *entity = tr.m_pEnt;

		if (entity && (ToBaseCombatWeapon(entity) == NULL))
		{
			IPhysicsObject *pObj = entity->VPhysicsGetObject();
			if ( pObj )
			{
				Vector vPushAway = (entity->WorldSpaceCenter() - WorldSpaceCenter());
				vPushAway.z = 0;

				float flDist = VectorNormalize( vPushAway );
				flDist = MAX( flDist, 1 );

				float flForce = sv_pushaway_force.GetFloat() / flDist;
				flForce = MIN( flForce, sv_pushaway_max_force.GetFloat() );

				pObj->ApplyForceOffset( vPushAway * flForce, WorldSpaceCenter() );
			}
		}
	}
#endif

	if ( m_afButtonPressed & IN_USE )
	{
		// Controlling some latched entity?
		if ( ClearUseEntity() )
		{
			return;
		}
		else
		{
			if ( m_afPhysicsFlags & PFLAG_DIROVERRIDE )
			{
				m_afPhysicsFlags &= ~PFLAG_DIROVERRIDE;
				m_iTrain = TRAIN_NEW|TRAIN_OFF;
				return;
			}
			else
			{	// Start controlling the train!
				CBaseEntity *pTrain = GetGroundEntity();
				if ( pTrain && !(m_nButtons & IN_JUMP) && (GetFlags() & FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(this) )
				{
					m_afPhysicsFlags |= PFLAG_DIROVERRIDE;
					m_iTrain = TrainSpeed(pTrain->m_flSpeed, ((CFuncTrackTrain*)pTrain)->GetMaxSpeed());
					m_iTrain |= TRAIN_NEW;
					EmitSound( "Player.UseTrain" );
					return;
				}
			}
		}
	}

	CBaseEntity *pUseEntity = FindUseEntity();

	// Found an object
	if ( pUseEntity )
	{
		//!!!UNDONE: traceline here to prevent +USEing buttons through walls			

		int caps = pUseEntity->ObjectCaps();
		variant_t emptyVariant;
		if ( ( (m_nButtons & IN_USE) && (caps & FCAP_CONTINUOUS_USE) ) || ( (m_afButtonPressed & IN_USE) && (caps & (FCAP_IMPULSE_USE|FCAP_ONOFF_USE)) ) )
		{
			if ( caps & FCAP_CONTINUOUS_USE )
			{
				m_afPhysicsFlags |= PFLAG_USING;
			}

			if ( pUseEntity->ObjectCaps() & FCAP_ONOFF_USE )
			{
				pUseEntity->AcceptInput( "Use", this, this, emptyVariant, USE_ON );
			}
			else
			{
				pUseEntity->AcceptInput( "Use", this, this, emptyVariant, USE_TOGGLE );
			}
		}
		// UNDONE: Send different USE codes for ON/OFF.  Cache last ONOFF_USE object to send 'off' if you turn away
		else if ( (m_afButtonReleased & IN_USE) && (pUseEntity->ObjectCaps() & FCAP_ONOFF_USE) )	// BUGBUG This is an "off" use
		{
			pUseEntity->AcceptInput( "Use", this, this, emptyVariant, USE_OFF );
		}

		return;
	}

	CBaseCombatWeapon* pWeapon = GetActiveWeapon();
	if (pWeapon && pWeapon->Use())
		return; // INF -- what about client side?
#endif
}

ConVar	sv_suppress_viewpunch( "sv_suppress_viewpunch", "0", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::ViewPunch( const QAngle &angleOffset )
{
	//See if we're suppressing the view punching
	if ( sv_suppress_viewpunch.GetBool() )
		return;

	m_Local.m_vecPunchAngleVel += angleOffset * 20;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::ViewPunchReset( float tolerance )
{
	if ( tolerance != 0 )
	{
		tolerance *= tolerance;	// square
		float check = m_Local.m_vecPunchAngleVel->LengthSqr() + m_Local.m_vecPunchAngle->LengthSqr();
		if ( check > tolerance )
			return;
	}
	m_Local.m_vecPunchAngle = vec3_angle;
	m_Local.m_vecPunchAngleVel = vec3_angle;
}

const QAngle& CBasePlayer::GetPunchAngle()
{
	return m_Local.m_vecPunchAngle.Get();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::RecoilViewPunch(const QAngle& angleOffset)
{
	m_Local.m_vecRecoilPunchAngleVel += angleOffset * 20.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::RecoilViewPunchReset(float tolerance)
{
	if (tolerance != 0)
	{
		tolerance *= tolerance;	// square
		float check = m_Local.m_vecRecoilPunchAngleVel->LengthSqr() + m_Local.m_vecRecoilPunchAngle->LengthSqr();
		if (check > tolerance)
			return;
	}
	m_Local.m_vecRecoilPunchAngle = vec3_angle;
	m_Local.m_vecRecoilPunchAngleVel = vec3_angle;
}

const QAngle& CBasePlayer::GetRecoilPunchAngle(void)
{
	return m_Local.m_vecRecoilPunchAngle.Get();
}

#if defined( CLIENT_DLL )

#include "iviewrender.h"
#include "ivieweffects.h"

#endif

static ConVar smoothstairs( "smoothstairs", "1", FCVAR_REPLICATED, "Smooth player eye z coordinate when traversing stairs." );

//-----------------------------------------------------------------------------
// Handle view smoothing when going up or down stairs
//-----------------------------------------------------------------------------
void CBasePlayer::SmoothViewOnStairs( Vector& eyeOrigin )
{
	CBaseEntity *pGroundEntity = GetGroundEntity();
	float flCurrentPlayerZ = GetLocalOrigin().z;
	float flCurrentPlayerViewOffsetZ = GetViewOffset().z;

	// Smooth out stair step ups
	// NOTE: Don't want to do this when the ground entity is moving the player
	if ( ( pGroundEntity != NULL && pGroundEntity->GetMoveType() == MOVETYPE_NONE ) && ( flCurrentPlayerZ != m_flOldPlayerZ ) && smoothstairs.GetBool() &&
		 m_flOldPlayerViewOffsetZ == flCurrentPlayerViewOffsetZ )
	{
		int dir = ( flCurrentPlayerZ > m_flOldPlayerZ ) ? 1 : -1;

		float steptime = gpGlobals->frametime;
		if (steptime < 0)
		{
			steptime = 0;
		}

		m_flOldPlayerZ += steptime * 150 * dir;

		const float stepSize = 18.0f;

		if ( dir > 0 )
		{
			if (m_flOldPlayerZ > flCurrentPlayerZ)
			{
				m_flOldPlayerZ = flCurrentPlayerZ;
			}
			if (flCurrentPlayerZ - m_flOldPlayerZ > stepSize)
			{
				m_flOldPlayerZ = flCurrentPlayerZ - stepSize;
			}
		}
		else
		{
			if (m_flOldPlayerZ < flCurrentPlayerZ)
			{
				m_flOldPlayerZ = flCurrentPlayerZ;
			}
			if (flCurrentPlayerZ - m_flOldPlayerZ < -stepSize)
			{
				m_flOldPlayerZ = flCurrentPlayerZ + stepSize;
			}
		}

		eyeOrigin[2] += m_flOldPlayerZ - flCurrentPlayerZ;
	}
	else
	{
		m_flOldPlayerZ = flCurrentPlayerZ;
		m_flOldPlayerViewOffsetZ = flCurrentPlayerViewOffsetZ;
	}
}

static bool IsWaterContents( int contents )
{
	if ( contents & MASK_WATER )
		return true;

//	if ( contents & CONTENTS_TESTFOGVOLUME )
//		return true;

	return false;
}

void CBasePlayer::ResetObserverMode()
{
	m_hObserverTarget.Set( 0 );
	m_iObserverMode = (int)OBS_MODE_NONE;

#ifndef CLIENT_DLL
	m_iObserverLastMode = OBS_MODE_ROAMING;
	m_bForcedObserverMode = false;
	m_afPhysicsFlags &= ~PFLAG_OBSERVER;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : eyeOrigin - 
//			eyeAngles - 
//			zNear - 
//			zFar - 
//			fov - 
//-----------------------------------------------------------------------------
void CBasePlayer::CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov )
{
	if (IsObserver())
	{
		CalcObserverView(eyeOrigin, eyeAngles, fov);
	}
	else
	{
		CalcPlayerView(eyeOrigin, eyeAngles, fov);
	}
}

void CBasePlayer::CalcViewModelView( const Vector& eyeOrigin, const QAngle& eyeAngles)
{
	CBaseViewModel *vm = GetViewModel();
	if (vm)
		vm->CalcViewModelView(this, eyeOrigin, eyeAngles + m_Local.m_vecPunchAngle);
}

void CBasePlayer::CalcPlayerView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
#if defined( CLIENT_DLL )
	if ( !prediction->InPrediction() )
	{
		// FIXME: Move into prediction
		view->DriftPitch();
	}
#endif

	VectorCopy( EyePosition(), eyeOrigin );
	VectorCopy( EyeAngles(), eyeAngles );

#if defined( CLIENT_DLL )
	if ( !prediction->InPrediction() )
#endif
	{
		SmoothViewOnStairs( eyeOrigin );
	}

	// Snack off the origin before bob + water offset are applied
	Vector vecBaseEyePosition = eyeOrigin;

	CalcViewRoll(eyeAngles);
	ApplyPlayerView(eyeOrigin, eyeAngles, fov);

	// Apply punch angle
	//VectorAdd( eyeAngles, m_Local.m_vecPunchAngle, eyeAngles );
	VectorAdd(eyeAngles, m_Local.m_vecRecoilPunchAngle, eyeAngles);

#if defined( CLIENT_DLL )
	if ( !prediction->InPrediction() )
	{
		// Shake it up baby!
		vieweffects->CalcShake();
		vieweffects->ApplyShake( eyeOrigin, eyeAngles, 1.0 );
	}
#endif

#if defined( CLIENT_DLL )
	// Apply a smoothing offset to smooth out prediction errors.
	Vector vSmoothOffset;
	GetPredictionErrorSmoothingVector( vSmoothOffset );
	eyeOrigin += vSmoothOffset;
	m_flObserverChaseDistance = 0.0;
#endif

	// calc current FOV
	fov = GetFOV();
}

void CBasePlayer::CalcObserverView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
#if defined( CLIENT_DLL )
	switch ( GetObserverMode() )
	{

		case OBS_MODE_DEATHCAM	:	CalcDeathCamView( eyeOrigin, eyeAngles, fov );
									break;

		case OBS_MODE_ROAMING	:	// just copy current position without view offset
		case OBS_MODE_FIXED		:	CalcRoamingView( eyeOrigin, eyeAngles, fov );
									break;

		case OBS_MODE_IN_EYE	:	CalcInEyeCamView( eyeOrigin, eyeAngles, fov );
									break;

		case OBS_MODE_CHASE		:	CalcChaseCamView( eyeOrigin, eyeAngles, fov  );
									break;

		case OBS_MODE_FREEZECAM	:	CalcFreezeCamView( eyeOrigin, eyeAngles, fov  );
									break;
	}
#else
	// on server just copy target postions, final view positions will be calculated on client
	VectorCopy( EyePosition(), eyeOrigin );
	VectorCopy( EyeAngles(), eyeAngles );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Determine view roll, including data kick
//-----------------------------------------------------------------------------
void CBasePlayer::CalcViewRoll( QAngle& eyeAngles )
{
}

void CBasePlayer::DoMuzzleFlash()
{
	CBaseViewModel* vm = GetViewModel();
	if (vm)
		vm->DoMuzzleFlash();
}

float CBasePlayer::GetFOVDistanceAdjustFactor()
{
	float defaultFOV	= (float)GetDefaultFOV();
	float localFOV		= (float)GetFOV();

	if ( localFOV == defaultFOV || defaultFOV < 0.001f )
	{
		return 1.0f;
	}

	// If FOV is lower, then we're "zoomed" in and this will give a factor < 1 so apparent LOD distances can be
	//  shorted accordingly
	return localFOV / defaultFOV;

}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecTracerSrc - 
//			&tr - 
//			iTracerType - 
//-----------------------------------------------------------------------------
void CBasePlayer::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	if ( GetActiveWeapon() )
	{
		GetActiveWeapon()->MakeTracer( vecTracerSrc, tr, iTracerType );
		return;
	}

	BaseClass::MakeTracer( vecTracerSrc, tr, iTracerType );
}

void CBasePlayer::SetNewSolidFlags( bool bNonSolid )
{
	if ( bNonSolid )
	{
		if ( IsSolid() )
		{
			RemoveSolidFlags(FSOLID_NOT_STANDABLE);
			SetSolid(SOLID_NONE);
			AddSolidFlags(FSOLID_NOT_SOLID);
			AddSolidFlags(FSOLID_TRIGGER);
		}
	}
	else
	{
		if ( !IsSolid() )
		{
			RemoveSolidFlags(FSOLID_NOT_SOLID);
			RemoveSolidFlags(FSOLID_TRIGGER);
			SetSolid(SOLID_BBOX);
			AddSolidFlags(FSOLID_NOT_STANDABLE);
		}
	}
}

void CBasePlayer::SharedSpawn()
{
	SetMoveType( MOVETYPE_WALK );
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetFriction( 1.0f );

	pl.deadflag	= false;
	m_lifeState	= LIFE_ALIVE;
	m_iHealth = 100;
	m_takedamage		= DAMAGE_YES;

	m_Local.m_flStepSize = sv_stepsize.GetFloat();
	m_Local.m_bAllowAutoMovement = true;
	m_Local.m_bIsInOtherView = false;

	m_nRenderFX = kRenderFxNone;
	m_flNextAttack	= gpGlobals->curtime;
	m_flMaxspeed		= 0.0f;

	// dont let uninitialized value here hurt the player
	m_Local.m_flFallVelocity = 0;

	SetBloodColor( BLOOD_COLOR_RED );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CBasePlayer::GetDefaultFOV( void ) const
{
#if defined( CLIENT_DLL )
	if ( GetObserverMode() == OBS_MODE_IN_EYE )
	{
		C_BasePlayer *pTargetPlayer = ToBasePlayer( GetObserverTarget() );
		if ( pTargetPlayer && !pTargetPlayer->IsObserver() )
		{
			return pTargetPlayer->GetDefaultFOV();
		}
	}
#endif

	return (m_Local.m_iDefaultFOV == 0 ? g_pGameRules->DefaultFOV() : m_Local.m_iDefaultFOV);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CBasePlayer::GetDefaultViewmodelFOV(void) const
{
	return g_pGameRules->DefaultWeaponFOV();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBasePlayer::GetScopeFOV(void) const
{
	return m_Local.m_iScopeFOV;
}

void CBasePlayer::AvoidPhysicsProps( CUserCmd *pCmd )
{
#ifndef _XBOX
	// Don't avoid if noclipping or in movetype none
	switch ( GetMoveType() )
	{
	case MOVETYPE_NOCLIP:
	case MOVETYPE_NONE:
	case MOVETYPE_OBSERVER:
		return;
	default:
		break;
	}

	if ( GetObserverMode() != OBS_MODE_NONE || !IsAlive() )
		return;

	AvoidPushawayProps( this, pCmd );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const char
//-----------------------------------------------------------------------------
const char *CBasePlayer::GetTracerType( void )
{
	if ( GetActiveWeapon() )
	{
		return GetActiveWeapon()->GetTracerType();
	}

	return BaseClass::GetTracerType();
}

//-----------------------------------------------------------------------------
// Purpose: Sets the FOV of the client, doing interpolation between old and new if requested
// Input  : FOV - New FOV
//			zoomRate - Amount of time (in seconds) to move between old and new FOV
//-----------------------------------------------------------------------------
bool CBasePlayer::SetFOV(int iFOV, float zoomRate)
{
	m_iFOV = iFOV;
	m_Local.m_flFOVRate = zoomRate;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::UpdateUnderwaterState( void )
{
	if ( GetWaterLevel() == WL_Eyes )
	{
		if ( IsPlayerUnderwater() == false )
		{
			SetPlayerUnderwater( true );
		}
		return;
	}

	if ( IsPlayerUnderwater() )
	{
		SetPlayerUnderwater( false );
	}

	if ( GetWaterLevel() == 0 )
	{
		if ( GetFlags() & FL_INWATER )
		{
#ifndef CLIENT_DLL
			if ( m_iHealth > 0 && IsAlive() )
			{
				EmitSound( "Player.Wade" );
			}
#endif
			RemoveFlag( FL_INWATER );
		}
	}
	else if ( !(GetFlags() & FL_INWATER) )
	{
#ifndef CLIENT_DLL
		// player enter water sound
		if (GetWaterType() == CONTENTS_WATER)
		{
			EmitSound( "Player.Wade" );
		}
#endif

		AddFlag( FL_INWATER );
	}
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
// ensure that for every emitsound there is a matching stopsound
//-----------------------------------------------------------------------------
void CBasePlayer::SetPlayerUnderwater( bool state )
{
	if ( m_bPlayerUnderwater != state )
	{
		m_bPlayerUnderwater = state;
#ifdef CLIENT_DLL
		if ( state )
			EmitSound( "Player.AmbientUnderWater" );
		else
			StopSound( "Player.AmbientUnderWater" );		
#endif
	}
}

void CBasePlayer::SetPreviouslyPredictedOrigin( const Vector &vecAbsOrigin )
{
	m_vecPreviouslyPredictedOrigin = vecAbsOrigin;
}

const Vector &CBasePlayer::GetPreviouslyPredictedOrigin() const
{
	return m_vecPreviouslyPredictedOrigin;
}

bool fogparams_t::operator != ( const fogparams_t& other ) const
{
	if ( this->enable != other.enable ||
		this->blend != other.blend ||
		!VectorsAreEqual(this->dirPrimary, other.dirPrimary, 0.01f ) || 
		this->colorPrimary.Get() != other.colorPrimary.Get() ||
		this->colorSecondary.Get() != other.colorSecondary.Get() ||
		this->start != other.start ||
		this->end != other.end ||
		this->farz != other.farz ||
		this->maxdensity != other.maxdensity ||
		this->colorPrimaryLerpTo.Get() != other.colorPrimaryLerpTo.Get() ||
		this->colorSecondaryLerpTo.Get() != other.colorSecondaryLerpTo.Get() ||
		this->startLerpTo != other.startLerpTo ||
		this->endLerpTo != other.endLerpTo ||
		this->lerptime != other.lerptime ||
		this->duration != other.duration )
		return true;

	return false;
}
