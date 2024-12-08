//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "gamemovement.h"
#include "in_buttons.h"
#include "movevars_shared.h"
#include "ins_player_shared.h"
#include "ins_utils.h"
#include "ins_gamerules.h"
#include "ins_spawnprotection_shared.h"
#include "weapon_ins_base.h"

#ifdef CLIENT_DLL

#include "iprediction.h"
#include "prediction.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
class CINSGameMovement : public CGameMovement
{
	DECLARE_CLASS( CINSGameMovement, CGameMovement );

public:
	CINSGameMovement( );

private:
	void ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMove );

	Vector GetPlayerViewOffset( int iStance ) const;
	const Vector &GetPlayerMins( void ) const;
	const Vector &GetPlayerMaxs( void ) const;

	bool CheckJumpButton( void );

	void StanceTransition( void );
	bool CanChangeStance( int iFromStance, int iToStance );
	void SetTransitionEyeOffset( float flTransitionFraction, const Vector &vecCurrentViewOffset, const Vector &vecNextViewOffset );

	bool IsMovementFrozen( void ) const;
	bool IsSideMoveAllowed( void ) const;

	void DoFallEffects( void );
	void ClampWalkVelocity( void );

	float GetStopSpeed( void );

	void HandleSpeedCrop( void );
	float GetSpeedFraction( void );

	void DecayPunchAngle( void );
	void DecayPunchAngle( float flDamping, QAngle &angAngle, QAngle &angAngleVel );

	bool CanAccelerate( void );

	void FullWalkMove( void );

	void ProneMove( void );
	void PlayerMove( void );

private:
	CINSPlayer *m_pINSPlayer;
};

//=========================================================
//=========================================================
static CINSGameMovement g_GameMovement;
IGameMovement *g_pGameMovement = ( IGameMovement * )&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CGameMovement, IGameMovement, INTERFACENAME_GAMEMOVEMENT, g_GameMovement );

//=========================================================
//=========================================================
CINSGameMovement::CINSGameMovement( )
{
}

//=========================================================
//=========================================================
void CINSGameMovement::ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMove )
{
	m_pINSPlayer = ToINSPlayer( pPlayer );

	BaseClass::ProcessMovement( pPlayer, pMove );
}

//=========================================================
//=========================================================
Vector CINSGameMovement::GetPlayerViewOffset( int iStance ) const
{
	if( player->IsObserver( ) )
		return VEC_OBS_HULL_MIN;	

	return UTIL_PlayerViewOffset( m_pINSPlayer, iStance );
}

//=========================================================
//=========================================================
const Vector& CINSGameMovement::GetPlayerMins( void ) const
{
	return UTIL_PlayerViewMins( m_pINSPlayer );
}

//=========================================================
//=========================================================
const Vector& CINSGameMovement::GetPlayerMaxs( void ) const
{	
	return UTIL_PlayerViewMaxs( m_pINSPlayer );
}

