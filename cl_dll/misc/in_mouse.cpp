//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Mouse input routines
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include <windows.h>
#include "cbase.h"
#include "hud.h"
#include "cdll_int.h"
#include "kbutton.h"
#include "usercmd.h"
#include "keydefs.h"
#include "input.h"
#include "iviewrender.h"
#include "iclientmode.h"
#include "vstdlib/icommandline.h"
#include "vgui/isurface.h"
#include "vgui_controls/controls.h"
#include "vgui/cursor.h"
#include "mathlib.h"
#include "c_ins_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//Spence
// up / down
#define	PITCH	0
// left / right
#define	YAW		1

extern ConVar lookstrafe;
extern ConVar cl_pitchdown;
extern ConVar cl_pitchup;

ConVar m_pitch( "m_pitch","0.022", FCVAR_ARCHIVE, "Mouse pitch factor." );

static ConVar m_filter( "m_filter","0", FCVAR_ARCHIVE, "Mouse filtering (set this to 1 to average the mouse over 2 frames)." );
ConVar sensitivity( "sensitivity","3", FCVAR_ARCHIVE, "Mouse sensitivity.", true, 0.0001f, false, 10000000 );

static ConVar m_side( "m_side","0.8", FCVAR_ARCHIVE, "Mouse side factor." );
static ConVar m_yaw( "m_yaw","0.022", FCVAR_ARCHIVE, "Mouse yaw factor." );
static ConVar m_forward( "m_forward","1", FCVAR_ARCHIVE, "Mouse forward factor." );

static ConVar m_customaccel( "m_customaccel", "0", FCVAR_ARCHIVE, "Custom mouse acceleration (0 disable, 1 to enable, 2 enable with separate yaw/pitch rescale)."\
	"\nFormula: mousesensitivity = ( rawmousedelta^m_customaccel_exponent ) * m_customaccel_scale + sensitivity"\
	"\nIf mode is 2, then x and y sensitivity are scaled by m_pitch and m_yaw respectively.", true, 0, false, 0.0f );
static ConVar m_customaccel_scale( "m_customaccel_scale", "0.04", FCVAR_ARCHIVE, "Custom mouse acceleration value.", true, 0, false, 0.0f );
static ConVar m_customaccel_max( "m_customaccel_max", "0", FCVAR_ARCHIVE, "Max mouse move scale factor, 0 for no limit" );
static ConVar m_customaccel_exponent( "m_customaccel_exponent", "1", FCVAR_ARCHIVE, "Mouse move is raised to this power before being scaled by scale factor.");

static ConVar m_mouseaccel1( "m_mouseaccel1", "0", FCVAR_ARCHIVE, "Windows mouse acceleration initial threshold (2x movement).", true, 0, false, 0.0f );
static ConVar m_mouseaccel2( "m_mouseaccel2", "0", FCVAR_ARCHIVE, "Windows mouse acceleration secondary threshold (4x movement).", true, 0, false, 0.0f );
static ConVar m_mousespeed( "m_mousespeed", "1", FCVAR_ARCHIVE, "Windows mouse speed factor (range 1 to 20).", true, 1, true, 20 );

ConVar cl_mouselook( "cl_mouselook", "1", FCVAR_ARCHIVE | FCVAR_NOT_CONNECTED, "Set to 1 to use mouse for look, 0 for keyboard look. Cannot be set while connected to a server." );

ConVar cl_mouseenable( "cl_mouseenable", "1" );

// From other modules...
void GetVGUICursorPos( int& x, int& y );
void SetVGUICursorPos( int x, int y );

