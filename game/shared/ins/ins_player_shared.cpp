//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "ins_gamerules.h"
#include "ins_spawnprotection_shared.h"
#include "ins_mantlezone_shared.h"
#include "weapon_ballistic_base.h"
#include "in_buttons.h"
#include "play_team_shared.h"
#include "ins_squad_shared.h"
#include "team_lookup.h"
#include "weapondef.h"
#include "filesystem.h"
#include "keyvalues.h"
#include "debugoverlay_shared.h"
#include "ins_utils.h"
#include "decals.h"
#include "soundenvelope.h"
#include "convar.h"

#ifdef GAME_DLL

#include "engine/ienginesound.h"
#include "soundemittersystem/isoundemittersystembase.h"

#else

#include "pain_helper.h"
#include "c_playerresource.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
void CINSPlayer::SharedSpawn( void )
{
	BaseClass::SharedSpawn( );

	m_iPlayerFlags = 0;

	StanceReset( );

	SetCollisionBounds( VEC_HULL_MIN, VEC_HULL_MAX );

	SetMaxSpeed( PLAYER_MAXSPEED_WALK );

	m_nButtons = 0;
}

//=========================================================
//=========================================================
CSpawnProtection *CINSPlayer::GetClippingEntity(void) const
{
	CBaseEntity *pEntity = m_hClippingEntity;

	if(!pEntity)
		return NULL;

	CSpawnProtection *pSpawnProtection = dynamic_cast<CSpawnProtection*>(pEntity);

	if(!pSpawnProtection)
		return NULL;

	return pSpawnProtection;
}

class CMantleZone;
//=========================================================
//=========================================================
CMantleZone *CINSPlayer::GetMantleEntity( void ) const
{
	CBaseEntity *pEntity = m_hMantleEntity;

	if(!pEntity)
		return NULL;

	CMantleZone *pMantleZone = dynamic_cast<CMantleZone*>( pEntity );

	if(!pMantleZone)
		return NULL;

	return pMantleZone;
}

//=========================================================
//=========================================================
void CINSPlayer::StanceReset( void )
{
    m_iCurrentStance = STANCE_STAND;
	m_iLastStance = STANCE_INVALID;
	m_iOldStance = STANCE_INVALID;
    m_flStanceTransMarker = 0.0f;
}

//=========================================================
//=========================================================
bool CINSPlayer::CanStanceTransition( void ) const
{
	return ( ( m_iPlayerFlags & FL_PLAYER_BIPOD ) == 0 );
}

//=========================================================
//=========================================================
bool CINSPlayer::CanChangeStance( int iFromStance, int iToStance )
{
	if( iFromStance == STANCE_PRONE || iToStance == STANCE_STAND )
	{
		if( m_flNextStanceThreshold > gpGlobals->curtime )
			return false;
	}

	CWeaponINSBase *pWeapon = GetActiveINSWeapon( );

	if( pWeapon && !pWeapon->AllowPlayerStance( iFromStance, iToStance ) )
		return false;

	return true;
}

//=========================================================
//=========================================================
#define DIVE_VIEWPUNCH 8.0f;

void CINSPlayer::SetDesiredStance( int iStance )
{
	if( m_iCurrentStance == iStance )
		return;

#ifdef GAME_DLL

	if( !IsSpenceStance( ) && m_iCurrentStance == STANCE_STAND && iStance == STANCE_CROUCH )
		UTIL_SendHint( this, HINT_STANCEALT );

#endif

	m_iOldStance = m_iCurrentStance;

	float flTransitionTime = UTIL_StanceTransitionTime( m_iLastStance, m_iCurrentStance );
	float flTransitioningTime = gpGlobals->curtime - m_flStanceTransMarker;

	m_flStanceTransMarker = gpGlobals->curtime;

	if( m_flStanceTransMarker > 0.0f && flTransitioningTime < flTransitionTime )
		m_flStanceTransMarker += UTIL_StanceTransitionTime( m_iLastStance, iStance ) * ( flTransitioningTime / flTransitionTime );

	bool bDesiredStanceProne = ( iStance == STANCE_PRONE );

	if( bDesiredStanceProne || m_iCurrentStance == STANCE_PRONE )
	{
		Weapon_ToShouldered( false );

		Weapon_CancelReload( );

	#ifdef GAME_DLL

		const char *pszProneSound = NULL;

		if( bDesiredStanceProne )
			pszProneSound = "INSPlayer.ProneTo";
		else
			pszProneSound = "INSPlayer.ProneFrom";

		EmitSound( pszProneSound );

	#endif

		// to simulate diving
		if( bDesiredStanceProne && IsSprinting( ) )
		{
			static float flViewPunchAdjustX = DIVE_VIEWPUNCH;
			static float flViewPunchAdjustY = flViewPunchAdjustX * 0.25f;

			RecoilViewPunch( QAngle( flViewPunchAdjustX, flViewPunchAdjustY, 0 ) );
		}

		// set threshold
		m_flNextStanceThreshold = gpGlobals->curtime;

		if( bDesiredStanceProne )
			m_flNextStanceThreshold += UTIL_ProneThreshold( m_iLastStance );
		else
			m_flNextStanceThreshold += UTIL_ProneThreshold( iStance );
	}

	m_iLastStance = m_iCurrentStance;
	m_iCurrentStance = iStance;
}

//=========================================================
//=========================================================
void CINSPlayer::HandleTransitionTime( float flTransitionTime )
{
	Assert( m_flStanceTransMarker != 0.0f );

	if( ( gpGlobals->curtime - m_flStanceTransMarker ) < flTransitionTime )
		return;

	int iNewStance = UTIL_GetNextStance( m_iLastStance, m_iCurrentStance );

	if( iNewStance != m_iCurrentStance )
	{
		m_iLastStance = iNewStance;
		m_flStanceTransMarker = gpGlobals->curtime;

		return;
	}

	if( iNewStance == STANCE_PRONE )
		DoProneViewEffect( false );

	m_flStanceTransMarker = 0.0f;
}

//=========================================================
//=========================================================
void CINSPlayer::DoProneViewEffect( bool bFrom )
{
	static QAngle angProneTo = QAngle( 6.0f, 6.0f, 6.0f );
	static QAngle angProneFrom = QAngle( -2.0f, -2.0f, -2.0f );

	ViewPunch( bFrom ? angProneFrom : angProneTo );
}

//=========================================================
//=========================================================
bool CINSPlayer::IsSpenceStance( void ) const
{
	return GetCmdValue( CMDREGISTER_STANCE ) ? true : false;
}

//=========================================================
//=========================================================
int CINSPlayer::GetCurrentStance( void ) const
{
	return m_iCurrentStance;
}

//=========================================================
//=========================================================
int CINSPlayer::GetLastStance( void ) const
{
	return m_iLastStance;
}

//=========================================================
//=========================================================
float CINSPlayer::GetStanceTransitionMarker( void ) const
{
	return m_flStanceTransMarker;
}