//=========================================================
//=========================================================
bool CINSGameMovement::CheckJumpButton( void )
{
	if( player->pl.deadflag )
	{
		mv->m_nOldButtons |= IN_JUMP;
		return false;
	}

	// see if we are waterjumping - if so, decrement count and return
	if( player->m_flWaterJumpTime )
	{
		player->m_flWaterJumpTime -= gpGlobals->frametime;

		if( player->m_flWaterJumpTime < 0 )
			player->m_flWaterJumpTime = 0;
		
		return false;
	}

	// if we are in the water most of the way...
	if ( player->GetWaterLevel( ) >= 2 )
	{	
		// swimming, not jumping
		SetGroundEntity( ( CBaseEntity * )NULL );

		if( player->GetWaterType( ) == CONTENTS_WATER )    // we move up a certain amount
			mv->m_vecVelocity[ 2 ] = 100;
		else if( player->GetWaterType( ) == CONTENTS_SLIME )
			mv->m_vecVelocity[ 2 ] = 80;
		
		// play swiming sound
		if( player->m_flSwimSoundTime <= 0 )
		{
			// don't play sound again for 1 second
			player->m_flSwimSoundTime = 1000.0f;
			PlaySwimSound( );
		}

		return false;
	}

	// if in air, no more jumping
 	if( player->GetGroundEntity( ) == NULL )
	{
		mv->m_nOldButtons |= IN_JUMP;
		return false;
	}

	// TODO: write a climb system

	// don't pogo stick
	if( mv->m_nOldButtons & IN_JUMP )
		return false;

	// test the player
	if( !m_pINSPlayer->CanJump( ) )
		return false;

	// in the air now
    SetGroundEntity( ( CBaseEntity* )NULL );
	
	player->PlayStepSound( mv->m_vecAbsOrigin, player->m_pSurfaceData, 1.0, true );
	
	MoveHelper( )->PlayerSetAnimation( PLAYER_JUMP );

	float flGroundFactor = 1.0f;

	if( player->m_pSurfaceData )
		flGroundFactor = player->m_pSurfaceData->game.jumpFactor; 

	float flStartZ = mv->m_vecVelocity[ 2 ];
	mv->m_vecVelocity[ 2 ] += flGroundFactor * sqrt( 2 * sv_gravity.GetFloat( ) * PLAYER_JUMP_HEIGHT );

	FinishGravity( );

	mv->m_outJumpVel.z += mv->m_vecVelocity[ 2 ] - flStartZ;
	mv->m_outStepHeight += 0.15f;

	// don't jump again until released
	mv->m_nOldButtons |= IN_JUMP;

	// handle the jump
	m_pINSPlayer->Jumped( );

	return true;
}

