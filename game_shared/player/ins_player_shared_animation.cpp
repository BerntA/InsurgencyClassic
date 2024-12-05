//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "effect_dispatch_data.h"
#include "engine/ivdebugoverlay.h"
#include "ins_player_shared.h"

#ifdef CLIENT_DLL
#include "c_basetempentity.h"
#include "c_playerresource.h"
#define CBaseTempEntity C_BaseTempEntity
#endif

//=========================================================
//=========================================================
class CTEPlayerAnimEvent : public CBaseTempEntity
{
	DECLARE_CLASS(CTEPlayerAnimEvent, CBaseTempEntity);

public:

#ifdef GAME_DLL

	DECLARE_SERVERCLASS();
	
	CTEPlayerAnimEvent(const char *pszName)
		: CBaseTempEntity(pszName)
	{
	}

#else

	DECLARE_CLIENTCLASS();
	
	void PostDataUpdate(DataUpdateType_t updateType)
	{
		C_INSPlayer *pPlayer = m_hPlayer.Get();

		if(pPlayer && !pPlayer->IsDormant())
			pPlayer->DoAnimationEvent((PlayerAnimEvent_e)m_iEvent.Get());
	}

#endif

public:
	CNetworkVar(CINSPlayerHandle, m_hPlayer);
	CNetworkVar(int, m_iEvent);
};

//=========================================================
//=========================================================
#ifdef GAME_DLL

static CTEPlayerAnimEvent g_TEPlayerAnimEvent("PlayerAnimEvent");

IMPLEMENT_SERVERCLASS_ST_NOBASE(CTEPlayerAnimEvent, DT_TEPlayerAnimEvent)

	SendPropEHandle(SENDINFO(m_hPlayer)),
	SendPropInt(SENDINFO(m_iEvent), Q_log2(PLAYERANIMEVENT_COUNT) + 1, SPROP_UNSIGNED),

END_SEND_TABLE()

void TE_PlayerAnimEvent(CINSPlayer *pPlayer, PlayerAnimEvent_e event)
{
	CPVSFilter filter((const Vector&)pPlayer->EyePosition());

	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.Create(filter, 0);
}

#else

IMPLEMENT_CLIENTCLASS_EVENT(CTEPlayerAnimEvent, DT_TEPlayerAnimEvent, CTEPlayerAnimEvent);

BEGIN_RECV_TABLE_NOBASE(CTEPlayerAnimEvent, DT_TEPlayerAnimEvent)

	RecvPropEHandle(RECVINFO(m_hPlayer)),
	RecvPropInt(RECVINFO(m_iEvent))

END_RECV_TABLE()

#endif

//=========================================================
//=========================================================
CBaseCombatWeapon *CINSPlayer::INSAnim_GetActiveWeapon( void )
{
	return m_hActiveWeapon;
}

//=========================================================
//=========================================================
int CINSPlayer::INSAnim_GetPlayerFlags( void )
{
	return m_iPlayerFlags;
}

//=========================================================
//=========================================================
bool CINSPlayer::INSAnim_InStanceTransition( void )
{
	return InStanceTransition( );
}

//=========================================================
//=========================================================
int CINSPlayer::INSAnim_CurrentStance( void )
{
	return m_iCurrentStance;
}

//=========================================================
//=========================================================
int CINSPlayer::INSAnim_LastStance( void )
{
	return m_iLastStance;
}

//=========================================================
//=========================================================
int CINSPlayer::INSAnim_OldStance( void )
{
	return m_iOldStance;
}

//=========================================================
//=========================================================
int CINSPlayer::INSAnim_LeaningType( void )
{
	return m_iLeanType;
}

//=========================================================
//=========================================================
void CINSPlayer::DoAnimationEvent( PlayerAnimEvent_e event )
{
    m_pPlayerAnimState->DoAnimationEvent( event );

#ifdef GAME_DLL

    TE_PlayerAnimEvent( this, event );

#endif
}