//=========================================================
//=========================================================
bool CINSPlayer::InStanceTransition( void ) const
{
	return ( m_flStanceTransMarker > 0.0f );
}

//=========================================================
//=========================================================
bool CINSPlayer::IsProned( void ) const
{
	return ( m_iCurrentStance == STANCE_PRONE );
}

//=========================================================
//=========================================================
bool CINSPlayer::IsCrouched( void ) const
{
	return ( m_iCurrentStance == STANCE_CROUCH );
}

//=========================================================
//=========================================================
bool CINSPlayer::IsStanding( void ) const
{
	return ( m_iCurrentStance == STANCE_STAND );
}

//=========================================================
//=========================================================
bool CINSPlayer::IsMoving(void) const
{
	return UTIL_IsMoving( GetAbsVelocity( ) );
}

//=========================================================
//=========================================================
#define MIN_CRAWLSPEED 15

bool CINSPlayer::IsCrawling(void) const
{
	return ( IsProned( ) && UTIL_IsMoving( GetAbsVelocity( ), MIN_CRAWLSPEED ) );
}

//=========================================================
//=========================================================
bool CINSPlayer::IsWalking( void ) const
{
	return ( ( m_iPlayerFlags & FL_PLAYER_WALKING ) != 0 );
}

//=========================================================
//=========================================================
void CINSPlayer::HandleWalking( void )
{
	if( m_nButtons & IN_SHUFFLE )
		AddPlayerFlag( FL_PLAYER_WALKING );
	else
		RemovePlayerFlag( FL_PLAYER_WALKING );
}

//=========================================================
//=========================================================
bool CINSPlayer::IsSprinting( void ) const
{
	return ( ( GetPlayerFlags( ) & FL_PLAYER_SPRINTING ) != 0 );
}

//=========================================================
//=========================================================
int CINSPlayer::GetStamina( void ) const
{
	return m_iStamina;
}

//=========================================================
//=========================================================
bool CINSPlayer::IsBrustSprint( void ) const
{
	return ( m_iStamina >= STAMINA_NEEDED_BURST );
}

//=========================================================
//=========================================================
void CINSPlayer::SimulateStamina( void )
{
	static float flStaminaFactors[ STANCE_COUNT ] = { 
		STAMINA_REGAIN_FACTOR_STAND, 
		STAMINA_REGAIN_FACTOR_CROUCH, 
		STAMINA_REGAIN_FACTOR_PRONE };

	// if we're sprinting and we can't sprint ... stop it
	if( IsSprinting( ) && !CanSprint( ) )
		SetSprinting( false );

	// handle sprint buttons
	HandleSprintButtons( );

	// reset bunnyhop time when passed threshold
	if( gpGlobals->curtime >= m_flStartBunnyHopTime + m_flBunnyHopLength )
		m_flStartBunnyHopTime = m_flBunnyHopLength = 0.0f;

#ifdef GAME_DLL

	CSoundEnvelopeController &SoundEnvelopeController = CSoundEnvelopeController::GetController( );

	if( !IsBrustSprint( ) )
	{
		if( !m_pStaminaSound )
		{
			CPASAttenuationFilter filter( this );
			m_pStaminaSound = ( CSoundEnvelopeController::GetController( ) ).SoundCreate( filter, entindex( ), CHAN_BODY, "INSPlayer.Sprinting", ATTN_NORM );

			static float flEstimatedTake = ( STAMINA_TAKE * STAMINA_UPDATE_THRESHOLD );

			SoundEnvelopeController.Play( m_pStaminaSound, 0.0f, 100 );
			SoundEnvelopeController.SoundChangeVolume( m_pStaminaSound, 1.0f, m_iStamina * flEstimatedTake );
		}
	}
	else if( m_pStaminaSound )
	{
		SoundEnvelopeController.SoundFadeOut( m_pStaminaSound, 1.0f );
		m_pStaminaSound = NULL;
	}

#endif

	// handle the stamina when update past threshold and we're spriting or we're not full stamina
	if( gpGlobals->curtime > m_flStaminaUpdateThreshold && ( IsSprinting( ) || m_iStamina < STAMINA_MAX ) )
	{
		if( IsSprinting( ) )
		{
			// remove stamina when sprinting
			m_iStamina -= RoundFloatToInt( ( STAMINA_TAKE * ( 1.0f / m_INSLocal.m_flWeightFactor ) ) );
			m_iStamina = max( m_iStamina, 0 );
		}
		else if( m_iStamina < STAMINA_MAX )
		{
			// start regaining
			m_iStamina += RoundFloatToInt( ( STAMINA_REGAIN * m_INSLocal.m_flWeightFactor ) * flStaminaFactors[ m_iCurrentStance ] ) * ( IsMoving( ) ? 0.5f : 1.0f );
			m_iStamina = min( m_iStamina, STAMINA_MAX );
		}
			
		// mark the new threshold
		m_flStaminaUpdateThreshold = gpGlobals->curtime + STAMINA_UPDATE_THRESHOLD;
	}
}

//=========================================================
//=========================================================
void CINSPlayer::SetSprinting( bool bState )
{
	if( bState )
	{
		AddPlayerFlag( FL_PLAYER_SPRINTING );
		SetMaxSpeed( PLAYER_MAXSPEED_BURST );

		Weapon_ToShouldered( true );
	}
	else
	{
		RemovePlayerFlag( FL_PLAYER_SPRINTING );
		SetMaxSpeed( PLAYER_MAXSPEED_WALK );
	}
}

//=========================================================
//=========================================================
void CINSPlayer::HandleSprintButtons( void )
{
	if( IsSprinting( ) )
	{
		if( m_afButtonReleased & IN_SPRINT )
			SetSprinting( false );
	}
	else
	{
		if( m_afButtonPressed & IN_SPRINT && CanStartSprint( ) )
			SetSprinting( true );
	}
}

//=========================================================
//=========================================================
bool CINSPlayer::CanSprint( void )
{
	// must be walking
	if( GetMoveType( ) != MOVETYPE_WALK )
		return false;

	// they must be standing, moving and not jumping
	if( !IsStanding( ) || !IsMoving( ) || IsJumping( ) )
		return false;

	// must not be pressing back
	if( m_nButtons & IN_BACK || m_afButtonPressed & IN_BACK )
		return false;

	// must be on dryland
	if( GetWaterLevel( ) != WL_NotInWater )
		return false;

	return true;
}

//=========================================================
//=========================================================
bool CINSPlayer::CanStartSprint( void )
{
	// must be hitting forward
	if( ( m_nButtons & IN_FORWARD ) == 0 && ( m_afButtonPressed & IN_FORWARD ) == 0 )
		return false;

	// check cansprint
	if( !CanSprint( ) )
		return false;

	// ensure we're not reloading
	if( IsReloading( ) )
		return false;

	// check the weapon
	CWeaponINSBase *pWeapon = GetActiveINSWeapon( );

	if( pWeapon && !pWeapon->AllowSprintStart( ) )
		return false;

	return true;
}