//=========================================================
//=========================================================
void CINSGameMovement::StanceTransition( void )
{
	// don't change stances while in transition
	float flViewTransitionLength, flViewTransitionEnd;
	flViewTransitionLength = m_pINSPlayer->ViewTransitionLength( );
	flViewTransitionEnd = m_pINSPlayer->ViewTransitionEnd( );

	if( flViewTransitionEnd != 0.0f && flViewTransitionEnd >= gpGlobals->curtime && flViewTransitionLength != 0.0f )
	{
		Vector vecViewTransitionFrom = Vector( 0, 0, m_pINSPlayer->ViewTransitionFrom( ) );
		Vector vecViewTransitionTarget = Vector( 0, 0, m_pINSPlayer->ViewTransitionTarget( ) );

		float flTransitionFraction = ( flViewTransitionEnd - gpGlobals->curtime ) / flViewTransitionLength;
		SetTransitionEyeOffset( flTransitionFraction, vecViewTransitionFrom, vecViewTransitionTarget );

		return;
	}

	// handle stance transition
	if( m_pINSPlayer->CanStanceTransition( ) )
	{
		int iLastStance, iCurrentStance, iNewStance;

		iLastStance = m_pINSPlayer->GetLastStance( );
		iCurrentStance = m_pINSPlayer->GetCurrentStance( );
		iNewStance = STANCE_INVALID;

		// we cannot change stance when moving in and out of prone
		if( mv->m_nButtons & IN_PRONE && ( mv->m_nOldButtons & IN_PRONE ) == 0 )
		{
			if( iCurrentStance == STANCE_PRONE )
				iNewStance = STANCE_STAND;
			else
				iNewStance = STANCE_PRONE;
		}
		else
		{
			if( m_pINSPlayer->IsSpenceStance( ) )
			{
				// spence stance system is up/down
				if( mv->m_nButtons & IN_JUMP && ( mv->m_nOldButtons & IN_JUMP ) == 0 )
				{
					if( iCurrentStance == STANCE_CROUCH )
						iNewStance = STANCE_STAND;
					else if( iCurrentStance == STANCE_PRONE )
						iNewStance = STANCE_CROUCH;

					mv->m_nOldButtons |= IN_JUMP;
				}
				else if( mv->m_nButtons & IN_CROUCH && ( mv->m_nOldButtons & IN_CROUCH ) == 0 )
				{
					if( iCurrentStance == STANCE_STAND )
						iNewStance = STANCE_CROUCH;
					else if( iCurrentStance == STANCE_CROUCH )
						iNewStance = STANCE_PRONE;

					mv->m_nOldButtons |= IN_CROUCH;
				}
				else
				{
					mv->m_nOldButtons &= ~( IN_JUMP | IN_CROUCH );
				}
			}
			else
			{
				if( mv->m_nButtons & IN_JUMP && ( mv->m_nOldButtons & IN_JUMP ) == 0 && iCurrentStance == STANCE_PRONE )
				{
					iNewStance = STANCE_STAND;

					mv->m_nOldButtons &= ~IN_JUMP;
				}
				else
				{
					if( mv->m_nButtons & IN_CROUCH )
					{
						switch( iCurrentStance )
						{
							case STANCE_STAND:
							{
								iNewStance = STANCE_CROUCH;
								break;
							}

							case STANCE_PRONE:
							{
								if( ( mv->m_nOldButtons & IN_CROUCH ) == 0 )
									iNewStance = STANCE_CROUCH;

								break;
							}
						}
					}
					else if( iCurrentStance == STANCE_CROUCH )
					{
						iNewStance = STANCE_STAND;
					}
				}
			}
		}

		if( iNewStance != STANCE_INVALID && CanChangeStance( iCurrentStance, iNewStance ) )
		{
			if( iCurrentStance == STANCE_PRONE )
				m_pINSPlayer->DoProneViewEffect( true );

			m_pINSPlayer->SetDesiredStance( iNewStance );
		}
	}

	// handle any transitions
	bool bViewSetOffset = true;

	if( m_pINSPlayer->InStanceTransition( ) )
	{
		float flTransitionTime = 0.0f;

		int iLastStance, iCurrentStance;
		iLastStance = m_pINSPlayer->GetLastStance( );
		iCurrentStance = m_pINSPlayer->GetCurrentStance( );

		// get the transition time
		flTransitionTime = UTIL_StanceTransitionTime( iLastStance, iCurrentStance );

		// if we are past our marker, set a new current stance
		m_pINSPlayer->HandleTransitionTime( flTransitionTime );
		iLastStance = m_pINSPlayer->GetLastStance( );

		float flStanceTransitionMarker = m_pINSPlayer->GetStanceTransitionMarker( );

		// do the transition
		if( flStanceTransitionMarker > 0.0f )
		{
			// when we're going into or out of prone, make the view shake
			if( ( iLastStance == STANCE_PRONE ) || ( iCurrentStance == STANCE_PRONE && iLastStance == STANCE_CROUCH ) )
			{
				static QAngle angProneJerk = QAngle( random->RandomFloat( -16.0, 16.0 ), random->RandomFloat( -16.0, 16.0 ), random->RandomFloat( -32.0, 32.0 ) );
				m_pINSPlayer->RecoilViewPunch( gpGlobals->frametime * angProneJerk );
			}

			// do eye transition
			SetTransitionEyeOffset( 1.0f - max( 0.0f, ( gpGlobals->curtime - flStanceTransitionMarker ) / flTransitionTime ), GetPlayerViewOffset( iLastStance ), GetPlayerViewOffset( UTIL_GetNextStance( iLastStance, iCurrentStance ) ) );
			bViewSetOffset = false;
		}
	}

	if( bViewSetOffset )
		player->SetViewOffset( m_pINSPlayer->GetCurrentViewOffset( ) );
}