//=========================================================
//=========================================================

// Below this many degrees, slow down turning rate linearly
#define FADE_TURN_DEGREES	45.0f
// After this, need to start turning feet
#define MAX_TORSO_ANGLE		90.0f
// Below this amount, don't play a turning animation/perform IK
#define MIN_TURN_ANGLE_REQUIRING_TURN_ANIMATION		15.0f


extern ConVar sv_backspeed;
extern ConVar mp_feetyawrate;
extern ConVar mp_facefronttime;
extern ConVar mp_ik;

CPlayerAnimState::CPlayerAnimState( CINSPlayer *outer )
	: m_pOuter( outer )
{
	m_flGaitYaw = 0.0f;
	m_flGoalFeetYaw = 0.0f;
	m_flCurrentFeetYaw = 0.0f;
	m_flCurrentTorsoYaw = 0.0f;
	m_flLastYaw = 0.0f;
	m_flLastTurnTime = 0.0f;
	m_flTurnCorrectionTime = 0.0f;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerAnimState::Update()
{
	m_angRender = GetOuter()->GetLocalAngles();

	ComputePoseParam_BodyYaw();
	ComputePoseParam_BodyPitch();
	ComputePoseParam_BodyLookYaw();

	ComputePlaybackRate();

#ifdef CLIENT_DLL
	GetOuter()->UpdateLookAt();
#endif

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerAnimState::ComputePlaybackRate()
{
	// Determine ideal playback rate
	Vector vel;
	GetOuterAbsVelocity( vel );

	float speed = vel.Length2D();

	bool isMoving = ( speed > 0.5f ) ? true : false;

	float maxspeed = GetOuter()->GetSequenceGroundSpeed( GetOuter()->GetSequence() );
	
	if ( isMoving && ( maxspeed > 0.0f ) )
	{
		float flFactor = 1.0f;

		// Note this gets set back to 1.0 if sequence changes due to ResetSequenceInfo below
		GetOuter()->SetPlaybackRate( ( speed * flFactor ) / maxspeed );

		// BUG BUG:
		// This stuff really should be m_flPlaybackRate = speed / m_flGroundSpeed
	}
	else
	{
		GetOuter()->SetPlaybackRate( 1.0f );
	}
}

// xENO [
//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CPlayerAnimState::CalcMovementPlaybackRate()
{
    bool   isMoving;
    float  retval;
    float  speed;
    float  maxspeed;
    Vector velocity;

    GetOuterAbsVelocity( velocity );
    speed = velocity.Length2D();
	isMoving = ( speed > 0.5f ) ? true : false;
	maxspeed = GetOuter()->GetSequenceGroundSpeed( GetOuter()->GetSequence() );
	
    retval = 1.0f;

	if ( isMoving && ( maxspeed > 0.0f ) )
	{
		retval = ( ( speed * 1.0f ) / maxspeed );
	}

    return retval;
}
// ] xENO

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CBasePlayer
//-----------------------------------------------------------------------------
CINSPlayer *CPlayerAnimState::GetOuter()
{
	return m_pOuter;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : dt - 
//-----------------------------------------------------------------------------
void CPlayerAnimState::EstimateYaw( void )
{
	float dt = gpGlobals->frametime;

	if ( !dt )
	{
		return;
	}

	Vector est_velocity;
	QAngle	angles;

	GetOuterAbsVelocity( est_velocity );

	angles = GetOuter()->GetLocalAngles();

	if ( est_velocity[1] == 0 && est_velocity[0] == 0 )
	{
		float flYawDiff = angles[YAW] - m_flGaitYaw;
		flYawDiff = flYawDiff - (int)(flYawDiff / 360) * 360;
		if (flYawDiff > 180)
			flYawDiff -= 360;
		if (flYawDiff < -180)
			flYawDiff += 360;

		if (dt < 0.25)
			flYawDiff *= dt * 4;
		else
			flYawDiff *= dt;

		m_flGaitYaw += flYawDiff;
		m_flGaitYaw = m_flGaitYaw - (int)(m_flGaitYaw / 360) * 360;
	}
	else
	{
		m_flGaitYaw = (atan2(est_velocity[1], est_velocity[0]) * 180 / M_PI);

		if (m_flGaitYaw > 180)
			m_flGaitYaw = 180;
		else if (m_flGaitYaw < -180)
			m_flGaitYaw = -180;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override for backpeddling
// Input  : dt - 
//-----------------------------------------------------------------------------
void CPlayerAnimState::ComputePoseParam_BodyYaw( void )
{
    // xENO [
    /*
    int iYaw = GetOuter()->LookupPoseParameter( "move_yaw" );
    if ( iYaw < 0 )
    return;
    */
    // ] xENO

    // view direction relative to movement
    float flYaw;	 

    EstimateYaw();

    QAngle	angles = GetOuter()->GetLocalAngles();
    float ang = angles[ YAW ];
    if ( ang > 180.0f )
    {
        ang -= 360.0f;
    }
    else if ( ang < -180.0f )
    {
        ang += 360.0f;
    }

    // calc side to side turning
    flYaw = ang - m_flGaitYaw;
    // Invert for mapping into 8way blend
    flYaw = -flYaw;
    flYaw = flYaw - (int)(flYaw / 360) * 360;

    if (flYaw < -180)
    {
        flYaw = flYaw + 360;
    }
    else if (flYaw > 180)
    {
        flYaw = flYaw - 360;
    }
    /* xENO [
    GetOuter()->SetPoseParameter( iYaw, flYaw );
       ] xENO */

#ifndef CLIENT_DLL
    //Adrian: Make the model's angle match the legs so the hitboxes match on both sides.
    GetOuter()->SetLocalAngles( QAngle( GetOuter()->GetAnimEyeAngles().x, m_flCurrentFeetYaw, 0 ) );
#endif
    // xENO [

    int iMoveX = GetOuter()->LookupPoseParameter( "move_x" );
    int iMoveY = GetOuter()->LookupPoseParameter( "move_y" );
    if ( iMoveX < 0 || iMoveY < 0 )
        return;

    bool bIsMoving = true;
    float flPlaybackRate = CalcMovementPlaybackRate();

    // Setup the 9-way blend parameters based on our speed and direction.
    Vector2D vCurMovePose( 0, 0 );

    if ( bIsMoving )
    {
        vCurMovePose.x = cos( DEG2RAD( flYaw ) ) * flPlaybackRate;
        vCurMovePose.y = -sin( DEG2RAD( flYaw ) ) * flPlaybackRate;
    }

    GetOuter()->SetPoseParameter( iMoveX, vCurMovePose.x );
    GetOuter()->SetPoseParameter( iMoveY, vCurMovePose.y );

    m_vLastMovePose = vCurMovePose;

    // ] xENO
}

// xENO	[
//-----------------------------------------------------------------------------
// Purpose:	Compute	and	assign the value for the aim_pitch pose	parameter.
void CPlayerAnimState::ComputePoseParam_BodyPitch( void	)
{
    // Get the angles.
    QAngle absangles = GetOuter()->GetLocalAngles();
    float flPitch =	absangles[PITCH];

    // Clamp the pitch value between -90 and 90	degrees
    if ( flPitch > 180.0f )
    {
        flPitch	-= 360.0f;
    }
    flPitch	= clamp( flPitch, -90, 90 );

    // Get the pose	parameter 'aim_pitch'
    int	aim_pitch =	GetOuter()->LookupPoseParameter( "body_pitch" );

    // Make	sure we	have such a	pose parameter.
    Assert(	aim_pitch >= 0 );
    if ( aim_pitch < 0 )
        return;

    // Set pose	parameter 'aim_pitch' to the appropriate pitch value
    GetOuter()->SetPoseParameter( aim_pitch, flPitch );

    // Zero	m_angRender[PITCH],	presumably for the purpose of preventing
    // the player from rotating.  Note:	this does not appear to	work.
    absangles[PITCH] = 0.0f;
    m_angRender	= absangles;
}
//-----------------------------------------------------------------------------
// ] xENO

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : goal - 
//			maxrate - 
//			dt - 
//			current - 
// Output : int
//-----------------------------------------------------------------------------
int CPlayerAnimState::ConvergeAngles( float goal,float maxrate, float dt, float& current )
{
	int direction = TURN_NONE;

	float anglediff = goal - current;
	float anglediffabs = fabs( anglediff );

	anglediff = AngleNormalize( anglediff );

	float scale = 1.0f;
	if ( anglediffabs <= FADE_TURN_DEGREES )
	{
		scale = anglediffabs / FADE_TURN_DEGREES;
		// Always do at least a bit of the turn ( 1% )
		scale = clamp( scale, 0.01f, 1.0f );
	}

	float maxmove = maxrate * dt * scale;

	if ( fabs( anglediff ) < maxmove )
	{
		current = goal;
	}
	else
	{
		if ( anglediff > 0 )
		{
			current += maxmove;
			direction = TURN_LEFT;
		}
		else
		{
			current -= maxmove;
			direction = TURN_RIGHT;
		}
	}

	current = AngleNormalize( current );

	return direction;
}

void CPlayerAnimState::ComputePoseParam_BodyLookYaw( void )
{
	QAngle absangles = GetOuter()->GetAbsAngles();
	absangles.y = AngleNormalize( absangles.y );
	m_angRender = absangles;

	// See if we even have a blender for pitch
	int upper_body_yaw = GetOuter()->LookupPoseParameter( "body_yaw" );
	if ( upper_body_yaw < 0 )
	{
		return;
	}

	// Assume upper and lower bodies are aligned and that we're not turning
	float flGoalTorsoYaw = 0.0f;
	int turning = TURN_NONE;
	float turnrate = 360.0f;

	Vector vel;
	
	GetOuterAbsVelocity( vel );

	bool isMoving = ( vel.Length() > 1.0f ) ? true : false;

	if ( !isMoving )
	{
		// Just stopped moving, try and clamp feet
		if ( m_flLastTurnTime <= 0.0f )
		{
			m_flLastTurnTime	= gpGlobals->curtime;
			m_flLastYaw			= GetOuter()->GetAnimEyeAngles().y;
			// Snap feet to be perfectly aligned with torso/eyes
			m_flGoalFeetYaw		= GetOuter()->GetAnimEyeAngles().y;
			m_flCurrentFeetYaw	= m_flGoalFeetYaw;
			m_nTurningInPlace	= TURN_NONE;
		}

		// If rotating in place, update stasis timer
		if ( m_flLastYaw != GetOuter()->GetAnimEyeAngles().y )
		{
			m_flLastTurnTime	= gpGlobals->curtime;
			m_flLastYaw			= GetOuter()->GetAnimEyeAngles().y;
		}

		if ( m_flGoalFeetYaw != m_flCurrentFeetYaw )
		{
			m_flLastTurnTime	= gpGlobals->curtime;
		}

		turning = ConvergeAngles( m_flGoalFeetYaw, turnrate, gpGlobals->frametime, m_flCurrentFeetYaw );

		QAngle eyeAngles = GetOuter()->GetAnimEyeAngles();
		QAngle vAngle = GetOuter()->GetLocalAngles();

		// See how far off current feetyaw is from true yaw
		float yawdelta = GetOuter()->GetAnimEyeAngles().y - m_flCurrentFeetYaw;
		yawdelta = AngleNormalize( yawdelta );

		bool rotated_too_far = false;

		float yawmagnitude = fabs( yawdelta );

		// If too far, then need to turn in place
		if ( yawmagnitude > 45 )
		{
			rotated_too_far = true;
		}

		// Standing still for a while, rotate feet around to face forward
		// Or rotated too far
		// FIXME:  Play an in place turning animation
		if ( rotated_too_far || 
			( gpGlobals->curtime > m_flLastTurnTime + mp_facefronttime.GetFloat() ) )
		{
			m_flGoalFeetYaw		= GetOuter()->GetAnimEyeAngles().y;
			m_flLastTurnTime	= gpGlobals->curtime;

		/*	float yd = m_flCurrentFeetYaw - m_flGoalFeetYaw;
			if ( yd > 0 )
			{
				m_nTurningInPlace = TURN_RIGHT;
			}
			else if ( yd < 0 )
			{
				m_nTurningInPlace = TURN_LEFT;
			}
			else
			{
				m_nTurningInPlace = TURN_NONE;
			}

			turning = ConvergeAngles( m_flGoalFeetYaw, turnrate, gpGlobals->frametime, m_flCurrentFeetYaw );
			yawdelta = GetOuter()->GetAnimEyeAngles().y - m_flCurrentFeetYaw;*/

		}

		// Snap upper body into position since the delta is already smoothed for the feet
		flGoalTorsoYaw = yawdelta;
		m_flCurrentTorsoYaw = flGoalTorsoYaw;
	}
	else
	{
		m_flLastTurnTime = 0.0f;
		m_nTurningInPlace = TURN_NONE;
		m_flCurrentFeetYaw = m_flGoalFeetYaw = GetOuter()->GetAnimEyeAngles().y;
		flGoalTorsoYaw = 0.0f;
		m_flCurrentTorsoYaw = GetOuter()->GetAnimEyeAngles().y - m_flCurrentFeetYaw;
	}


	if ( turning == TURN_NONE )
	{
		m_nTurningInPlace = turning;
	}

	if ( m_nTurningInPlace != TURN_NONE )
	{
		// If we're close to finishing the turn, then turn off the turning animation
		if ( fabs( m_flCurrentFeetYaw - m_flGoalFeetYaw ) < MIN_TURN_ANGLE_REQUIRING_TURN_ANIMATION )
		{
			m_nTurningInPlace = TURN_NONE;
		}
	}

	// Rotate entire body into position
	absangles = GetOuter()->GetAbsAngles();
	absangles.y = m_flCurrentFeetYaw;
	m_angRender = absangles;

	GetOuter()->SetPoseParameter( upper_body_yaw, clamp( m_flCurrentTorsoYaw, -60.0f, 60.0f ) );

	/*
	// FIXME: Adrian, what is this?
	int body_yaw = GetOuter()->LookupPoseParameter( "body_yaw" );

	if ( body_yaw >= 0 )
	{
		GetOuter()->SetPoseParameter( body_yaw, 30 );
	}
	*/

}


 
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : activity - 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CPlayerAnimState::BodyYawTranslateActivity( Activity activity )
{
	// Not even standing still, sigh
	if ( activity != ACT_IDLE )
		return activity;

	// Not turning
	switch ( m_nTurningInPlace )
	{
	default:
	case TURN_NONE:
		return activity;
	/*
	case TURN_RIGHT:
		return ACT_TURNRIGHT45;
	case TURN_LEFT:
		return ACT_TURNLEFT45;
	*/
	case TURN_RIGHT:
	case TURN_LEFT:
		return mp_ik.GetBool() ? ACT_TURN : activity;
	}

	Assert( 0 );
	return activity;
}

const QAngle& CPlayerAnimState::GetRenderAngles()
{
	return m_angRender;
}


void CPlayerAnimState::GetOuterAbsVelocity( Vector& vel )
{
#if defined( CLIENT_DLL )
	GetOuter()->EstimateAbsVelocity( vel );
#else
	vel = GetOuter()->GetAbsVelocity();
#endif
}