//=========================================================
//=========================================================
void CINSPlayer::ResetSprinting( void )
{
	SetSprinting( false );

	m_iStamina = STAMINA_MAX;
	m_flStaminaUpdateThreshold = 0.0f;

#ifdef GAME_DLL

	if( m_pStaminaSound )
	{
		( CSoundEnvelopeController::GetController( ) ).SoundDestroy( m_pStaminaSound );
		m_pStaminaSound = NULL;
	}

#endif
}

//=========================================================
//=========================================================
void CINSPlayer::SimulateDamageDecay( void )
{
	if( m_iDamageDecay == 0 || gpGlobals->curtime < m_flDamageDecayThreshold )
		return;

	m_iDamageDecay -= DAMAGE_REDUCTION;
	m_iDamageDecay = max( m_iDamageDecay, 0 );

	m_flDamageDecayThreshold = gpGlobals->curtime + DAMAGE_UPDATE_TIME;

#ifdef _DEBUG

	Msg("Damage Decay: %i\n", m_iDamageDecay);

#endif
}

//=========================================================
//=========================================================
bool CINSPlayer::IsLeaning( void ) const
{
	return ( m_iLeanType != PLEANING_NONE );
}

//=========================================================
//=========================================================
bool CINSPlayer::CanLean( void ) const
{
	CWeaponINSBase *pWeapon = GetActiveINSWeapon( );

	if( !pWeapon || !pWeapon->AllowLeaning( ) )
		return false;

	if( IsProned( ) || IsMoving( ) || IsJumping( ) || IsReloading( ) )
		return false;

	if( GetWaterLevel( ) != WL_NotInWater )
		return false;

	return true;
}

//=========================================================
//=========================================================
#define LEAN_LEAN_TIME 4.0f
#define LEAN_BADLEAN_TIME 8.0f

#ifdef _DEBUG

ConVar leaningshow( "sv_leaning_show", "1", FCVAR_REPLICATED );
ConVar leaninglength( "sv_leaning_length", "25", FCVAR_REPLICATED );
ConVar leaningbox( "sv_leaning_box", "4", FCVAR_REPLICATED );

#define LEAN_WALLCHECK_LENGTH leaninglength.GetFloat( )
#define LEAN_WALLCHECK_BOX leaningbox.GetFloat( )

#else

#define LEAN_WALLCHECK_LENGTH 25
#define LEAN_WALLCHECK_BOX 4

#endif

void CINSPlayer::HandleLeaning( void )
{
	// ensure we can lean
	if( !CanLean( ) )
	{
		// move back to center
		if( m_INSLocal.m_flLean != 0.0f )
			m_INSLocal.m_flLean = Approach( 0.0f, m_INSLocal.m_flLean, gpGlobals->frametime * LEAN_BADLEAN_TIME );

		return;
	}

	// handle buttons
	float flTarget = 0.0f;

	if( m_nButtons & IN_LEAN_LEFT )
		flTarget = 1.0f;
	else if( m_nButtons & IN_LEAN_RIGHT )
		flTarget = -1.0f;

	// ... ensure we are not near a wall
	if( flTarget != 0.0f )
	{
		float flFraction = ( ( flTarget > 0.0f ) ? 1.0f : -1.0f );

		Vector vecOrigin, vecEnd, vecForward, vecRight, vecDown;

		vecOrigin = GetAbsOrigin( ) + GetViewOffset( );

		vecDown = Vector( 0, 0, -1 );
		AngleVectors( EyeAngles( ), &vecForward );
		vecRight = CrossProduct( vecForward, vecDown );
		vecEnd = vecOrigin + ( vecRight * LEAN_WALLCHECK_LENGTH * flFraction );

		static Vector vecLeanMin = Vector( -LEAN_WALLCHECK_BOX, -LEAN_WALLCHECK_BOX, -LEAN_WALLCHECK_BOX );
		static Vector vecLeanMax = Vector( LEAN_WALLCHECK_BOX, LEAN_WALLCHECK_BOX, LEAN_WALLCHECK_BOX );
	
		trace_t tr;
		UTIL_TraceHull( vecOrigin, vecEnd, vecLeanMin, vecLeanMax, 
			MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr );

		if( tr.DidHit( ) )
		{
			if( tr.fraction < 0.75f )
				flTarget = tr.fraction * flFraction;
			else
				flTarget = 0.0f;
		}

	#ifdef _DEBUG

		if( leaningshow.GetBool( ) )
		{
		#ifdef GAME_DLL
	
			NDebugOverlay::Line( vecOrigin, vecEnd, 0, 0, 255, true, 0.1f );

		#else

			debugoverlay->AddLineOverlay( vecOrigin, vecEnd, 255, 0, 0, true, 0.1f );

		#endif
		}

	#endif
	}

	if( m_INSLocal.m_flLean != flTarget )
		m_INSLocal.m_flLean = Approach( flTarget, m_INSLocal.m_flLean, gpGlobals->frametime * LEAN_LEAN_TIME );

	// update leantype
	m_iLeanType = CalculateLeanType( );
}

//=========================================================
//=========================================================
int CINSPlayer::CalculateLeanType( void ) const
{
	if( !CanLean( ) || abs( m_INSLocal.m_flLean ) < 0.5f )
		return PLEANING_NONE;

	return ( m_INSLocal.m_flLean > 0.0f ) ? PLEANING_LEFT : PLEANING_RIGHT;
}

//=========================================================
//=========================================================
bool CINSPlayer::IsBandaging( void ) const
{
	return ( m_INSLocal.m_BandagingPlayer != NULL );
}

//=========================================================
//=========================================================
void CINSPlayer::Bandage( void )
{
	// ensure player is on a playteam and alive
	if( !IsRunningAround( ) )
		return;

	// lets a player
	CINSPlayer *pBandagePlayer = NULL;

	// do a traceline and see if you can find another player to bandage
	if( IsMedic( ) )
	{
		Vector vecStart, vecDir, vecEnd;

		vecStart = GetAbsOrigin( ) + GetViewOffset( );
		AngleVectors( EyeAngles( ), &vecDir );

		vecEnd = vecStart + ( vecDir * PLAYER_BANDAGE_LENGTH_FWD );
		vecStart = vecStart - ( vecDir * PLAYER_BANDAGE_LENGTH_BWD );

		trace_t tr;
		CTraceFilterSimple traceFilter( this, COLLISION_GROUP_NONE );

		UTIL_TraceLine( vecStart, vecEnd, MASK_PLAYERSOLID, &traceFilter, &tr );
	
		if( tr.fraction != 1.0f && tr.m_pEnt && tr.m_pEnt->IsPlayer( ) )
		{
			CINSPlayer *pPossibleBandage = ToINSPlayer( tr.m_pEnt );

			// ensure player is on same team
			if( pPossibleBandage && ( pPossibleBandage->GetTeamID( ) == GetTeamID( ) ) )
				pBandagePlayer = pPossibleBandage;
		}
	}

	// no possible bandage? bandage yourself
	if( !pBandagePlayer )
		pBandagePlayer = this;

	// can we bandage?
	if( !CanBandage( pBandagePlayer ) )
		return;

	// bandage them up
	m_INSLocal.m_BandagingPlayer = pBandagePlayer;
	m_INSLocal.m_flBandageThreshold = gpGlobals->curtime + PLAYER_BANDAGE_TIME;

#ifdef _DEBUG

	Msg( "BANDAGE: Starting %s\n", pBandagePlayer->GetPlayerName( ) );

#endif

	// holster weapon
	CBaseCombatWeapon *pWeapon = GetActiveWeapon( );

	if( pWeapon )
		pWeapon->Holster( );

	// TODO: need to restrict the mouse and play sounds etc
}