//=========================================================
//=========================================================
bool CINSGameMovement::CanChangeStance( int iFromStance, int iToStance )
{
	// ask the player
	if( !m_pINSPlayer->CanChangeStance( iFromStance, iToStance ) )
		return false;

	// cannot change while transiting in and out of prone
	if( m_pINSPlayer->InStanceTransition( ) && ( iFromStance == STANCE_PRONE || iToStance == STANCE_PRONE ) )
		return false;

	// can't look at the sky when going prone
	if( iToStance == STANCE_PRONE )
	{
		const QAngle &angViewAngles = m_pINSPlayer->GetLocalAngles( );

		if( angViewAngles[ PITCH ] < -10.0f )
			return false;
	}

	bool bOnGround = ( player->GetFlags( ) & FL_ONGROUND ) ? true : false;

	// can't change to or from prone while falling
	if( !bOnGround && ( iFromStance == STANCE_PRONE || iToStance == STANCE_PRONE ) )
		return false;

	// there will never be any problem going from stand to crouch
	if( iFromStance == STANCE_STAND && iToStance == STANCE_CROUCH )
		return true;

	Vector vecStartOrigin, vecNewOrigin, vecHullMin, vecHullMax;
	VectorCopy( mv->m_vecAbsOrigin, vecStartOrigin );
	VectorCopy( mv->m_vecAbsOrigin, vecNewOrigin );

	if( bOnGround )
	{
		Vector vecFrom, vecTo;

		// NOTE: assume lower stances have bigger hulls
		vecHullMin = UTIL_PlayerViewMins( iToStance );
		vecHullMax = UTIL_PlayerViewMaxs( iToStance );

		vecHullMin[ 2 ] = 0.0f;
		vecHullMax[ 2 ] = 0.0f;
					
		vecFrom = UTIL_PlayerViewMaxs( iFromStance );
		vecStartOrigin[ 2 ] += vecFrom[ 2 ];

		if( iToStance > iFromStance )
			vecTo = UTIL_PlayerViewMins( iToStance );
		else
			vecTo = UTIL_PlayerViewMaxs( iToStance );			

		vecNewOrigin[ 2 ] += vecTo[ 2 ];
	}
	else
	{
		// if in air and letting go of crouch, make sure we can offset origin to make
		// up for uncrouching
		Assert( iToStance == STANCE_STAND );

		vecHullMin = VEC_HULL_MIN;
		vecHullMax = VEC_HULL_MAX;

		Vector vecHullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
		Vector vecHullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;
		Vector vecViewDelta = ( vecHullSizeNormal - vecHullSizeCrouch );
		vecViewDelta.Negate( );

		VectorAdd( vecNewOrigin, vecViewDelta, vecNewOrigin );
	}

	trace_t trace;
	Ray_t ray;

	ray.Init( vecStartOrigin, vecNewOrigin, vecHullMin, vecHullMax );
	UTIL_TraceRay( ray, PlayerSolidMask( ), mv->m_nPlayerHandle.Get( ), COLLISION_GROUP_PLAYER_MOVEMENT, &trace );

#if defined( _DEBUG ) && defined( GAME_DLL )

	NDebugOverlay::Box( vecStartOrigin, Vector( 4, 4, 4 ), Vector( -4, -4, -4 ), 255, 0, 0, 128, 5.0f );
	NDebugOverlay::Box( vecNewOrigin, Vector( 4, 4, 4 ), Vector( -4, -4, -4 ), 0, 0, 255, 128, 5.0f );

#endif

	return ( ( trace.fraction == 1.0f ) && !trace.startsolid );
}

//=========================================================
//=========================================================
void CINSGameMovement::SetTransitionEyeOffset( float flTransitionFraction, const Vector &vecCurrentViewOffset, const Vector &vecNextViewOffset )
{
	player->SetViewOffset( ( vecCurrentViewOffset * flTransitionFraction ) + ( vecNextViewOffset * ( 1.0f - flTransitionFraction ) ) );
}

//=========================================================
//=========================================================
bool CINSGameMovement::IsMovementFrozen( void ) const
{
	if( BaseClass::IsMovementFrozen( ) )
		return true;

	if( !INSRules( )->IsModeRunning( ) || INSRules( )->RunningMode( )->IsFrozen( ) )
		return true;

	return false;
}

//=========================================================
//=========================================================
bool CINSGameMovement::IsSideMoveAllowed( void ) const
{
	return !m_pINSPlayer->IsSprinting( );
}

//=========================================================
//=========================================================
void CINSGameMovement::DoFallEffects( void )
{
#ifdef CLIENT_DLL

	if( !prediction->InPrediction( ) || !prediction->IsFirstTimePredicted( ) )
		return;

#endif

	// knock around viewmodel
	player->m_Local.m_vecPunchAngle.Set( ROLL, player->m_Local.m_flFallVelocity * 0.013f );

	if ( player->m_Local.m_vecPunchAngle[ PITCH ] > 8 )
		player->m_Local.m_vecPunchAngle.Set( PITCH, 8 );

	// knock around other view
	float flViewPitchAdjust = ( player->m_Local.m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED ) ? 40.0f : 25.0f;
	player->RecoilViewPunch( QAngle( flViewPitchAdjust, flViewPitchAdjust * 0.25f, 0 ) );
}