//-----------------------------------------------------------------------------
// Purpose: Hides cursor and starts accumulation/recentering
//-----------------------------------------------------------------------------
void CInput::ActivateMouse (void)
{
	if ( m_fMouseActive )
		return;

	if ( m_fMouseInitialized )
	{
		if ( m_fMouseParmsValid )
		{
			m_fRestoreSPI = SystemParametersInfo (SPI_SETMOUSE, 0, m_rgNewMouseParms, 0) ? true : false;
		}
		m_fMouseActive = true;

		ResetMouse();

		// Clear accumulated error, too
		m_flAccumulatedMouseXMovement = 0;
		m_flAccumulatedMouseYMovement = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Gives back the cursor and stops centering of mouse
//-----------------------------------------------------------------------------
void CInput::DeactivateMouse (void)
{
	// This gets called whenever the mouse should be inactive. We only respond to it if we had 
	// previously activated the mouse. We'll show the cursor in here.
	if ( !m_fMouseActive )
		return;

	if ( m_fMouseInitialized )
	{
		if ( m_fRestoreSPI )
		{
			SystemParametersInfo( SPI_SETMOUSE, 0, m_rgOrigMouseParms, 0 );
		}
		m_fMouseActive = false;
		vgui::surface()->SetCursor( vgui::dc_arrow );

		// Clear accumulated error, too
		m_flAccumulatedMouseXMovement = 0;
		m_flAccumulatedMouseYMovement = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CInput::CheckMouseAcclerationVars()
{
	// Don't change them if the mouse is inactive, invalid, or not using parameters for restore
	if ( !m_fMouseActive ||
		 !m_fMouseInitialized || 
		 !m_fMouseParmsValid || 
		 !m_fRestoreSPI )
	{
		return;
	}

	int values[ NUM_MOUSE_PARAMS ];

	values[ MOUSE_SPEED_FACTOR ]		= m_mousespeed.GetInt();
	values[ MOUSE_ACCEL_THRESHHOLD1 ]	= m_mouseaccel1.GetInt();
	values[ MOUSE_ACCEL_THRESHHOLD2 ]	= m_mouseaccel2.GetInt();

	bool dirty = false;

	int i;
	for ( i = 0; i < NUM_MOUSE_PARAMS; i++ )
	{
		if ( !m_rgCheckMouseParam[ i ] )
			continue;

		if ( values[ i ] != m_rgNewMouseParms[ i ] )
		{
			dirty = true;
			m_rgNewMouseParms[ i ] = values[ i ];

			char const *name = "";
			switch ( i )
			{
			default:
			case MOUSE_SPEED_FACTOR:
				name = "m_mousespeed";
				break;
			case MOUSE_ACCEL_THRESHHOLD1:
				name = "m_mouseaccel1";
				break;
			case MOUSE_ACCEL_THRESHHOLD2:
				name = "m_mouseaccel2";
				break;
			}

			char sz[ 256 ];
			Q_snprintf( sz, sizeof( sz ), "Mouse parameter '%s' set to %i\n", name, values[ i ] );
			DevMsg( "%s", sz );
		}
	}

	if ( dirty )
	{
		// Update them
		m_fRestoreSPI = SystemParametersInfo( SPI_SETMOUSE, 0, m_rgNewMouseParms, 0 ) ? true : false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: One-time initialization
//-----------------------------------------------------------------------------
void CInput::Init_Mouse (void)
{
	if ( CommandLine()->FindParm("-nomouse" ) ) 
		return; 

	m_flPreviousMouseXPosition = 0.0f;
	m_flPreviousMouseYPosition = 0.0f;

	m_fMouseInitialized = true;

	m_fMouseParmsValid = false;

	if ( CommandLine()->FindParm ("-useforcedmparms" ) ) 
	{
		m_fMouseParmsValid = SystemParametersInfo( SPI_GETMOUSE, 0, m_rgOrigMouseParms, 0 ) ? true : false;
		if ( m_fMouseParmsValid )
		{
			if ( CommandLine()->FindParm ("-noforcemspd" ) ) 
			{
				m_rgNewMouseParms[ MOUSE_SPEED_FACTOR ] = m_rgOrigMouseParms[ MOUSE_SPEED_FACTOR ];
			}
			else
			{
				m_rgCheckMouseParam[ MOUSE_SPEED_FACTOR ] = true;
			}

			if ( CommandLine()->FindParm ("-noforcemaccel" ) ) 
			{
				m_rgNewMouseParms[ MOUSE_ACCEL_THRESHHOLD1 ] = m_rgOrigMouseParms[ MOUSE_ACCEL_THRESHHOLD1 ];
				m_rgNewMouseParms[ MOUSE_ACCEL_THRESHHOLD2 ] = m_rgOrigMouseParms[ MOUSE_ACCEL_THRESHHOLD2 ];
			}
			else
			{
				m_rgCheckMouseParam[ MOUSE_ACCEL_THRESHHOLD1 ] = true;
				m_rgCheckMouseParam[ MOUSE_ACCEL_THRESHHOLD2 ] = true;
			}
		}
	}

	m_nMouseButtons = MOUSE_BUTTON_COUNT;
}

//-----------------------------------------------------------------------------
// Purpose: Get the center point of the engine window
// Input  : int&x - 
//			y - 
//-----------------------------------------------------------------------------
void CInput::GetWindowCenter( int&x, int& y )
{
	int w, h;
	engine->GetScreenSize( w, h );

	x = w >> 1;
	y = h >> 1;
}

//-----------------------------------------------------------------------------
// Purpose: Recenter the mouse
//-----------------------------------------------------------------------------
void CInput::ResetMouse( void )
{
	int x, y;
	GetWindowCenter( x,  y );
	SetMousePos( x, y );	
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : mstate - 
//			down - 
//-----------------------------------------------------------------------------
void CInput::MouseEvent( int mstate, bool down )
{
	// perform button actions
	for (int i=0 ; i<m_nMouseButtons ; i++)
	{
		// Mouse buttons 1 & 2 are swallowed when the mouse is visible
		if ( (i < 2) && ( m_fCameraInterceptingMouse || vgui::surface()->IsCursorVisible() ) )
			continue;

		// Only fire changed buttons
		int nBit = 1 << i;
		if ( (mstate & nBit) && !(m_nMouseOldButtons & nBit) )
		{
			engine->Key_Event( K_MOUSE1 + i, 1 );
		}
		if ( !(mstate & nBit) && (m_nMouseOldButtons & nBit) )
		{
			// Force 0 instead of down, because MouseMove calls this with down set to true.
			engine->Key_Event( K_MOUSE1 + i, 0 );
		}
	}	
	
	m_nMouseOldButtons = mstate;
}

//-----------------------------------------------------------------------------
// Purpose: GetAccumulatedMouse -- the mouse can be sampled multiple times per frame and
//  these results are accumulated each time. This function gets the accumulated mouse changes and resets the accumulators
// Input  : *mx - 
//			*my - 
//-----------------------------------------------------------------------------
void CInput::GetAccumulatedMouseDeltasAndResetAccumulators( float *mx, float *my )
{
	Assert( mx );
	Assert( my );

	*mx = m_flAccumulatedMouseXMovement;
	*my = m_flAccumulatedMouseYMovement;

	m_flAccumulatedMouseXMovement = 0;
	m_flAccumulatedMouseYMovement = 0;
}

//-----------------------------------------------------------------------------
// Purpose: GetMouseDelta -- Filters the mouse and stores results in old position
// Input  : mx - 
//			my - 
//			*oldx - 
//			*oldy - 
//			*x - 
//			*y - 
//-----------------------------------------------------------------------------
void CInput::GetMouseDelta( float inmousex, float inmousey, float *pOutMouseX, float *pOutMouseY )
{
	// Apply filtering?
	if ( m_filter.GetBool() )
	{
		// Average over last two samples
		*pOutMouseX = ( inmousex + m_flPreviousMouseXPosition ) * 0.5f;
		*pOutMouseY = ( inmousey + m_flPreviousMouseYPosition ) * 0.5f;
	}
	else
	{
		*pOutMouseX = inmousex;
		*pOutMouseY = inmousey;
	}

	// Latch previous
	m_flPreviousMouseXPosition = inmousex;
	m_flPreviousMouseYPosition = inmousey;

}

//-----------------------------------------------------------------------------
// Purpose: Multiplies mouse values by sensitivity.  Note that for windows mouse settings
//  the input x,y offsets are already scaled based on that.  The custom acceleration, therefore,
//  is totally engine-specific and applies as a post-process to allow more user tuning.
// Input  : *x - 
//			*y - 
//-----------------------------------------------------------------------------
void CInput::ScaleMouse( float *x, float *y )
{
	float mx = *x;
	float my = *y;

	float mouse_senstivity = ( gHUD.GetSensitivity() != 0 ) 
		?  gHUD.GetSensitivity() : sensitivity.GetFloat();

	// Pongles [

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if( pPlayer && pPlayer->IsProned() )
		mouse_senstivity = clamp( mouse_senstivity, 0.0f, 3.5f );

	// Pongles ]

	if ( m_customaccel.GetBool() ) 
	{ 
		float raw_mouse_movement_distance = sqrt( mx * mx + my * my );
		float acceleration_scale = m_customaccel_scale.GetFloat();
		float accelerated_sensitivity_max = m_customaccel_max.GetFloat();
		float accelerated_sensitivity_exponent = m_customaccel_exponent.GetFloat();
		float accelerated_sensitivity = ( (float)pow( raw_mouse_movement_distance, accelerated_sensitivity_exponent ) * acceleration_scale + mouse_senstivity );

		if ( accelerated_sensitivity_max > 0.0001f && 
			accelerated_sensitivity > accelerated_sensitivity_max )
		{
			accelerated_sensitivity = accelerated_sensitivity_max;
		}

		*x *= accelerated_sensitivity; 
		*y *= accelerated_sensitivity; 

		// Further re-scale by yaw and pitch magnitude if user requests alternate mode 2
		// This means that they will need to up their value for m_customaccel_scale greatly (>40x) since m_pitch/yaw default
		//  to 0.022
		if ( m_customaccel.GetInt() == 2 )
		{ 
			*x *= m_yaw.GetFloat(); 
			*y *= m_pitch.GetFloat(); 
		} 
	}
	else
	{ 
		*x *= mouse_senstivity;
		*y *= mouse_senstivity;
	}
}

#define BIPOD_YAW_LIMIT   60
#define BIPOD_PITCH_LIMIT_MIN 35
#define BIPOD_PITCH_LIMIT_MAX 20

#define PRONE_PITCH_LIMIT_MIN 30

//-----------------------------------------------------------------------------
// Purpose: ApplyMouse -- applies mouse deltas to CUserCmd
// Input  : viewangles - 
//			*cmd - 
//			mouse_x - 
//			mouse_y - 
//-----------------------------------------------------------------------------
extern ConVar cl_pitchup;

#define WEAPON_WEIGHT_LIMIT 8

void CInput::ApplyMouse( QAngle& viewangles, CUserCmd *cmd, float mouse_x, float mouse_y )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	if( pPlayer->IsPlayerDead( ) )
	{
		viewangles[PITCH] += mouse_y * m_pitch.GetFloat( );
		viewangles[YAW] -= mouse_x * m_yaw.GetFloat( );
	}
	else
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon( );

		if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_BIPOD )
		{
			QAngle bipod = pPlayer->GetSupportedFireAngles( );

			viewangles[ PITCH ] += mouse_y * m_pitch.GetFloat( );
			viewangles[ PITCH ] = clamp( viewangles[ PITCH ], bipod[ PITCH ] - BIPOD_PITCH_LIMIT_MIN, bipod[ PITCH ] + BIPOD_PITCH_LIMIT_MAX );

			viewangles[ YAW ] -= ( mouse_x * m_yaw.GetFloat( ) ) * 0.5f;

			if( !pPlayer->IsMoving( ) )
			{
				// clamp our view to min/max angles
				viewangles[ YAW ] = clamp( viewangles[ YAW ], bipod[ YAW ] - BIPOD_YAW_LIMIT, bipod[ YAW ] + BIPOD_YAW_LIMIT );
			
				// and now as viewangles € [180;-180] let's change the values to get it right
				if( viewangles[ YAW ] > 180.0f )
				{
					viewangles[ YAW ] -= 360.0f;

					if( bipod[ YAW ] + BIPOD_YAW_LIMIT  > 180.0f )
						bipod[ YAW ] -= 360.0f;
				}

				if( viewangles[ YAW ] < -180.0f )
				{
					viewangles[ YAW ] += 360.0f;

					if( bipod[ YAW ] - BIPOD_YAW_LIMIT < -180.0f )
						bipod[ YAW ] += 360.0f;
				}

				pPlayer->SetSupportedFireAngles( bipod );
			}
		}

		/*else if( (pWeapon == NULL) || (pPlayer->GetFlags() & FL_IRONSIGHTS) )
		{
			// PNOTE: this could a hole for cheaters, since this is only done client-side
			// but how much of an advantage is it?

			float flVChange[ 2 ] = {
				mouse_y * m_pitch.GetFloat(),
				-mouse_x * m_yaw.GetFloat() };

		#if 0

			for( int i = 0; i < 2; i++ )
			{
				if( flVChange[ i ] == 0.0f )
					continue;
					
				if( pWeapon->ValidISViewAngles( viewangles, flVChange[ i ], i ) )
					viewangles[ i ] += flVChange[ i ];
			}

		#else

			for( int i = 0; i < 2; i++ )
				viewangles[ i ] += flVChange[ i ];

		#endif

		}*/
		else
		{
			QAngle angFreeaim = vec3_angle;

			if( !pPlayer->IsFreeaimEnabled( ) )
			{
				viewangles[PITCH] += mouse_y * m_pitch.GetFloat();
				viewangles[YAW] -= mouse_x * m_yaw.GetFloat();
			}
			else
			{
				// freeaim
				float flFreeaim = pPlayer->GetFreeaimDistance();

				angFreeaim = pPlayer->GetFreeAimAngles( );

				float pitch = viewangles[PITCH];
				pitch = pitch - clamp(viewangles[PITCH], -cl_pitchup.GetFloat(), cl_pitchdown.GetFloat());

				float flMouseFraction = 0.5f + min( 1.0f - ( pWeapon->GetWeight() / WEAPON_WEIGHT_LIMIT ), 0.5f );

				angFreeaim[PITCH] += mouse_y * m_pitch.GetFloat() * flMouseFraction;
				angFreeaim[YAW] -= mouse_x * m_yaw.GetFloat() * flMouseFraction;

				float flWeaponScreenRelation = pPlayer->GetFreeaimScreenWeaponRelation( );
				float flFALength = angFreeaim.Length( );

				bool bSmoothFreeaim = ( flWeaponScreenRelation > 0.0f );

				if( bSmoothFreeaim || flFALength >= flFreeaim )
				{
					float flAimAngle = atan2f( angFreeaim[ PITCH ], angFreeaim[ YAW ] );

					float flFAMaxPitch = abs( flFreeaim * sin( flAimAngle ) );
					float flFAMaxYaw = abs( flFreeaim * cos( flAimAngle ) );

					float flFAPitch = abs( flFALength * sin( flAimAngle ) );
					float flFAYaw = abs( flFALength * cos( flAimAngle ) );

					float flPitchChange = mouse_y * m_pitch.GetFloat();

					float flLengthFraction = 1.0f;
					
					if( pPlayer->UseFreeaimSWRFraction( ) && flFALength >= 0.0f )
						flLengthFraction = flFALength / flFreeaim;

					if( flFAPitch > flFAMaxPitch )
					{
						angFreeaim[PITCH] = clamp( angFreeaim[PITCH], -flFAMaxPitch, flFAMaxPitch );
						viewangles[PITCH] += flPitchChange;
					}
					else if( bSmoothFreeaim )
					{
						viewangles[PITCH] += flPitchChange * ( flWeaponScreenRelation * flLengthFraction );
					}

					float flYawChange = mouse_x * m_yaw.GetFloat();

					if( flFAYaw > flFAMaxPitch )
					{
						angFreeaim[YAW] = clamp( angFreeaim[YAW], -flFAMaxYaw, flFAMaxYaw );
						viewangles[YAW] -= flYawChange;
					}
					else if( bSmoothFreeaim )
					{
						viewangles[YAW] -= flYawChange * ( flWeaponScreenRelation * flLengthFraction );
					}
				}

				pPlayer->SetFreeAimAngles( angFreeaim );
			}
		}
	}

	viewangles[PITCH] = clamp( viewangles[PITCH], -cl_pitchup.GetFloat(), cl_pitchdown.GetFloat() );

    cmd->mousedx = (int)mouse_x;
	cmd->mousedy = (int)mouse_y;
}

//-----------------------------------------------------------------------------
// Purpose: AccumulateMouse
//-----------------------------------------------------------------------------
void CInput::AccumulateMouse( void )
{
	if( !cl_mouseenable.GetBool() )
	{
		return;
	}

	if( !cl_mouselook.GetBool() )
	{
		return;
	}


	int x, y;
	GetWindowCenter( x,  y );

	//only accumulate mouse if we are not moving the camera with the mouse
	if ( !m_fCameraInterceptingMouse && vgui::surface()->IsCursorLocked() )
	{
		//Assert( !vgui::surface()->IsCursorVisible() );

		int current_posx, current_posy;

		GetMousePos(current_posx, current_posy);

		m_flAccumulatedMouseXMovement += current_posx - x;
		m_flAccumulatedMouseYMovement += current_posy - y;

		// force the mouse to the center, so there's room to move
		ResetMouse();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get raw mouse position
// Input  : &ox - 
//			&oy - 
//-----------------------------------------------------------------------------
void CInput::GetMousePos(int &ox, int &oy)
{
	GetVGUICursorPos( ox, oy );
}

//-----------------------------------------------------------------------------
// Purpose: Force raw mouse position
// Input  : x - 
//			y - 
//-----------------------------------------------------------------------------
void CInput::SetMousePos(int x, int y)
{
	SetVGUICursorPos(x, y);
}

//-----------------------------------------------------------------------------
// Purpose: MouseMove -- main entry point for applying mouse
// Input  : *cmd - 
//-----------------------------------------------------------------------------
void CInput::MouseMove( CUserCmd *cmd )
{
	float	mouse_x, mouse_y;
	float	mx, my;
	QAngle	viewangles;
	QAngle  freeAimAngles;

	// Get view angles from engine
	engine->GetViewAngles( viewangles );
	
	// Validate mouse speed/acceleration settings
	CheckMouseAcclerationVars();

	// Don't dript pitch at all while mouselooking.
	view->StopPitchDrift ();

	// Pongles [

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	//jjb - this disbles normal mouse control if the user is trying to 
	//      move the camera, or if the mouse cursor is visible 
	//    - don't let the player move when frozen
	if ( !m_fCameraInterceptingMouse && 
		 !vgui::surface()->IsCursorVisible() && 
         !(pPlayer && (pPlayer->GetFlags() & FL_FROZEN)) )
	{
		// Sample mouse one more time
		AccumulateMouse();

		// Latch accumulated mouse movements and reset accumulators
		GetAccumulatedMouseDeltasAndResetAccumulators( &mx, &my );

		// Filter, etc. the delta values and place into mouse_x and mouse_y
		GetMouseDelta( mx, my, &mouse_x, &mouse_y );

		// Apply scaling factor
		ScaleMouse( &mouse_x, &mouse_y );

		// Let the client mode at the mouse input before it's used
		g_pClientMode->OverrideMouseInput( &mouse_x, &mouse_y );

		// Add mouse X/Y movement to cmd
		ApplyMouse( viewangles, cmd, mouse_x, mouse_y );

		// Re-center the mouse.
		ResetMouse();
	}
	// Pongles ]

	// Store out the new viewangles.
	engine->SetViewAngles( viewangles );
	
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *mx - 
//			*my - 
//			*unclampedx - 
//			*unclampedy - 
//-----------------------------------------------------------------------------
void CInput::GetFullscreenMousePos( int *mx, int *my, int *unclampedx /*=NULL*/, int *unclampedy /*=NULL*/ )
{
	Assert( mx );
	Assert( my );

	if ( !vgui::surface()->IsCursorVisible() )
	{
		return;
	}

	int x, y;
	GetWindowCenter( x,  y );

	int		current_posx, current_posy;

	GetMousePos(current_posx, current_posy);

	current_posx -= x;
	current_posy -= y;

	// Now need to add back in mid point of viewport
	//

	int w, h;
	vgui::surface()->GetScreenSize( w, h );
	current_posx += w  / 2;
	current_posy += h / 2;

	if ( unclampedx )
	{
		*unclampedx = current_posx;
	}

	if ( unclampedy )
	{
		*unclampedy = current_posy;
	}

	// Clamp
	current_posx = max( 0, current_posx );
	current_posx = min( ScreenWidth(), current_posx );

	current_posy = max( 0, current_posy );
	current_posy = min( ScreenHeight(), current_posy );

	*mx = current_posx;
	*my = current_posy;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : mx - 
//			my - 
//-----------------------------------------------------------------------------
void CInput::SetFullscreenMousePos( int mx, int my )
{
	SetMousePos( mx, my );
}

//-----------------------------------------------------------------------------
// Purpose: ClearStates -- Resets mouse accumulators so you don't get a pop when returning to trapped mouse
//-----------------------------------------------------------------------------
void CInput::ClearStates (void)
{
	if ( !m_fMouseActive )
	{
		return;
	}

	m_flAccumulatedMouseXMovement = 0;
	m_flAccumulatedMouseYMovement = 0;
	m_nMouseOldButtons = 0;
}