//=========================================================
//=========================================================
bool CINSPlayer::FinishedBandage( CINSPlayer *pPlayer )
{
	return ( gpGlobals->curtime >= m_INSLocal.m_flBandageThreshold );
}

//=========================================================
//=========================================================
bool CINSPlayer::CanBandage( CINSPlayer *pPlayer )
{
	if( !pPlayer || !pPlayer->IsPlayer( ) )
		return false;

	// no need to bandage the dead
	if( !pPlayer->IsAlive( ) )
		return false;

	// i'm full of it!
	if( pPlayer->GetHealthType( ) == HEALTHTYPE_UNINJURED )
		return false;

	// can't bandage serious wounds when not a medic
	if( pPlayer->GetHealthType( ) == HEALTHTYPE_SERIOUS && !IsMedic( ) )
		return false;

	// don't let us wander off
	if( IsMoving( ) )
		return false;
	
	// don't let them wander off
	if( pPlayer != this && pPlayer->IsMoving( ) )
		return false;

	// can't bandage when prone
	if( IsProned( ) )
		return false;

	// ensure weapon can be holstered
	CWeaponINSBase *pWeapon = GetActiveINSWeapon( );

	if( pWeapon && !pWeapon->CanHolster( ) )
		return false;

	return true;
}

//=========================================================
//=========================================================
void CINSPlayer::HandleBandaging( void )
{
	// try and start bandaging
	if( !IsBandaging( ) )
	{
		if( m_afButtonReleased & IN_BANDAGE )
			Bandage( );

		return;
	}

	// get player and ...
	CINSPlayer *pPlayer = ToINSPlayer( m_INSLocal.m_BandagingPlayer );

	// stop bandaging if they die/wander off etc
	if( !CanBandage( pPlayer ) )
	{
		StopBandaging( );
		return;
	}

	// finished?
	if( FinishedBandage( pPlayer ) )
	{
	#ifdef GAME_DLL

		pPlayer->UpdateHealth( PLAYER_BANDAGE_MAXHEALTH * ( 1.0f - pPlayer->GetHealthFraction( ) ) );

	#endif

		StopBandaging( );
	}
}

//=========================================================
//=========================================================
void CINSPlayer::StopBandaging( void )
{
	ResetBandaging( );

	CBaseCombatWeapon *pWeapon = GetActiveWeapon( );

	if( pWeapon )
		pWeapon->Deploy( );

#ifdef _DEBUG

	Msg( "BANDAGE: Stopped\n" );

#endif
}

//=========================================================
//=========================================================
void CINSPlayer::ResetBandaging( void )
{
	m_INSLocal.m_BandagingPlayer = NULL;
	m_INSLocal.m_flBandageThreshold = 0.0f;
}

//=========================================================
//=========================================================
CPlayerClass *CINSPlayer::GetClass( void ) const
{
	CINSSquad *pSquad = GetSquad( );
	return ( pSquad ? pSquad->GetClass( GetSlotID( ) ) : NULL );
}

//=========================================================
//=========================================================
int CINSPlayer::GetClassID( void ) const
{
	CINSSquad *pSquad = GetSquad( );
	return ( pSquad ? pSquad->GetClassID( GetSlotID( ) ) : INVALID_CLASS );
}

//=========================================================
//=========================================================
bool CINSPlayer::OnTeam( CTeam *pTeam ) const
{
	return ( GetTeam( ) == pTeam );
}

//=========================================================
//=========================================================
bool CINSPlayer::OnSquad( CINSSquad *pSquad ) const
{
	return ( GetSquad( ) == pSquad );
}

//=========================================================
//=========================================================
bool CINSPlayer::IsMedic(void) const
{
	CPlayerClass *pClass = GetClass();
	return ( pClass && pClass->GetType() == CLASSTYPE_MEDIC );
}

//=========================================================
//=========================================================
CBaseCombatWeapon *CINSPlayer::GetPrimaryWeapon(void) const
{
	return GetWeapon(WEAPONSLOT_PRIMARY);
}

//=========================================================
//=========================================================
CBaseCombatWeapon *CINSPlayer::GetSecondaryWeapon(void) const
{
	return GetWeapon(WEAPONSLOT_SECONDARY);
}

//=========================================================
//=========================================================
bool CINSPlayer::HasPrimaryWeapon(void)
{
	return (GetPrimaryWeapon() != NULL);
}

//=========================================================
//=========================================================
bool CINSPlayer::HasSecondaryWeapon(void)
{
	return (GetPrimaryWeapon() != NULL);
}

//=========================================================
//=========================================================
void CINSPlayer::SetViewTransition( int iTransitionTarget, float flTransitionTime )
{
	if( InStanceTransition( ) )
		return;

	Assert( flTransitionTime != 0.0f );

	if( flTransitionTime == 0.0f )
		return;

	m_flViewTransitionLength = flTransitionTime;
	m_flViewTransitionEnd = gpGlobals->curtime + flTransitionTime;
	m_iViewTransitionFrom = GetViewOffset( ).z;
	m_iViewTransitionTarget = iTransitionTarget;
}

//=========================================================
//=========================================================
void CINSPlayer::SetViewTransition( const Vector &vecTransitionTarget, float flTransitionTime )
{
	SetViewTransition( vecTransitionTarget.z, flTransitionTime );
}

//=========================================================
//=========================================================
float CINSPlayer::ViewTransitionLength( void ) const
{
	return m_flViewTransitionLength;
}

//=========================================================
//=========================================================
float CINSPlayer::ViewTransitionEnd( void ) const
{
	return m_flViewTransitionEnd;
}

//=========================================================
//=========================================================
int CINSPlayer::ViewTransitionFrom( void ) const
{
	return m_iViewTransitionFrom;
}

//=========================================================
//=========================================================
int CINSPlayer::ViewTransitionTarget( void ) const
{
	return m_iViewTransitionTarget;
}

//=========================================================
//=========================================================
void CINSPlayer::SetViewBipodOffset( int iOffset )
{
	m_iViewOffsetBipod = iOffset;
}

//=========================================================
//=========================================================
void CINSPlayer::ResetViewBipodOffset( void )
{
	m_iViewOffsetBipod = 0;
}

//=========================================================
//=========================================================
int CINSPlayer::ViewOffsetBipod( void ) const
{
	return m_iViewOffsetBipod;
}

