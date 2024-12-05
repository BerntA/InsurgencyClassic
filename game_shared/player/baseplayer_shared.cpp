//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
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

	#include "iclientvehicle.h"
	#include "prediction.h"
	#include "c_basedoor.h"
	#include "c_world.h"
	#include "view.h"
	#include "cl_dll/iviewport.h"

	#define CRecipientFilter C_RecipientFilter

#else

	#include "iservervehicle.h"
	#include "trains.h"
	#include "world.h"
	#include "doors.h"
	//#include "ai_basenpc.h"
	
#endif

#include "in_buttons.h"
#include "engine/ienginesound.h"
#include "tier0/vprof.h"
#include "soundemittersystem/isoundemittersystembase.h"
#include "decals.h"
#include "obstacle_pushaway.h"
#include "gamemovement.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef GAME_DLL
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
	{
		return;
	}

	if (!GetActiveWeapon())
		return;

#if defined( CLIENT_DLL )
	// Not predicting this weapon
	if ( !GetActiveWeapon()->IsPredicted() )
		return;
#endif

	GetActiveWeapon()->ItemPreFrame();

	// Pongles [
	/*CBaseCombatWeapon *pWeapon;

	// Allow all the holstered weapons to update
	for ( int i = 0; i < WeaponCount(); i++ )
	{
		pWeapon = GetWeapon( i );

		if ( pWeapon == NULL )
			continue;

		if ( GetActiveWeapon() == pWeapon )
			continue;

		pWeapon->ItemHolsterFrame();
	}*/
	// Pongles ]
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBasePlayer::UsingStandardWeaponsInVehicle( void )
{
	Assert( IsInAVehicle() );
#if !defined( CLIENT_DLL )
	IServerVehicle *pVehicle = GetVehicle();
#else
	IClientVehicle *pVehicle = GetVehicle();
#endif
	Assert( pVehicle );
	if ( !pVehicle )
		return true;

	// NOTE: We *have* to do this before ItemPostFrame because ItemPostFrame
	// may dump us out of the vehicle
	int nRole = pVehicle->GetPassengerRole( this );
	bool bUsingStandardWeapons = pVehicle->IsPassengerUsingStandardWeapons( nRole );

	// Fall through and check weapons, etc. if we're using them 
	if (!bUsingStandardWeapons )
		return false;

	return true;
}

// Pongles [

//-----------------------------------------------------------------------------
// Purpose: Called every usercmd by the player PostThink
//-----------------------------------------------------------------------------
void CBasePlayer::ItemPostFrame( void )
{
	VPROF( "CBasePlayer::ItemPostFrame" );

	// put viewmodels into basically correct place based on new player origin
	CalcViewModelView( EyePosition( ), EyeAngles( ) );

	// switch weapon when needed
	if( m_flNextActiveWeapon != 0.0f && gpGlobals->curtime >= m_flNextActiveWeapon )
	{
		CBaseCombatWeapon *pNextWeapon = m_hNextActiveWeapon;

		if( pNextWeapon )
		{
			Weapon_SwitchToNext( );
		}
		else
		{
			m_flNextActiveWeapon = 0.0f;

			CBaseViewModel *pViewModel = GetViewModel( );

			if( pViewModel )
				pViewModel->RemoveEffects( EF_NODRAW );
		}
	}

	// find active weapon and call
	CBaseCombatWeapon *pWeapon = GetActiveWeapon( );

	if( pWeapon )
	{
		if( gpGlobals->curtime >= m_flNextAttack )
			pWeapon->ItemPostFrame( );
		else
			pWeapon->ItemBusyFrame( );
	}
}

// Pongles ]

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
	IClientVehicle *pVehicle = GetVehicle();
#else
	IServerVehicle *pVehicle = GetVehicle();
#endif
	if ( pVehicle )
	{
		Assert( pVehicle );
		int nRole = pVehicle->GetPassengerRole( this );

		Vector vecEyeOrigin;
		QAngle angEyeAngles;
		pVehicle->GetVehicleViewPosition( nRole, &vecEyeOrigin, &angEyeAngles );
		return vecEyeOrigin;
	}
	else
	{
#ifdef CLIENT_DLL
		if ( IsObserver() )
		{
			if ( m_iObserverMode == OBS_MODE_CHASE )
			{
				if ( IsLocalPlayer() )
				{
					return MainViewOrigin();
				}
			}
		}
#endif
		return BaseClass::EyePosition();
	}
}