//=========================================================
//=========================================================
void CINSGameMovement::ClampWalkVelocity( void )
{
	CSpawnProtection *pSpawnProtection = m_pINSPlayer->GetClippingEntity( );

	if( pSpawnProtection )
		pSpawnProtection->ClampMoveVelocity( m_pINSPlayer, mv->m_vecVelocity );
}

//=========================================================
//=========================================================
float CINSGameMovement::GetStopSpeed( void )
{
	return ( m_pINSPlayer->IsStanding( ) ? sv_stopspeed.GetFloat( ) : sv_stopspeed_prone.GetFloat( ) );
}

//=========================================================
//=========================================================
void CINSGameMovement::HandleSpeedCrop( void )
{
	if( m_bSpeedCropped )
		return;

	float flFraction = GetSpeedFraction( );

	if( flFraction != 1.0f )
	{
		mv->m_flForwardMove *= flFraction;
		mv->m_flSideMove *= flFraction;
		mv->m_flUpMove *= flFraction;
		m_bSpeedCropped = true;
	}
}

//=========================================================
//=========================================================
float CINSGameMovement::GetSpeedFraction( void )
{
	// TODO: acceleration buggers this up

	if( player->GetGroundEntity( ) == NULL )
		return 1.0f;

	float flFraction = 1.0f;
	int iCurrentStance = m_pINSPlayer->GetCurrentStance( );

	if( m_pINSPlayer->IsMoveFrozen( ) )
	{
		flFraction = 0.0f;
	}
	else
	{
		bool bIronsights = ( ( m_pINSPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) != 0 ) ? true : false;

		switch( iCurrentStance )
		{
			case STANCE_STAND:
			{
				if( m_pINSPlayer->IsSprinting( ) )
				{
					if( !m_pINSPlayer->IsBrustSprint( ) )
					{
						static float flMinFraction = PLAYER_MAXSPEED_SPRINT / ( float )PLAYER_MAXSPEED_BURST;
						static float flMinFractionStep = 1.0f - flMinFraction;

						flFraction = flMinFraction + ( flMinFractionStep * ( ( float )m_pINSPlayer->GetStamina( ) / STAMINA_NEEDED_BURST ) );
					}
				}
				else
				{
					if( bIronsights || m_pINSPlayer->m_flBunnyHopLength != 0.0f )
					{
						flFraction = PLAYER_FRACSPEED_STAND_IRONSIGHT;
					}
					else if( m_pINSPlayer->GetPlayerFlags( ) & FL_PLAYER_WALKING )
					{
						flFraction = PLAYER_FRACSPEED_STAND_SHUFFLE;
					}
					else if( m_pINSPlayer->IsReloading( ) )
					{
						CWeaponINSBase *pWeapon = m_pINSPlayer->GetActiveINSWeapon( );

						if( pWeapon && pWeapon->GetWeaponClass( ) != WEAPONCLASS_PISTOL )
							flFraction = PLAYER_FRACSPEED_STAND_RELOADING;
					}
				}

				int iDamageDecay = m_pINSPlayer->GetDamageDecay( );

				if( iDamageDecay > 0 )
				{
					static float flRemainDecay = 1.0f - PLAYER_FRACSPEED_DMGDECAY;
					flFraction *= PLAYER_FRACSPEED_DMGDECAY + ( ( 1.0f - ( ( float )iDamageDecay / MAX_DAMAGEDECAY ) ) * flRemainDecay );
				}

				break;
			}

			case STANCE_CROUCH:
			{
				if( bIronsights )
					flFraction = PLAYER_FRACSPEED_CROUCHED_IRONSIGHT;
				else
					flFraction = PLAYER_FRACSPEED_CROUCHED;

				break;
			}

			case STANCE_PRONE:
			{
				flFraction = PLAYER_FRACSPEED_PRONE;

				break;
			}
		}

		if( iCurrentStance != STANCE_PRONE )
		{
			flFraction *= m_pINSPlayer->m_INSLocal.m_flWeightFactor;
		}
	}

	return flFraction;
}

//=========================================================
//=========================================================
#define PUNCH_DAMPING		9.0f		// bigger number makes the response more damped, smaller is less damped
										// currently the system will overshoot, with larger damping values it won't
#define PUNCH_SPRING_CONSTANT	65.0f	// bigger number increases the speed at which the view corrects