//=========================================================
//=========================================================
Vector CINSPlayer::GetCurrentViewOffset( void )
{
	return UTIL_PlayerViewOffset( this, m_iCurrentStance );
}

//=========================================================
//=========================================================
float CINSPlayer::CalcProneRoll( const QAngle &angAngles, const Vector &vecVelocity )
{
	if( !IsProned( ) )
		return 0.0f;

	// multiply by cosine of some counter
	// TODO: global timer for now, use a different timer?
	return cos( 4.0f * gpGlobals->curtime ) * ( vecVelocity.Length2D( ) * 0.01f );
}

//=========================================================
//=========================================================
void CINSPlayer::CalcViewRoll( QAngle &eyeAngles )
{
	if( GetMoveType( ) == MOVETYPE_NOCLIP )
		return;

	BaseClass::CalcViewRoll( eyeAngles );
	eyeAngles[ ROLL ] += CalcProneRoll( GetAbsAngles( ), GetAbsVelocity( ) );
}

//=========================================================
//=========================================================
void CINSPlayer::CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov )
{
	if( InViewpoint( ) )
	{
	#ifdef CLIENT_DLL

		VectorCopy( INSRules( )->GetCurrentViewpointOrigin( ), eyeOrigin );
		VectorCopy( INSRules( )->GetCurrentViewpointAngle( ), eyeAngles );

	#endif

		fov = GetFOV( );

		return;
	}

	BaseClass::CalcView( eyeOrigin, eyeAngles, zNear, zFar, fov );
}

//=========================================================
//=========================================================
const QAngle &CINSPlayer::HeadAngles( void ) const
{
	return vec3_angle;
}

//=========================================================
//=========================================================
float CINSPlayer::GetHealthFraction( void ) const
{
#ifdef CLIENT_DLL

	// PNOTE: m_iHealth is only sent to localplayers
	Assert( IsLocalPlayer( ) );

#endif

	return m_iHealth / ( float )GetMaxHealth( );
}

//=========================================================
//=========================================================
bool OnSameTeam(CINSPlayer *pP1, CINSPlayer *pP2)
{
	return (pP1->GetTeamID() == pP2->GetTeamID());
}

bool OnSameTeam(CINSPlayer *pP1, int iTeamID)
{
	return (pP1->GetTeamID() == iTeamID);
}

bool CINSPlayer::IsCustomized(void) const
{
	return m_bIsCustomized;
}

bool CINSPlayer::IsCommander(void) const
{
	return m_bCommander;
}

bool CINSPlayer::IsRealObserver(void) const
{
	return (IsObserver() && GetObserverMode() != OBS_MODE_DEATHCAM);
}

void CINSPlayer::UpdateWeaponStates(void)
{
	CWeaponINSBase *pWeapon = GetActiveINSWeapon();

	if( pWeapon )
		pWeapon->UpdateIdleState();
}

bool CINSPlayer::IsReloading( void ) const
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon( );
	return ( pWeapon && pWeapon->IsReloading( ) );
}

bool CINSPlayer::IsJumping( void ) const
{
#ifdef CLIENT_DLL

	Assert( IsLocalPlayer( ) );

#endif

	return ( m_Local.m_flJumpTime > 0.0f || ( ( GetFlags( ) & FL_ONGROUND ) == 0 ) );
}

//=========================================================
//=========================================================
bool CINSPlayer::CanJump(void) const
{
	// cannot jump while in bipod
	if( ( m_iPlayerFlags & FL_PLAYER_BIPOD ) != 0 )
		return false;

	// cannot while in clipping entity
	if( m_hClippingEntity )
		return false;

	// can only jump while in a solid standing state
	if( m_iCurrentStance != STANCE_STAND )
		return false;

	if( InStanceTransition( ) )
		return false;

	// cannot bunnyhop
	if( m_flBunnyHopLength != 0.0f )
		return false;

	// or while reloading
	CWeaponINSBase *pWeapon = GetActiveINSWeapon( );

	if( pWeapon )
	{
		if( pWeapon->IsReloading( ) )
			return false;

		if( !pWeapon->AllowJump( ) )
			return false;
	}

	return true;
}

//=========================================================
//=========================================================
CPlayTeam *CINSPlayer::GetPlayTeam(void) const
{
	if(!OnPlayTeam())
		return NULL;

	return GetGlobalPlayTeam(GetTeamID());
}

CTeamLookup *CINSPlayer::GetTeamLookup(void) const
{
	CPlayTeam *pTeam = GetPlayTeam();

	if(!pTeam)
		return NULL;

	return pTeam->GetTeamLookup();
}

//=========================================================
const char *CINSPlayer::GetRankName( void ) const
{
	CTeamLookup *pTeam = GetTeamLookup( );
	int iRankID = GetRank( );

	if( !pTeam || iRankID == INVALID_RANK )
		return NULL;

	return pTeam->GetRankName( iRankID );
}

const char *CINSPlayer::GetFullRankName(void) const
{
	CTeamLookup *pTeam = GetTeamLookup();
	int iRankID = GetRank();

	if(!pTeam || iRankID == INVALID_RANK)
		return NULL;

	return pTeam->GetFullRankName(iRankID);
}

//=========================================================
//=========================================================
#define WEAPON_WEIGHT_BREATH_LIMIT 7.0f

#ifdef _DEBUG

ConVar lissajous_a( "lissajous_a", "2", FCVAR_REPLICATED );
ConVar lissajous_b( "lissajous_b", "3", FCVAR_REPLICATED );
ConVar lissajous_curve_speed( "lissajous_curve_speed", "0.5", FCVAR_REPLICATED );
ConVar lissajous_rot_speed( "lissajous_rot_speed", "1", FCVAR_REPLICATED );

#define SWAY_LISSAJOUS_A lissajous_a.GetInt( )
#define SWAY_LISSAJOUS_B lissajous_b.GetInt( )
#define SWAY_LISSAJOUS_SPEED lissajous_curve_speed.GetFloat( )
#define SWAY_ROTATION_SPEED lissajous_rot_speed.GetFloat( )

#else

#define SWAY_LISSAJOUS_A 2
#define SWAY_LISSAJOUS_B 3
#define SWAY_LISSAJOUS_SPEED 0.2
#define SWAY_ROTATION_SPEED 1

#endif

