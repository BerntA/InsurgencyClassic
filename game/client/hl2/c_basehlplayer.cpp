//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basehlplayer.h"
#include "playerandobjectenumerator.h"
#include "engine/ivdebugoverlay.h"
#include "in_buttons.h"
#include "collisionutils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// How fast to avoid collisions with center of other object, in units per second
#define AVOID_SPEED 2000.0f

extern ConVar cl_forwardspeed;
extern ConVar cl_backspeed;
extern ConVar cl_sidespeed;
extern ConVar default_fov;

IMPLEMENT_CLIENTCLASS_DT(C_BaseHLPlayer, DT_HL2_Player, CHL2_Player)
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_BaseHLPlayer )
END_PREDICTION_DATA()

//-----------------------------------------------------------------------------
// Purpose: Drops player's primary weapon
//-----------------------------------------------------------------------------
void CC_DropPrimary( void )
{
	C_BasePlayer *pPlayer = (C_BasePlayer *) C_BasePlayer::GetLocalPlayer();
	
	if ( pPlayer == NULL )
		return;

	pPlayer->Weapon_DropPrimary();
}

static ConCommand dropprimary("dropprimary", CC_DropPrimary, "dropprimary: Drops the primary weapon of the player.");

#if !defined (HL2MP)
LINK_ENTITY_TO_CLASS(player, C_BaseHLPlayer);
#endif

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
C_BaseHLPlayer::C_BaseHLPlayer()
{
	AddVar( &m_Local.m_vecPunchAngle, &m_Local.m_iv_vecPunchAngle, LATCH_SIMULATION_VAR );
	AddVar( &m_Local.m_vecPunchAngleVel, &m_Local.m_iv_vecPunchAngleVel, LATCH_SIMULATION_VAR );

	m_flZoomStart		= 0.0f;
	m_flZoomEnd			= 0.0f;
	m_flZoomRate		= 0.0f;
	m_flZoomStartTime	= 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::Weapon_DropPrimary( void )
{
	engine->ServerCmd( "DropPrimary" );
}

float C_BaseHLPlayer::GetFOV()
{
	//Find our FOV with offset zoom value
	float flFOVOffset = BaseClass::GetFOV() + GetZoom();

	// Clamp FOV in MP
	int min_fov = ( gpGlobals->maxClients == 1 ) ? 5 : default_fov.GetInt();
	
	// Don't let it go too low
	flFOVOffset = MAX( min_fov, flFOVOffset );

	return flFOVOffset;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float C_BaseHLPlayer::GetZoom( void )
{
	float fFOV = m_flZoomEnd;

	//See if we need to lerp the values
	if ( ( m_flZoomStart != m_flZoomEnd ) && ( m_flZoomRate > 0.0f ) )
	{
		float deltaTime = (float)( gpGlobals->curtime - m_flZoomStartTime ) / m_flZoomRate;

		if ( deltaTime >= 1.0f )
		{
			//If we're past the zoom time, just take the new value and stop lerping
			fFOV = m_flZoomStart = m_flZoomEnd;
		}
		else
		{
			fFOV = SimpleSplineRemapVal( deltaTime, 0.0f, 1.0f, m_flZoomStart, m_flZoomEnd );
		}
	}

	return fFOV;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : FOVOffset - 
//			time - 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::Zoom( float FOVOffset, float time )
{
	m_flZoomStart		= GetZoom();
	m_flZoomEnd			= FOVOffset;
	m_flZoomRate		= time;
	m_flZoomStartTime	= gpGlobals->curtime;
}


//-----------------------------------------------------------------------------
// Purpose: Hack to zero out player's pitch, use value from poseparameter instead
// Input  : flags - 
// Output : int
//-----------------------------------------------------------------------------
int C_BaseHLPlayer::DrawModel( int flags )
{
	// Not pitch for player
	QAngle saveAngles = GetLocalAngles();

	QAngle useAngles = saveAngles;
	useAngles[ PITCH ] = 0.0f;

	SetLocalAngles( useAngles );

	int iret = BaseClass::DrawModel( flags );

	SetLocalAngles( saveAngles );

	return iret;
}

//-----------------------------------------------------------------------------
// Purpose: Helper to remove from ladder
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::ExitLadder()
{
	if ( MOVETYPE_LADDER != GetMoveType() )
		return;
	
	SetMoveType( MOVETYPE_WALK );
	SetMoveCollide( MOVECOLLIDE_DEFAULT );
}

//-----------------------------------------------------------------------------
// Purpose: Determines if a player can be safely moved towards a point
// Input:   pos - position to test move to, fVertDist - how far to trace downwards to see if the player would fall,
//			radius - how close the player can be to the object, objPos - position of the object to avoid,
//			objDir - direction the object is travelling
//-----------------------------------------------------------------------------
bool C_BaseHLPlayer::TestMove( const Vector &pos, float fVertDist, float radius, const Vector &objPos, const Vector &objDir )
{
	trace_t trUp;
	trace_t trOver;
	trace_t trDown;
	float flHit1, flHit2;
	
	UTIL_TraceHull( GetAbsOrigin(), pos, GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trOver );
	if ( trOver.fraction < 1.0f )
	{
		// check if the endpos intersects with the direction the object is travelling.  if it doesn't, this is a good direction to move.
		if ( objDir.IsZero() ||
			( IntersectInfiniteRayWithSphere( objPos, objDir, trOver.endpos, radius, &flHit1, &flHit2 ) && 
			( ( flHit1 >= 0.0f ) || ( flHit2 >= 0.0f ) ) )
			)
		{
			// our first trace failed, so see if we can go farther if we step up.

			// trace up to see if we have enough room.
			UTIL_TraceHull( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, m_Local.m_flStepSize ), 
				GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trUp );

			// do a trace from the stepped up height
			UTIL_TraceHull( trUp.endpos, pos + Vector( 0, 0, trUp.endpos.z - trUp.startpos.z ), 
				GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trOver );

			if ( trOver.fraction < 1.0f )
			{
				// check if the endpos intersects with the direction the object is travelling.  if it doesn't, this is a good direction to move.
				if ( objDir.IsZero() ||
					( IntersectInfiniteRayWithSphere( objPos, objDir, trOver.endpos, radius, &flHit1, &flHit2 ) && ( ( flHit1 >= 0.0f ) || ( flHit2 >= 0.0f ) ) ) )
				{
					return false;
				}
			}
		}
	}

	// trace down to see if this position is on the ground
	UTIL_TraceLine( trOver.endpos, trOver.endpos - Vector( 0, 0, fVertDist ), 
		MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &trDown );

	if ( trDown.fraction == 1.0f ) 
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Input handling
//-----------------------------------------------------------------------------
bool C_BaseHLPlayer::CreateMove(float flInputSampleTime, CUserCmd *pCmd, bool bFakeInput)
{
	return BaseClass::CreateMove(flInputSampleTime, pCmd, bFakeInput);
}

//-----------------------------------------------------------------------------
// Purpose: Input handling
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::BuildTransformations( CStudioHdr *hdr, Vector *pos, Quaternion q[], const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList &boneComputed )
{
	BaseClass::BuildTransformations( hdr, pos, q, cameraTransform, boneMask, boneComputed );
}