//=========================================================
//=========================================================
void CINSGameMovement::DecayPunchAngle( void )
{
    float flDamping = 1.0f - ( PUNCH_DAMPING * gpGlobals->frametime );

    if( flDamping < 0.0f )
        flDamping = 0.0f;

	DecayPunchAngle( flDamping, player->m_Local.m_vecRecoilPunchAngle.GetForModify( ), player->m_Local.m_vecRecoilPunchAngleVel.GetForModify( ) );
	DecayPunchAngle( flDamping, player->m_Local.m_vecPunchAngle.GetForModify( ), player->m_Local.m_vecPunchAngleVel.GetForModify( ) );
}

//=========================================================
//=========================================================
void CINSGameMovement::DecayPunchAngle( float flDamping, QAngle &angAngle, QAngle &angAngleVel )
{
    if( angAngle.LengthSqr( ) <= 0.001f && angAngleVel.LengthSqr( ) <= 0.001f )
		return;

	angAngle += angAngleVel * gpGlobals->frametime;
    angAngleVel *= flDamping;

	// torsional spring
    // UNDONE: Per-axis spring constant?
    float flSpringForceMagnitude = PUNCH_SPRING_CONSTANT * gpGlobals->frametime;
    flSpringForceMagnitude = clamp( flSpringForceMagnitude, 0.0f, 2.0f );
    angAngleVel -= angAngle * flSpringForceMagnitude;

    // don't wrap around
    angAngle.Init( 
		clamp( angAngle.x, -89.0f, 89.0f ), 
		clamp( angAngle.y, -179.0f, 179.0f ),
		clamp( angAngle.z, -89.0f, 89.0f ) );
}

//=========================================================
//=========================================================
bool CINSGameMovement::CanAccelerate( void )
{
	if( player->IsObserver( ) )
		return true;

	return BaseClass::CanAccelerate( );
}

//=========================================================
//=========================================================
void CINSGameMovement::PlayerMove( void )
{
	BaseClass::PlayerMove( );

	/*if( m_pINSPlayer->IsProned( ) && !m_pINSPlayer->InStanceTransition( ) )
	{
		ProneMove( );
	}
	else
	{
		mv->m_vecAngles[ PITCH ] = 0;
		mv->m_vecAngles[ ROLL ] = 0;
	}*/
}

//=========================================================
//=========================================================
void CINSGameMovement::ProneMove( void )
{
	trace_t tr;
	Vector vecStart = m_pINSPlayer->GetAbsOrigin( );
	Vector vecEnd( vecStart );
	vecEnd.z-=50;
	CTraceFilterSimple traceFilter(m_pINSPlayer,COLLISION_GROUP_NONE);

	UTIL_TraceLine(vecStart,vecEnd,MASK_PLAYERSOLID,&traceFilter,&tr);

	if (tr.DidHit())
	{
		QAngle angPlayer=QAngle(0,mv->m_vecAngles[YAW],0),angNorm;

#define ang2rad (2 * M_PI / 360)
		Vector angdir = Vector(
			cos(angPlayer.y * ang2rad) * cos(angPlayer.x * ang2rad),
			sin(angPlayer.y * ang2rad) * cos(angPlayer.x * ang2rad),
			-sin(angPlayer.x * ang2rad));
		Vector angdiry = Vector(
			sin(angPlayer.y * ang2rad) * cos(angPlayer.x * ang2rad),
			cos(angPlayer.y * ang2rad) * cos(angPlayer.x * ang2rad),
			-sin(angPlayer.x * ang2rad));

		VectorAngles(angdir - DotProduct(angdir, tr.plane.normal) * tr.plane.normal, angPlayer);
		VectorAngles(angdiry - DotProduct(angdiry, tr.plane.normal) * tr.plane.normal, angNorm);
		angPlayer.z=-angNorm.x;

		mv->m_vecAngles=angPlayer;
	}
	else
	{
		mv->m_vecAngles[PITCH]=0;
		mv->m_vecAngles[ROLL]=0;
	}
}

//=========================================================
//=========================================================
void CINSGameMovement::FullWalkMove( void )
{
	StanceTransition( );
	HandleSpeedCrop( );

	BaseClass::FullWalkMove( );
}