void CINSPlayer::SimulateBreathing( void )
{
	// reset old values
	ResetBreathing( );

	// find wepaon
	CWeaponINSBase *pWeapon = GetActiveINSWeapon( );

	if( !pWeapon || !pWeapon->AllowBreathing( ) )
		return;	

	bool bIronsights = ( GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) ? true : false;

	// work out if its actually needed
	float flFactor = 1.0f;

	if( ( GetPlayerFlags( ) & FL_PLAYER_BIPOD ) == 0 )
	{
		if( m_iStamina < STAMINA_LOW )
		{
			flFactor = GetWeaponDef( )->flShakeLowStamina;
		}
		else
		{
			if( m_iDamageDecay > 0 )
				flFactor = GetWeaponDef( )->flShakeWounded;
			else if( IsMoving( ) )
				flFactor = GetWeaponDef( )->flShakeMoving;
			else if( m_flBreathTime != 0.0f && ( gpGlobals->curtime - m_flBreathTime ) >= GetWeaponDef( )->flShakeIronsightHoldTime )
				flFactor = GetWeaponDef( )->flShakeIronsightHold;

			if( m_iStamina < STAMINA_HIGH )
			{
				float flStaminaMax = ( 1.0f - ( ( float )m_iStamina / STAMINA_MAX ) ) * GetWeaponDef( )->flShakeLowStamina;
				flFactor = max( flFactor, flStaminaMax );
			}
		}
	}
	else
	{
		flFactor = 0.0f;
	}

	// find base radius
	float flRadius = GetWeaponDef( )->flShake;

	if( flFactor != 0.0f )
	{
		switch( m_iCurrentStance )
		{
			case STANCE_STAND:
			{
				if( bIronsights )
					flRadius = GetWeaponDef( )->flShakeIronsights;

				break;
			}

			case STANCE_CROUCH:
			{
				if( bIronsights )
					flRadius = GetWeaponDef( )->flShakeCrouchedIronsights;
				else
					flRadius = GetWeaponDef( )->flShakeCrouched;

				break;
			}

			case STANCE_PRONE:
			{
				if( bIronsights )
					flRadius = GetWeaponDef( )->flShakeProned;
				else
					flRadius = GetWeaponDef( )->flShakePronedIronsights;

				break;
			}
		}
	}

	// adjust radius 
	m_flRadiusDesired = clamp( flRadius * flFactor, 0.0f, MAX_SHAKE_RADIUS );
	m_flRadius = Approach( m_flRadiusDesired, m_flRadius, gpGlobals->frametime );

	float flXPos, flYPos;

	if( m_flRadius != 0.0f )
	{
		// work out next position
		flYPos = gpGlobals->curtime * SWAY_LISSAJOUS_SPEED;
		flXPos = sin( SWAY_LISSAJOUS_A * flYPos );
		flYPos = sin( SWAY_LISSAJOUS_B * flYPos );

		UTIL_FastRotate( gpGlobals->curtime * SWAY_ROTATION_SPEED, flXPos, flYPos );

		flXPos *= m_flRadius;
		flYPos *= m_flRadius;
	}
	else
	{
		flXPos = flYPos = 0.0f;
	}

	m_angShakyHands.Init( flXPos, flYPos );
}

//=========================================================
//=========================================================
void CINSPlayer::ResetBreathing( void )
{
    m_angShakyHands.Init( );
}

//=========================================================
//=========================================================
const QAngle &CINSPlayer::GetBreathingAdjust( void ) const
{
	return m_angShakyHands;
}

//=========================================================
//=========================================================
void CINSPlayer::ApplyPlayerView( Vector &eyeOrigin, QAngle &eyeAngles, float &fov )
{
	const QAngle &headAngles = HeadAngles( );
	VectorAdd( eyeAngles, headAngles, eyeAngles );

	// add shaky hands
	CBaseCombatWeapon *pWeapon = GetActiveWeapon( );

	if( pWeapon && !pWeapon->ShouldDrawViewModel( ) )
		VectorAdd( eyeAngles, m_angShakyHands, eyeAngles );

	// add lean
	CalcPlayerLean( eyeOrigin, eyeAngles );

	// add in sprint effects
	if( IsSprinting( ) )
	{
		int iVelocityLength = GetAbsVelocity( ).Length2D( );

		eyeAngles[ YAW ] += cos( 8.0f * gpGlobals->curtime ) * ( iVelocityLength * 0.00175f );
		eyeAngles[ PITCH ] += cos( 15.0f * gpGlobals->curtime ) * ( iVelocityLength * 0.0015f );
	}
}

//=========================================================
//=========================================================
#define MAX_LEAN_ROLL 8.0f
#define MAX_LEAN_SIDE 12.0f
#define MAX_LEAN_DOWN 3.0f

void CINSPlayer::CalcPlayerLean( Vector &eyeOrigin, QAngle &eyeAngles )
{
	// leaning causes the eye origin to move in an arc
	// the the eye angles to roll

	float flLean = m_INSLocal.m_flLean;

	if( flLean == 0.0f )
		return;

	// apply arc
	Vector vecFacing, vecDownDir, vecSideDir, vecSideLean, vecDownLean;

	vecDownDir = Vector( 0, 0, -1 );

	AngleVectors( eyeAngles, &vecFacing );
	CrossProduct( vecFacing, vecDownDir, vecSideDir );

	VectorMA( vec3_origin, m_INSLocal.m_flLean * MAX_LEAN_SIDE , vecSideDir, vecSideLean );
	VectorMA( vec3_origin, abs( m_INSLocal.m_flLean ) * MAX_LEAN_DOWN , vecDownDir, vecDownLean );

	VectorAdd( eyeOrigin, vecSideLean, eyeOrigin );
	VectorAdd( eyeOrigin, vecDownLean, eyeOrigin );

	// apply roll
	eyeAngles.z += -m_INSLocal.m_flLean * MAX_LEAN_ROLL;
}

//=========================================================
//=========================================================
Vector CINSPlayer::Weapon_ShootDirection( void )
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon( );

	Vector vecTemp = BaseClass::Weapon_ShootDirection( );

	if( pWeapon && GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS && !pWeapon->ShouldDrawViewModel( ) )
	{
		QAngle angTemp;

        VectorAngles( vecTemp, angTemp );
        VectorAdd( angTemp, m_angShakyHands, angTemp );
        AngleVectors( angTemp, &vecTemp );

        vecTemp *= VectorNormalize( vecTemp );
    }

    return vecTemp;
}

//=========================================================
//=========================================================
CBaseCombatWeapon *CINSPlayer::GetNextBestWeapon(CBaseCombatWeapon *pCurrentWeapon)
{
	CBaseCombatWeapon *pBest = NULL;
	int iCurrentType = pCurrentWeapon ? pCurrentWeapon->GetWeaponType() : WEAPONTYPE_INVALID;

	if(pCurrentWeapon)
	{
		if(!pCurrentWeapon->CanHolster())
			return NULL;
	}

	for(int i = 0; i < WeaponCount(); i++)
	{
		CBaseCombatWeapon *pCheck = GetWeapon(i);

		if(!pCheck || !pCheck->CanDeploy())
			continue;

		if(iCurrentType == WEAPONTYPE_INVALID || 
			(pCheck->GetWeaponType() != WEAPONTYPE_INVALID && 
			pCheck->GetWeaponType() < iCurrentType))
		{
			pBest = pCheck;
			iCurrentType = pCheck->GetWeaponType();
		}
	}

	return pBest;
}

//=========================================================
//=========================================================
CWeaponINSBase *CINSPlayer::GetActiveINSWeapon(void) const
{
	return ToINSWeapon(GetActiveWeapon());
}