// Pongles [

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector CBasePlayer::GetPlayerMins( void ) const
{
	return VEC_OBS_HULL_MIN;	
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector CBasePlayer::GetPlayerMaxs( void ) const
{	
	return VEC_OBS_HULL_MAX;	
}

// Pongles ]

//-----------------------------------------------------------------------------
// Returns eye vectors
//-----------------------------------------------------------------------------
void CBasePlayer::EyeVectors( Vector *pForward, Vector *pRight, Vector *pUp )
{
#ifdef CLIENT_DLL
	IClientVehicle *pVehicle = GetVehicle();
#else
	IServerVehicle *pVehicle = GetVehicle();
#endif
	if ( pVehicle )
	{
		Assert( pVehicle );
		int nRole = pVehicle->GetPassengerRole( this );

		Vector vecEyeOrigin;
		QAngle angEyeAngles;
		pVehicle->GetVehicleViewPosition( nRole, &vecEyeOrigin, &angEyeAngles );
		AngleVectors( angEyeAngles, pForward, pRight, pUp );
	}
	else
	{
		AngleVectors( EyeAngles(), pForward, pRight, pUp );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns the eye position and angle vectors.
//-----------------------------------------------------------------------------
void CBasePlayer::EyePositionAndVectors( Vector *pPosition, Vector *pForward,
										 Vector *pRight, Vector *pUp )
{
#ifdef CLIENT_DLL
	IClientVehicle *pVehicle = GetVehicle();
#else
	IServerVehicle *pVehicle = GetVehicle();
#endif

	if ( pVehicle )
	{
		Assert( pVehicle );

		int nRole = pVehicle->GetPassengerRole( this );

		Vector vecEyeOrigin;
		QAngle angEyeAngles;
		pVehicle->GetVehicleViewPosition( nRole, pPosition, &angEyeAngles );
		AngleVectors( angEyeAngles, pForward, pRight, pUp );
	}
	else
	{
		VectorCopy( BaseClass::EyePosition(), *pPosition );
		AngleVectors( EyeAngles(), pForward, pRight, pUp );
	}
}

#ifdef CLIENT_DLL
surfacedata_t * CBasePlayer::GetFootstepSurface( const Vector &origin, const char *surfaceName )
{
	return physprops->GetSurfaceData( physprops->GetSurfaceIndex( surfaceName ) );
}
#endif

// Pongles [

void CBasePlayer::UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity )
{
	// PNOTE: all moved into INS code
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : step - 
//			fvol - 
//			force - force sound to play
//-----------------------------------------------------------------------------
void CBasePlayer::PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force )
{
	// Pongles [

	/*if ( gpGlobals->maxClients > 1 && !sv_footsteps.GetFloat() )
		return;*/

	// Pongles ]

#if defined( CLIENT_DLL )
	// during prediction play footstep sounds only once
	if ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		return;
#endif

	if ( !psurface )
		return;

	unsigned short stepSoundName = m_Local.m_nStepside ? psurface->sounds.stepleft : psurface->sounds.stepright;
	m_Local.m_nStepside = !m_Local.m_nStepside;

	if ( !stepSoundName )
		return;

	IPhysicsSurfaceProps *physprops = MoveHelper( )->GetSurfaceProps();
	const char *pSoundName = physprops->GetString( stepSoundName );
	CSoundParameters params;
	if ( !CBaseEntity::GetParametersForSound( pSoundName, params, NULL ) )
		return;

	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

#ifndef CLIENT_DLL
	// in MP, server removed all players in origins PVS, these players 
	// generate the footsteps clientside
	if ( gpGlobals->maxClients > 1 )
		filter.RemoveRecipientsByPVS( vecOrigin );
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
}

void CBasePlayer::UpdateButtonState( int nUserCmdButtonMask )
{
	// Pongles [

	// PNOTE: zero covered this in Panel.cpp, but I'm not
	// sure if Ir reimplimented it so I've commented this
	// back in for the time being
#ifdef CLIENT_DLL

	if(gViewPortInterface->GetActivePanel() && gViewPortInterface->GetActivePanel()->HasInputElements())
		nUserCmdButtonMask = 0;

#endif

	// Pongles ]

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

int CBasePlayer::BloodColor()
{
	return BLOOD_COLOR_RED;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CBasePlayer::WeaponCount() const
{
	return MAX_PWEAPONS;
}

// Pongles [

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBaseCombatWeapon
//-----------------------------------------------------------------------------
CBaseCombatWeapon *CBasePlayer::GetActiveWeapon() const
{
	return m_hActiveWeapon;
}

// Pongles ]

CBaseCombatWeapon*	CBasePlayer::GetWeapon( int i ) const
{
	Assert( (i >= 0) && (i < MAX_PWEAPONS) );
	return m_hMyWeapons[i].Get();
}

// Pongles [
Vector CBasePlayer::Weapon_ShootPosition( )
{
	return EyePosition();
}

// Pongles [
Vector CBasePlayer::Weapon_ShootDirection( )
{
	Vector	forward;
#ifdef CLIENT_DLL
	AngleVectors( GetAbsAngles() + m_Local.m_vecRecoilPunchAngle, &forward );
#else
	AngleVectors( EyeAngles() + m_Local.m_vecRecoilPunchAngle, &forward );
#endif

	return	forward;
}
// Pongles ]

void CBasePlayer::SetAnimationExtension( const char *pExtension )
{
	Q_strncpy( m_szAnimExtension, pExtension, sizeof(m_szAnimExtension) );
}


//-----------------------------------------------------------------------------
// Purpose: Set the weapon to switch to when the player uses the 'lastinv' command
//-----------------------------------------------------------------------------
void CBasePlayer::Weapon_SetLast( CBaseCombatWeapon *pWeapon )
{
	m_hLastWeapon = pWeapon;
}

// Pongles [

//-----------------------------------------------------------------------------
// Purpose: Override base class so player can reset autoaim
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CBasePlayer::Weapon_Switch( CBaseCombatWeapon *pWeapon, bool bForce ) 
{
	MDLCACHE_CRITICAL_SECTION( );

	CBaseCombatWeapon *pLastWeapon = GetActiveWeapon( );

	// quit out if invalid
	if( pWeapon == NULL )
		return false;

	// ensure we are able to switch
	if( !Weapon_CanSwitchTo( pWeapon ) )
		return false;

	// already have it out?
	if( pLastWeapon == pWeapon )
	{
		// deploy again if invisible
		if( !pLastWeapon->IsWeaponVisible( ) )
			pLastWeapon->Deploy( );

		return false;
	}

	// find the deploy threshold of the lastweapon
	m_flNextActiveWeapon = 0.0f;

	if( pLastWeapon && pLastWeapon->GetOwner( ) )
	{
		if( !pLastWeapon->Holster( pWeapon/*, bForce*/ ) )
			return false;

		if( !bForce )
		{
			m_flNextActiveWeapon = pLastWeapon->GetHolsterTime( );

			if( m_flNextActiveWeapon == 0.0f )
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
	if( bForce )
		Weapon_SwitchToNext( );

	// set the last weapon
	if( pLastWeapon )
		Weapon_SetLast( pLastWeapon );

	// don't hide the viewmodel anymore
	CBaseViewModel *pViewModel = GetViewModel( );
	Assert( pViewModel );

	if( pViewModel )
		pViewModel->RemoveEffects( EF_NODRAW );

	return true;
}

void CBasePlayer::Weapon_SwitchToNext( void )
{
	CBaseCombatWeapon *pNewWeapon = m_hNextActiveWeapon;

	if( !pNewWeapon )
		return;

	// set as active weapon and deploy
	m_hActiveWeapon = pNewWeapon;
	pNewWeapon->Deploy( );

	// reset
	m_flNextActiveWeapon = 0.0f;
	m_hNextActiveWeapon = NULL;

	// reset blends
#ifdef CLIENT_DLL

	CBasePlayer *pPlayer = dynamic_cast< CBasePlayer* >( this );

	if( !pPlayer )
		return;

	CBaseViewModel *pVM = pPlayer->GetViewModel( );

	if( pVM )
		pVM->m_SequenceTransitioner.m_animationQueue.RemoveAll( );

#endif
}

//-----------------------------------------------------------------------------
// Purpose: Returns weapon if already owns a weapon of this class
//-----------------------------------------------------------------------------
CBaseCombatWeapon* CBasePlayer::Weapon_Owns( int iWeaponID ) const
{
	// check for duplicates
	for(int i = 0; i < MAX_PWEAPONS; i++) 
	{
		CBaseCombatWeapon *pWeapon = m_hMyWeapons[i];

		if(pWeapon && pWeapon->GetWeaponID() == iWeaponID )
		{
			return m_hMyWeapons[i];
		}
	}

	return NULL;
}

bool CBasePlayer::Weapon_CanSwitchTo( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatWeapon *pActiveWeapon = m_hActiveWeapon;

	if( pActiveWeapon && !pActiveWeapon->CanHolster( ) )
		return false;

	if ( !pWeapon->CanDeploy( ) )
		return false;
	
	return true;
}

// Pongles ]

void CBasePlayer::SelectLastItem(void)
{
	if ( m_hLastWeapon.Get() == NULL )
		return;

	if ( GetActiveWeapon() && !GetActiveWeapon()->CanHolster() )
		return;

	SelectItem( m_hLastWeapon.Get()->GetWeaponID() );
}

//-----------------------------------------------------------------------------
// Purpose: Switches to the best weapon that is also better than the given weapon.
// Input  : pCurrent - The current weapon used by the player.
// Output : Returns true if the weapon was switched, false if there was no better
//			weapon to switch to.
//-----------------------------------------------------------------------------
bool CBasePlayer::SwitchToNextBestWeapon(CBaseCombatWeapon *pCurrent)
{
#ifdef GAME_DLL

	if( pCurrent && pCurrent->IsExhaustible( ) && !pCurrent->HasAmmo( ) )
	{
		RemoveWeapon( pCurrent );
		pCurrent = NULL;
	}

#endif

	CBaseCombatWeapon *pNewWeapon = GetNextBestWeapon(pCurrent);

	// Pongles [
	// - don't switch when dead
	if ( IsAlive() && ( pNewWeapon != NULL ) && ( pNewWeapon != pCurrent ) )
	{
		return Weapon_Switch( pNewWeapon );
	}
	// Pongles ]

	return false;
}

// Pongles [

CBaseCombatWeapon *CBasePlayer::GetNextBestWeapon( CBaseCombatWeapon *pCurrentWeapon )
{
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Abort any reloads we're in
//-----------------------------------------------------------------------------
void CBasePlayer::AbortReload( void )
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon( );

	if( pWeapon )
		pWeapon->AbortReload( );
}

// Pongles ]

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
	return ( pWeapon != GetActiveWeapon() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::SelectItem( int iWeaponID )
{
	CBaseCombatWeapon *pItem = Weapon_Owns( iWeaponID );

	if (!pItem)
		return;

	if ( !Weapon_ShouldSelectItem( pItem ) )
		return;

	// FIX, this needs to queue them up and delay
	// Make sure the current weapon can be holstered
	if ( GetActiveWeapon() )
	{
		if ( !GetActiveWeapon()->CanHolster() )
			return;

		// Pongles [
		//ResetAutoaim( );
		// Pongles ]
	}

	Weapon_Switch( pItem );
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

CBaseEntity *CBasePlayer::FindUseEntity()
{
	Vector forward, up;
	EyeVectors( &forward, NULL, &up );

	trace_t tr;
	// Search for objects in a sphere (tests for entities that are not solid, yet still useable)
	Vector searchCenter = EyePosition();

	// NOTE: Some debris objects are useable too, so hit those as well
	// A button, etc. can be made out of clip brushes, make sure it's +useable via a traceline, too.
	int useableContents = MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_PLAYERCLIP;
	UTIL_TraceLine( searchCenter, searchCenter + forward * 1024, useableContents, this, COLLISION_GROUP_NONE, &tr );

	// try the hit entity if there is one, or the ground entity if there isn't.
	CBaseEntity *pNearest = NULL;
	CBaseEntity *pObject = tr.m_pEnt;
	int count = 0;

	// UNDONE: Might be faster to just fold this range into the sphere query
	const int NUM_TANGENTS = 7;
	while ( !IsUseableEntity(pObject, 0) && count < NUM_TANGENTS)
	{
		// trace a box at successive angles down
		//							45 deg, 30 deg, 20 deg, 15 deg, 10 deg, -10, -15
		const float tangents[NUM_TANGENTS] = { 1, 0.57735026919f, 0.3639702342f, 0.267949192431f, 0.1763269807f, -0.1763269807f, -0.267949192431f };
		Vector down = forward - tangents[count]*up;
		VectorNormalize(down);
		UTIL_TraceHull( searchCenter, searchCenter + down * 72, -Vector(16,16,16), Vector(16,16,16), useableContents, this, COLLISION_GROUP_NONE, &tr );
		pObject = tr.m_pEnt;
		count++;
	}
	float nearestDot = CONE_90_DEGREES;
	if ( IsUseableEntity(pObject, 0) )
	{
		Vector delta = tr.endpos - tr.startpos;
		float centerZ = CollisionProp()->WorldSpaceCenter().z;
		delta.z = IntervalDistance( tr.endpos.z, centerZ + CollisionProp()->OBBMins().z, centerZ + CollisionProp()->OBBMaxs().z );
		float dist = delta.Length();
		if ( dist < PLAYER_USE_RADIUS )
		{
		#ifdef GAME_DLL
			if ( sv_debug_player_use.GetBool() )
			{
				NDebugOverlay::Line( searchCenter, tr.endpos, 0, 255, 0, true, 30 );
				NDebugOverlay::Cross3D( tr.endpos, 16, 0, 255, 0, true, 30 );
			}
		#endif

			return pObject;
		}
	}

#ifdef GAME_DLL
	CBaseEntity *pFoundByTrace = pObject;
#endif

	// check ground entity first
	// if you've got a useable ground entity, then shrink the cone of this search to 45 degrees
	// otherwise, search out in a 90 degree cone (hemisphere)
	if ( GetGroundEntity() && IsUseableEntity(GetGroundEntity(), FCAP_USE_ONGROUND) )
	{
		pNearest = GetGroundEntity();
		nearestDot = CONE_45_DEGREES;
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

		if ( dot > nearestDot )
		{
			// Since this has purely been a radius search to this point, we now
			// make sure the object isn't behind glass or a grate.
			trace_t trCheckOccluded;
			UTIL_TraceLine( searchCenter, point, useableContents, this, COLLISION_GROUP_NONE, &trCheckOccluded );

			if ( trCheckOccluded.fraction == 1.0 || trCheckOccluded.m_pEnt == pObject )
			{
				pNearest = pObject;
				nearestDot = dot;
			}
		}
	}

#ifdef GAME_DLL
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

	return pNearest;
}

// Pongles [

//-----------------------------------------------------------------------------
// Purpose: Return true if this object can be +used by the player
//-----------------------------------------------------------------------------
bool CBasePlayer::IsUseableEntity( CBaseEntity *pEntity, unsigned int requiredCaps )
{
	if ( pEntity )
	{
		int caps = pEntity->ObjectCaps();
		if ( caps & (FCAP_IMPULSE_USE|FCAP_CONTINUOUS_USE|FCAP_ONOFF_USE|FCAP_DIRECTIONAL_USE) )
		{
			if ( (caps & requiredCaps) == requiredCaps )
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Handles USE keypress
//-----------------------------------------------------------------------------
void CBasePlayer::PlayerUse ( void )
{
	// was use pressed or released?
	if( !( ( m_nButtons | m_afButtonPressed | m_afButtonReleased ) & IN_USE ) )
		return;

#ifdef GAME_DLL

	if( IsObserver( ) )
	{
		// do special use operation in observer mode
		if( m_afButtonPressed & IN_USE )
			ObserverUse( true );
		else if ( m_afButtonReleased & IN_USE )
			ObserverUse( false );
		
		return;
	}

	// push objects in turbo physics mode
	if ( ( m_nButtons & IN_USE ) && sv_turbophysics.GetBool( ) )
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

		// Pongles [

		if ( entity && ( dynamic_cast< CBaseCombatWeapon* >( entity ) == NULL ) )
		{
			IPhysicsObject *pObj = entity->VPhysicsGetObject();

			Vector vPushAway = (entity->WorldSpaceCenter() - WorldSpaceCenter());
			vPushAway.z = 0;

			float flDist = VectorNormalize( vPushAway );
			flDist = max( flDist, 1 );

			float flForce = sv_pushaway_force.GetFloat() / flDist;
			flForce = min( flForce, sv_pushaway_max_force.GetFloat() );

			pObj->ApplyForceOffset( vPushAway * flForce, WorldSpaceCenter() );
		}

		// Pongles ]
	}

#endif

	// find an an object
	CBaseEntity *pUseEntity = FindUseEntity( );

	if( pUseEntity )
	{
	#ifdef GAME_DLL

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

	#endif

		return;
	}

	// pass to active weapon
	CBaseCombatWeapon *pWeapon = GetActiveWeapon( );

	if( pWeapon && pWeapon->Use( ) )
		return;
}

// Pongles [

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::ViewPunch( const QAngle &angleOffset )
{
	// We don't allow view kicks in the vehicle
	if ( IsInAVehicle( ) )
		return;

	m_Local.m_vecPunchAngleVel += angleOffset * 20.0f;
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


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::RecoilViewPunch( const QAngle &angleOffset )
{
	// We don't allow view kicks in the vehicle
	if ( IsInAVehicle() )
		return;

	m_Local.m_vecRecoilPunchAngleVel += angleOffset * 20.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePlayer::RecoilViewPunchReset( float tolerance )
{
	if ( tolerance != 0 )
	{
		tolerance *= tolerance;	// square
		float check = m_Local.m_vecRecoilPunchAngleVel->LengthSqr() + m_Local.m_vecRecoilPunchAngle->LengthSqr();
		if ( check > tolerance )
			return;
	}
	m_Local.m_vecRecoilPunchAngle = vec3_angle;
	m_Local.m_vecRecoilPunchAngleVel = vec3_angle;
}
// Pongles ]

#if defined( CLIENT_DLL )

#include "iviewrender.h"
#include "ivieweffects.h"

#endif

static ConVar smoothstairs( "smoothstairs", "1", FCVAR_REPLICATED, "Smooth player eye z coordinate when traversing stairs." );

//-----------------------------------------------------------------------------
// Handle view smoothing when going up stairs
//-----------------------------------------------------------------------------
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
#if defined( CLIENT_DLL )
	IClientVehicle *pVehicle; 
#else
	IServerVehicle *pVehicle;
#endif
	pVehicle = GetVehicle();

	if ( !pVehicle )
	{
		if ( IsObserver() )
			CalcObserverView( eyeOrigin, eyeAngles, fov );
		else
			CalcPlayerView( eyeOrigin, eyeAngles, fov );
	}
	else
	{
		CalcVehicleView( pVehicle, eyeOrigin, eyeAngles, zNear, zFar, fov );
	}
}


void CBasePlayer::CalcViewModelView( const Vector& eyeOrigin, const QAngle& eyeAngles)
{	
	CBaseViewModel *vm = GetViewModel();

	if ( !vm )
        return;

    vm->CalcViewModelView( this, eyeOrigin, eyeAngles + m_Local.m_vecPunchAngle );
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

	CalcViewRoll( eyeAngles );

	// Pongles [

	ApplyPlayerView( eyeOrigin, eyeAngles, fov );

	// Pongles ]

	// Apply punch angle
	VectorAdd( eyeAngles, m_Local.m_vecRecoilPunchAngle, eyeAngles );

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

//-----------------------------------------------------------------------------
// Purpose: The main view setup function for vehicles
//-----------------------------------------------------------------------------
void CBasePlayer::CalcVehicleView( 
#if defined( CLIENT_DLL )
	IClientVehicle *pVehicle, 
#else
	IServerVehicle *pVehicle,
#endif
	Vector& eyeOrigin, QAngle& eyeAngles,
	float& zNear, float& zFar, float& fov )
{
	Assert( pVehicle );


	// Initialize with default origin + angles
	int nRole = pVehicle->GetPassengerRole( this );
	pVehicle->GetVehicleViewPosition( nRole, &eyeOrigin, &eyeAngles );

#if defined( CLIENT_DLL )

	fov = GetFOV();

	// Allows the vehicle to change the clip planes
	pVehicle->GetVehicleClipPlanes( zNear, zFar );
#endif

	// Snack off the origin before bob + water offset are applied
	Vector vecBaseEyePosition = eyeOrigin;

	CalcViewRoll( eyeAngles );

	// Pongles [

	// apply punch angle
	VectorAdd( eyeAngles, m_Local.m_vecRecoilPunchAngle, eyeAngles );

	// Pongles ]

#if defined( CLIENT_DLL )
	if ( !prediction->InPrediction() )
	{
		// Shake it up baby!
		vieweffects->CalcShake();
		vieweffects->ApplyShake( eyeOrigin, eyeAngles, 1.0 );
	}
#endif

}

// Pongles [
void CBasePlayer::CalcObserverView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
#if defined( CLIENT_DLL )
	switch ( GetObserverMode() )
	{

		case OBS_MODE_DEATHCAM	:	CalcDeathCamView( eyeOrigin, eyeAngles, fov );
									break;

		case OBS_MODE_ROAMING:		CalcRoamingView( eyeOrigin, eyeAngles, fov );
									break;

		case OBS_MODE_IN_EYE	:	CalcInEyeCamView( eyeOrigin, eyeAngles, fov );
									break;

		case OBS_MODE_CHASE		:	CalcChaseCamView( eyeOrigin, eyeAngles, fov  );
									break;
	}
#else
	// on server just copy target postions, final view positions will be calculated on client
	VectorCopy( EyePosition(), eyeOrigin );
	VectorCopy( EyeAngles(), eyeAngles );
#endif
}

void CBasePlayer::CalcViewRoll( QAngle &eyeAngles )
{
}

void CBasePlayer::DoMuzzleFlash( void )
{
	CBaseViewModel *vm = GetViewModel( );

	if( vm )
		vm->DoMuzzleFlash( );
}

// Pongles ]

float CBasePlayer::GetFOVDistanceAdjustFactor()
{
	float defaultFOV	= GetDefaultFOV();
	float localFOV		= (float)GetFOV();

	if ( localFOV == defaultFOV || defaultFOV < 0.001f )
	{
		return 1.0f;
	}

	// If FOV is lower, then we're "zoomed" in and this will give a factor < 1 so apparent LOD distances can be
	//  shorted accordingly
	return localFOV / defaultFOV;

}

// Pongles [

void CBasePlayer::SharedSpawn( void )
{
	SetMoveType( MOVETYPE_WALK );
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetFriction( 1.0f );

	pl.deadflag	= false;
	m_lifeState	= LIFE_ALIVE;
	m_iHealth = 100;
	m_takedamage = DAMAGE_YES;

	m_Local.m_bDrawViewmodel = true;
	m_Local.m_flStepSize = sv_stepsize.GetFloat( );
	m_Local.m_bAllowAutoMovement = true;

	m_nRenderFX = kRenderFxNone;
	m_flNextAttack = gpGlobals->curtime;
	m_flMaxspeed = 0.0f;

	// don't let uninitialized value here hurt the player
	m_Local.m_flFallVelocity = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CBasePlayer::GetDefaultFOV( void ) const
{
#ifdef CLIENT_DLL

	if( !IsLocalPlayer( ) )
		return g_pGameRules->DefaultFOV( );

#endif

	return ( m_Local.m_iDefaultFOV == 0.0f ? g_pGameRules->DefaultFOV( ) : m_Local.m_iDefaultFOV );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CBasePlayer::GetDefaultViewmodelFOV( void ) const
{
	return g_pGameRules->DefaultWeaponFOV( );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBasePlayer::GetScopeFOV( void ) const
{
    return m_Local.m_iScopeFOV;
}

// Pongles ]

void CBasePlayer::AvoidPhysicsProps( CUserCmd *pCmd )
{
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
}