//=========================================================
//=========================================================
void CINSPlayer::Weapon_ToShouldered( bool bForce )
{
	if( GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
	{
		CWeaponINSBase *pWeapon = GetActiveINSWeapon( );

		if( pWeapon )
			pWeapon->SetIronsights( false, bForce );
	}
}

//=========================================================
//=========================================================
void CINSPlayer::Weapon_CancelReload( void )
{
	CWeaponINSBase *pWeapon = GetActiveINSWeapon( );

	if( pWeapon )
		pWeapon->AbortReload( );
}

//=========================================================
//=========================================================
bool CINSPlayer::IsRunningAround( void )
{
	return ( INSRules( )->IsModeRunning( ) && OnPlayTeam( ) && InSquad( ) && IsAlive( ) && !InViewpoint( ) && !IsObserver( ) );
}

//=========================================================
//=========================================================
bool CINSPlayer::InSquad( void )
{
	return ( OnPlayTeam( ) && IsValidSquad( ) );
}

//=========================================================
//=========================================================
int CINSPlayer::GetStatsMemberID(void) const
{
	return m_iStatsMemberID;
}

//=========================================================
//=========================================================
bool CINSPlayer::IsUsingStats(void) const
{
	return m_iStatsMemberID >= 0;
}

//=========================================================
//=========================================================
void CINSPlayer::GetMuzzle(Vector& vecMuzzle, QAngle& angMuzzle)
{
#ifdef CLIENT_DLL
	C_BaseAnimating* pRenderedWeapon = GetRenderedWeaponModel();
	if (pRenderedWeapon && pRenderedWeapon->GetAttachment(1, vecMuzzle, angMuzzle))
		return;
#else
	CBaseViewModel* pVM = GetViewModel();
	if (pVM && pVM->GetAttachment(1, vecMuzzle, angMuzzle))
		return;
#endif

	// revert to default
	vecMuzzle = Weapon_ShootPosition();
	VectorAngles(Weapon_ShootDirection(), angMuzzle);
}

//=========================================================
//=========================================================
DECLARE_STRING_LOOKUP_CONSTANTS(int, hitgroups)
#define GetHitGroupType(k, j) STRING_LOOKUP(hitgroups, k, j)

DEFINE_STRING_LOOKUP_CONSTANTS(int, hitgroups)
	ADD_LOOKUP(HITGROUP_HEAD)
	ADD_LOOKUP(HITGROUP_NECK)
	ADD_LOOKUP(HITGROUP_SPINE)
	ADD_LOOKUP(HITGROUP_PELVIS)
	ADD_LOOKUP(HITGROUP_LEFTTHIGH)
	ADD_LOOKUP(HITGROUP_RIGHTTHIGH)
	ADD_LOOKUP(HITGROUP_LEFTCALF)
	ADD_LOOKUP(HITGROUP_RIGHTCALF)
	ADD_LOOKUP(HITGROUP_LEFTFOOT)
	ADD_LOOKUP(HITGROUP_RIGHTFOOT)
	ADD_LOOKUP(HITGROUP_LEFTUPPERARM)
	ADD_LOOKUP(HITGROUP_RIGHTUPPERARM)
	ADD_LOOKUP(HITGROUP_LEFTFOREARM)
	ADD_LOOKUP(HITGROUP_RIGHTFOREARM)
END_STRING_LOOKUP_CONSTANTS()

//=========================================================
//=========================================================
LoadPlayerData_t &CINSPlayer::GetData(void)
{
	return m_CustomData;
}

//=========================================================
//=========================================================
LoadPlayerData_t CINSPlayer::m_CustomData;

void CINSPlayer::LoadData( void )
{
	static bool bLoadedPlayerData = false;

	if( bLoadedPlayerData )
		return;

	KeyValues * pPlayerData = new KeyValues( "PlayerData" );

	bLoadedPlayerData = true;

	pPlayerData->LoadFromFile( filesystem, "scripts/playerdata.txt" );

#ifdef GAME_DLL

	KeyValues *pPlayerHitgroups = pPlayerData->FindKey( "HitGroups" );

	if( pPlayerHitgroups )
	{
		const char *pszHitGroup;
		int iHitGroupID;

		for( KeyValues *pHitGroup = pPlayerHitgroups->GetFirstTrueSubKey( ); pHitGroup; pHitGroup = pHitGroup->GetNextKey( ) )
		{
			pszHitGroup = pHitGroup->GetName( );

			if( pszHitGroup && GetHitGroupType( pszHitGroup, iHitGroupID ) )
			{
				HitGroupData_t &HitGroupData = m_CustomData.m_HitGroupData[ iHitGroupID ];
				HitGroupData.m_iHitTolerance = pHitGroup->GetInt( "hit_tol", 1 );
				HitGroupData.m_flMultiplyer = pHitGroup->GetFloat( "multiplyer", 1.0f );
			}
		}
	}

#endif

	/*KeyValues *pPlayerInventory = pPlayerData->FindKey( "Inventory" );

	if( pPlayerInventory )
	{
		const char *pszAmmo;
		int iAmmoID;

		for( KeyValues *pAmmo = pPlayerInventory->GetFirstTrueSubKey( ); pAmmo; pAmmo = pAmmo->GetNextKey( ) )
		{
			pszAmmo = pAmmo->GetName( );

			if( !pszAmmo )
				continue;
			
			if( !GetWeaponAmmoType( pszAmmo, iAmmoID ) || !GET_AMMO_DATA( iAmmoID, IsWorldAmmo( ) ) )
			{
				Assert( false );
				continue;
			}
			
			m_CustomData.m_iMaxCarry[ iAmmoID ] = pAmmo->GetFloat( "maxcarry", 0 );
		}
	}*/

#ifdef GAME_DLL

	KeyValues *pRankBoundaries = pPlayerData->FindKey( "RankBoundaries" );

	if( pRankBoundaries )
	{
		const char *pszRank;
		int iRankID;

		for( KeyValues *pRank = pRankBoundaries->GetFirstSubKey( ); pRank; pRank = pRank->GetNextKey( ) )
		{
			pszRank = pRank->GetName( );

			if( pszRank && LookupRank( pszRank, iRankID ) )
				m_RankBoundaries[ iRankID ] = pRank->GetInt( );
		}
	}

#endif

	pPlayerData->deleteThis( );
}

//=========================================================
//=========================================================
void CINSPlayer::SetCmdValue(int iID, int iValue)
{
	m_iCmdRegister.Set(iID, iValue);
}

//=========================================================
//=========================================================
int CINSPlayer::GetCmdValue(int iID) const
{
	return m_iCmdRegister[iID];
}

//=========================================================
//=========================================================
void CINSPlayer::UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity )
{
	float flVolume, flHeight;
	bool bOnLadder;
	Vector vecKnee, vecFeet;

	if( m_flStepSoundTime > 0.0f )
	{
		m_flStepSoundTime -= 1000.0f * gpGlobals->frametime;

		if( m_flStepSoundTime < 0 )
			m_flStepSoundTime = 0.0f;
	}

	if ( m_flStepSoundTime > 0 )
		return;

	if( GetFlags( ) & FL_FROZEN )
		return;

	if( GetMoveType( ) == MOVETYPE_NOCLIP || GetMoveType( ) == MOVETYPE_OBSERVER )
		return;

	bOnLadder = ( GetMoveType( ) == MOVETYPE_LADDER );

	// to not hear step sounds you must be either not moving or not on the ground and not up a ladder
	if( !UTIL_IsMoving( vecVelocity ) || ( ( ( GetFlags( ) & FL_ONGROUND ) == 0 ) && !bOnLadder ) )
		return;

	// find out if we are running
	bool bRunning = ( MaxSpeed( ) >= PLAYER_MAXSPEED_SPRINT );

	// find our knee's
	VectorCopy( vecOrigin, vecKnee );
	VectorCopy( vecOrigin, vecFeet );

	flHeight = GetPlayerMaxs( )[ 2 ] - GetPlayerMins( )[ 2 ];

	vecKnee[ 2 ] = vecOrigin[ 2 ] + 0.2 * flHeight;

	// find out what we're stepping in or on...
	if( bOnLadder )
	{
	#ifdef CLIENT_DLL
		psurface = GetFootstepSurface( vecOrigin, "ladder" );
	#else
		psurface = physprops->GetSurfaceData( physprops->GetSurfaceIndex( "ladder" ) );
	#endif
		flVolume = 0.5;
		m_flStepSoundTime = 600.0f;
	}
	else if( enginetrace->GetPointContents( vecKnee ) & MASK_WATER )
	{
		static int iSkipStep = 0;

		if( iSkipStep == 0 )
		{
			iSkipStep++;
			return;
		}

		if( iSkipStep == 3 )
			iSkipStep = 0;

		iSkipStep++;

		psurface = physprops->GetSurfaceData( physprops->GetSurfaceIndex( "wade" ) );
		flVolume = 0.65f;
		m_flStepSoundTime = 1050.0f;
	}
	else if( enginetrace->GetPointContents( vecFeet ) & MASK_WATER )
	{
		psurface = physprops->GetSurfaceData( physprops->GetSurfaceIndex( "water" ) );
		flVolume = bRunning ? 0.7f : 0.3f;
		m_flStepSoundTime = bRunning ? 350.0f : 700.0f;
	}
	else
	{
		if( !psurface )
			return;

		m_flStepSoundTime = bRunning ? 350.0f : 700.0f;

		switch( psurface->game.material )
		{
			default:
			case CHAR_TEX_CONCRETE:						
				flVolume = bRunning ? 0.5f : 0.2f;
				break;

			case CHAR_TEX_METAL:	
				flVolume = bRunning ? 0.5f : 0.2f;
				break;

			case CHAR_TEX_DIRT:
				flVolume = bRunning ? 0.55f : 0.25f;
				break;

			case CHAR_TEX_VENT:	
				flVolume = bRunning ? 0.7f : 0.4f;
				break;

			case CHAR_TEX_GRATE:
				flVolume = bRunning ? 0.5f : 0.2f;
				break;

			case CHAR_TEX_TILE:	
				flVolume = bRunning ? 0.5f : 0.2f;
				break;

			case CHAR_TEX_SLOSH:
				flVolume = bRunning ? 0.5f : 0.2f;
				break;
		}
	}

	bool bNotStanding = !IsStanding( );

	// slow down if on a ladder or we're not standing
	m_flStepSoundTime += ( ( bOnLadder || bNotStanding ) ? 175.0f : 0.0f );

	// dampen sound if not sounding, in ironsights or shuffling
	if( bNotStanding || ( GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) || IsWalking( ) )
		flVolume *= 0.65f;

	PlayStepSound( vecFeet, psurface, flVolume, false );
}

//=========================================================
//=========================================================
const Vector CINSPlayer::GetPlayerMins( void ) const
{
	return UTIL_PlayerViewMins( this );
}

//=========================================================
//=========================================================
const Vector CINSPlayer::GetPlayerMaxs( void ) const
{
	return UTIL_PlayerViewMaxs( this );
}

//=========================================================
//=========================================================
CPlayerModelData *CINSPlayer::GetPlayerModelData( void ) const
{
	CPlayerModelData *pTeamModelData = GetTeamLookup( )->GetModelData( );
	Assert( pTeamModelData );

	if( !OnPlayTeam( ) )
		return pTeamModelData;

	CPlayerClass *pClass = GetClass( );
	Assert( pClass );

	if( !pClass )
		return pTeamModelData;

	CPlayerModelData *pClassModelData = pClass->GetModelData( );

	if( !pClassModelData )
		return pTeamModelData;

	return pClassModelData;
}

//=========================================================
//=========================================================
void CINSPlayer::Jumped( void )
{
	if( m_flStartBunnyHopTime == 0.0f )
		m_flStartBunnyHopTime = gpGlobals->curtime;

	m_flBunnyHopLength = min( m_flBunnyHopLength + PLAYER_JUMPTIME, PLAYER_JUMPTIME_MAX );

	Weapon_ToShouldered( false );

	m_iStamina = max( 0, m_iStamina - STAMINA_JUMP_COST );

	DoAnimationEvent( PLAYERANIMEVENT_PLAY_JUMP );
}

//=========================================================
//=========================================================
bool CINSPlayer::IsValidAction( int iID )
{
	return ( iID >= 0 && iID < PACTION_COUNT );
}

//=========================================================
//=========================================================
bool CINSPlayer::GetStatus( int &iType, int &iID )
{
	iType = GetStatusType( );
	iID = GetStatusID( );

	return ( iType != INVALID_PSTATUSTYPE && iID != INVALID_PSTATUSID );
}

//=========================================================
//=========================================================
void CINSPlayer::SetIronsightsState( bool bState )
{
	if( bState )
	{
		AddPlayerFlag( FL_PLAYER_IRONSIGHTS );
		m_flBreathTime = gpGlobals->curtime;
	}
	else
	{
		RemovePlayerFlag( FL_PLAYER_IRONSIGHTS );
		m_flBreathTime = 0.0f;
	}
}

//=========================================================
//=========================================================
void CINSPlayer::AddPlayerFlag( int iFlags )
{
	m_iPlayerFlags |= iFlags;
}

//=========================================================
//=========================================================
void CINSPlayer::RemovePlayerFlag( int iFlags )
{
	m_iPlayerFlags &= ~iFlags;
}

//=========================================================
//=========================================================
int CINSPlayer::GetPlayerFlags( void ) const
{
	return m_iPlayerFlags;
}

//=========================================================
//=========================================================
bool CINSPlayer::IsMoveFrozen( void ) const
{
	if( m_iPlayerFlags & FL_PLAYER_BIPOD && m_iCurrentStance != STANCE_PRONE )
		return true;

	CWeaponINSBase *pWeapon = GetActiveINSWeapon( );
	return ( pWeapon && !pWeapon->AllowPlayerMovement( ) );
}