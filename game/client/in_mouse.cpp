//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Mouse input routines
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//===========================================================================//
#if defined( WIN32 ) && !defined( _X360 )
#define _WIN32_WINNT 0x0502
#include <windows.h>
#endif
#include "cbase.h"
#include "hud.h"
#include "cdll_int.h"
#include "kbutton.h"
#include "basehandle.h"
#include "usercmd.h"
#include "input.h"
#include "iviewrender.h"
#include "iclientmode.h"
#include "tier0/icommandline.h"
#include "vgui/ISurface.h"
#include "vgui_controls/Controls.h"
#include "vgui/Cursor.h"
#include "cdll_client_int.h"
#include "cdll_util.h"
#include "tier1/convar_serverbounded.h"
#include "cam_thirdperson.h"
#include "inputsystem/iinputsystem.h"
#include "c_ins_player.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// up / down
#define	PITCH	0
// left / right
#define	YAW		1

extern ConVar lookstrafe;
extern ConVar cl_pitchdown;
extern ConVar cl_pitchup;
extern const ConVar *sv_cheats;

class ConVar_m_pitch : public ConVar_ServerBounded
{
public:
	ConVar_m_pitch() : 
		ConVar_ServerBounded( "m_pitch","0.022", FCVAR_ARCHIVE, "Mouse pitch factor." )
	{
	}
	
	virtual float GetFloat() const
	{
		if ( !sv_cheats )
			sv_cheats = cvar->FindVar( "sv_cheats" );

		// If sv_cheats is on then it can be anything.
		float flBaseValue = GetBaseFloatValue();
		if ( !sv_cheats || sv_cheats->GetBool() )
			return flBaseValue;

		// If sv_cheats is off than it can only be 0.022 or -0.022 (if they've reversed the mouse in the options).		
		if ( flBaseValue > 0 )
			return 0.022f;
		else
			return -0.022f;
	}
} cvar_m_pitch;
ConVar_ServerBounded *m_pitch = &cvar_m_pitch;

extern ConVar cam_idealyaw;
extern ConVar cam_idealpitch;

static ConVar m_filter( "m_filter","0", FCVAR_ARCHIVE, "Mouse filtering (set this to 1 to average the mouse over 2 frames)." );
ConVar sensitivity( "sensitivity","3", FCVAR_ARCHIVE, "Mouse sensitivity.", true, 0.0001f, true, 10000000 );

static ConVar m_side( "m_side","0.8", FCVAR_ARCHIVE, "Mouse side factor." );
static ConVar m_yaw( "m_yaw","0.022", FCVAR_ARCHIVE, "Mouse yaw factor." );
static ConVar m_forward( "m_forward","1", FCVAR_ARCHIVE, "Mouse forward factor." );

static ConVar m_customaccel( "m_customaccel", "0", FCVAR_ARCHIVE, "Custom mouse acceleration:"
	"\n0: custom accelaration disabled"
	"\n1: mouse_acceleration = min(m_customaccel_max, pow(raw_mouse_delta, m_customaccel_exponent) * m_customaccel_scale + sensitivity)"
	"\n2: Same as 1, with but x and y sensitivity are scaled by m_pitch and m_yaw respectively."
	"\n3: mouse_acceleration = pow(raw_mouse_delta, m_customaccel_exponent - 1) * sensitivity"
	);
static ConVar m_customaccel_scale( "m_customaccel_scale", "0.04", FCVAR_ARCHIVE, "Custom mouse acceleration value.", true, 0, false, 0.0f );
static ConVar m_customaccel_max( "m_customaccel_max", "0", FCVAR_ARCHIVE, "Max mouse move scale factor, 0 for no limit" );
static ConVar m_customaccel_exponent( "m_customaccel_exponent", "1", FCVAR_ARCHIVE, "Mouse move is raised to this power before being scaled by scale factor.", true, 1.0f, false, 0.0f);

static ConVar m_mousespeed( "m_mousespeed", "1", FCVAR_ARCHIVE, "Windows mouse acceleration (0 to disable, 1 to enable [Windows 2000: enable initial threshold], 2 to enable secondary threshold [Windows 2000 only]).", true, 0, true, 2 );
static ConVar m_mouseaccel1( "m_mouseaccel1", "0", FCVAR_ARCHIVE, "Windows mouse acceleration initial threshold (2x movement).", true, 0, false, 0.0f );
static ConVar m_mouseaccel2( "m_mouseaccel2", "0", FCVAR_ARCHIVE, "Windows mouse acceleration secondary threshold (4x movement).", true, 0, false, 0.0f );

static ConVar m_rawinput( "m_rawinput", "0", FCVAR_ARCHIVE, "Use Raw Input for mouse input.");

#if DEBUG
ConVar cl_mouselook( "cl_mouselook", "1", FCVAR_ARCHIVE, "Set to 1 to use mouse for look, 0 for keyboard look." );
#else
ConVar cl_mouselook( "cl_mouselook", "1", FCVAR_ARCHIVE | FCVAR_NOT_CONNECTED, "Set to 1 to use mouse for look, 0 for keyboard look. Cannot be set while connected to a server." );
#endif

ConVar cl_mouseenable( "cl_mouseenable", "1" );

// From other modules...
void GetVGUICursorPos( int& x, int& y );
void SetVGUICursorPos( int x, int y );

//-----------------------------------------------------------------------------
// Purpose: Hides cursor and starts accumulation/re-centering
//-----------------------------------------------------------------------------
void CInput::ActivateMouse (void)
{
	if ( m_fMouseActive )
		return;

	if ( m_fMouseInitialized )
	{
		if ( m_fMouseParmsValid )
		{
#if defined( PLATFORM_WINDOWS )
			m_fRestoreSPI = SystemParametersInfo (SPI_SETMOUSE, 0, m_rgNewMouseParms, 0) ? true : false;
#endif
		}
		m_fMouseActive = true;

		ResetMouse();
#if !defined( PLATFORM_WINDOWS )
		int dx, dy;
		engine->GetMouseDelta( dx, dy, true );
#endif

		// Clear accumulated error, too
		m_flAccumulatedMouseXMovement = 0;
		m_flAccumulatedMouseYMovement = 0;

		// clear raw mouse accumulated data
		int rawX, rawY;
		inputsystem->GetRawMouseAccumulators(rawX, rawY);
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
#if defined( PLATFORM_WINDOWS )
			SystemParametersInfo( SPI_SETMOUSE, 0, m_rgOrigMouseParms, 0 );
#endif
		}
		m_fMouseActive = false;
		vgui::surface()->SetCursor( vgui::dc_arrow );
#if !defined( PLATFORM_WINDOWS )
		// now put the mouse back in the middle of the screen
		ResetMouse();
#endif

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
#ifdef WIN32
		m_fRestoreSPI = SystemParametersInfo( SPI_SETMOUSE, 0, m_rgNewMouseParms, 0 ) ? true : false;
#endif
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
#ifdef WIN32
		m_fMouseParmsValid = SystemParametersInfo( SPI_GETMOUSE, 0, m_rgOrigMouseParms, 0 ) ? true : false;
#else
		m_fMouseParmsValid = false;
#endif
		if ( m_fMouseParmsValid )
		{
			if ( CommandLine()->FindParm ("-noforcemspd" ) ) 
			{
				m_rgNewMouseParms[ MOUSE_SPEED_FACTOR ] = m_rgOrigMouseParms[ MOUSE_SPEED_FACTOR ];

/*
				int mouseAccel[3];
				SystemParametersInfo(SPI_GETMOUSE, 0, &mouseAccel, 0); mouseAccel[2] = 0; 
				bool ok = SystemParametersInfo(SPI_SETMOUSE, 0, &mouseAccel, SPIF_UPDATEINIFILE); 
				
				// Now check registry and close/re-open Control Panel > Mouse and see 'Enhance pointer precision' is OFF 
				mouseAccel[2] = 1; 
				ok = SystemParametersInfo(SPI_SETMOUSE, 0, &mouseAccel, SPIF_UPDATEINIFILE); 
				
				// Now check registry and close/re-open Control Panel > Mouse and see 'Enhance pointer precision' is ON
*/
			}
			else
			{
				m_rgCheckMouseParam[ MOUSE_SPEED_FACTOR ] = 1;
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

	if ( m_rawinput.GetBool() )
	{
		int rawMouseX, rawMouseY;
		if ( inputsystem->GetRawMouseAccumulators(rawMouseX, rawMouseY) )
		{
			*mx = (float)rawMouseX;
			*my = (float)rawMouseY;
		}
	}
	
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

	float mouse_sensitivity = ( gHUD.GetSensitivity() != 0 ) 
		?  gHUD.GetSensitivity() : sensitivity.GetFloat();

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (pPlayer && pPlayer->IsProned())
		mouse_sensitivity = clamp(mouse_sensitivity, 0.0f, 3.5f);

	if ( m_customaccel.GetInt() == 1 ||  m_customaccel.GetInt() == 2 ) 
	{ 
		float raw_mouse_movement_distance = sqrt( mx * mx + my * my );
		float acceleration_scale = m_customaccel_scale.GetFloat();
		float accelerated_sensitivity_max = m_customaccel_max.GetFloat();
		float accelerated_sensitivity_exponent = m_customaccel_exponent.GetFloat();
		float accelerated_sensitivity = ( (float)pow( raw_mouse_movement_distance, accelerated_sensitivity_exponent ) * acceleration_scale + mouse_sensitivity );

		if ( accelerated_sensitivity_max > 0.0001f && 
			accelerated_sensitivity > accelerated_sensitivity_max )
		{
			accelerated_sensitivity = accelerated_sensitivity_max;
		}

		*x *= accelerated_sensitivity; 
		*y *= accelerated_sensitivity; 

		// Further re-scale by yaw and pitch magnitude if user requests alternate mode 2/4
		// This means that they will need to up their value for m_customaccel_scale greatly (>40x) since m_pitch/yaw default
		//  to 0.022
		if ( m_customaccel.GetInt() == 2 || m_customaccel.GetInt() == 4 )
		{ 
			*x *= m_yaw.GetFloat(); 
			*y *= m_pitch->GetFloat(); 
		} 
	}
	else if ( m_customaccel.GetInt() == 3 )
	{
		float raw_mouse_movement_distance_squared = mx * mx + my * my;
		float fExp = MAX(0.0f, (m_customaccel_exponent.GetFloat() - 1.0f) / 2.0f);
		float accelerated_sensitivity = powf( raw_mouse_movement_distance_squared, fExp ) * mouse_sensitivity;

		*x *= accelerated_sensitivity; 
		*y *= accelerated_sensitivity; 
	}
	else
	{ 
		*x *= mouse_sensitivity;
		*y *= mouse_sensitivity;
	}
}

//-----------------------------------------------------------------------------
// Purpose: ApplyMouse -- applies mouse deltas to CUserCmd
// Input  : viewangles - 
//			*cmd - 
//			mouse_x - 
//			mouse_y - 
//-----------------------------------------------------------------------------

#define BIPOD_YAW_LIMIT   60
#define BIPOD_PITCH_LIMIT_MIN 35
#define BIPOD_PITCH_LIMIT_MAX 20
#define PRONE_PITCH_LIMIT_MIN 30
#define WEAPON_WEIGHT_LIMIT 8

void CInput::ApplyMouse( QAngle& viewangles, CUserCmd *cmd, float mouse_x, float mouse_y )
{
	C_INSPlayer* pPlayer = C_INSPlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	if (pPlayer->IsPlayerDead())
	{
		viewangles[PITCH] += mouse_y * m_pitch->GetFloat();
		viewangles[YAW] -= mouse_x * m_yaw.GetFloat();
	}
	else
	{
		CBaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();

		if (pPlayer->GetPlayerFlags() & FL_PLAYER_BIPOD)
		{
			QAngle bipod = pPlayer->GetSupportedFireAngles();

			viewangles[PITCH] += mouse_y * m_pitch->GetFloat();
			viewangles[PITCH] = clamp(viewangles[PITCH], bipod[PITCH] - BIPOD_PITCH_LIMIT_MIN, bipod[PITCH] + BIPOD_PITCH_LIMIT_MAX);

			viewangles[YAW] -= (mouse_x * m_yaw.GetFloat()) * 0.5f;

			if (!pPlayer->IsMoving())
			{
				// clamp our view to min/max angles
				viewangles[YAW] = clamp(viewangles[YAW], bipod[YAW] - BIPOD_YAW_LIMIT, bipod[YAW] + BIPOD_YAW_LIMIT);

				// and now as viewangles € [180;-180] let's change the values to get it right
				if (viewangles[YAW] > 180.0f)
				{
					viewangles[YAW] -= 360.0f;

					if (bipod[YAW] + BIPOD_YAW_LIMIT > 180.0f)
						bipod[YAW] -= 360.0f;
				}

				if (viewangles[YAW] < -180.0f)
				{
					viewangles[YAW] += 360.0f;

					if (bipod[YAW] - BIPOD_YAW_LIMIT < -180.0f)
						bipod[YAW] += 360.0f;
				}

				pPlayer->SetSupportedFireAngles(bipod);
			}
		}
		else
		{
			QAngle angFreeaim = vec3_angle;

			if (!pPlayer->IsFreeaimEnabled())
			{
				viewangles[PITCH] += mouse_y * m_pitch->GetFloat();
				viewangles[YAW] -= mouse_x * m_yaw.GetFloat();
			}
			else
			{
				// freeaim
				float flFreeaim = pPlayer->GetFreeaimDistance();
				float flWepWeight = (pWeapon ? pWeapon->GetWeight() : 4.0f);

				angFreeaim = pPlayer->GetFreeAimAngles();

				float pitch = viewangles[PITCH];
				pitch = pitch - clamp(viewangles[PITCH], -cl_pitchup.GetFloat(), cl_pitchdown.GetFloat());

				float flMouseFraction = 0.5f + min(1.0f - (flWepWeight / WEAPON_WEIGHT_LIMIT), 0.5f);

				angFreeaim[PITCH] += mouse_y * m_pitch->GetFloat() * flMouseFraction;
				angFreeaim[YAW] -= mouse_x * m_yaw.GetFloat() * flMouseFraction;

				float flWeaponScreenRelation = pPlayer->GetFreeaimScreenWeaponRelation();
				float flFALength = angFreeaim.Length();

				bool bSmoothFreeaim = (flWeaponScreenRelation > 0.0f);

				if (bSmoothFreeaim || flFALength >= flFreeaim)
				{
					float flAimAngle = atan2f(angFreeaim[PITCH], angFreeaim[YAW]);

					float flFAMaxPitch = abs(flFreeaim * sin(flAimAngle));
					float flFAMaxYaw = abs(flFreeaim * cos(flAimAngle));

					float flFAPitch = abs(flFALength * sin(flAimAngle));
					float flFAYaw = abs(flFALength * cos(flAimAngle));

					float flPitchChange = mouse_y * m_pitch->GetFloat();

					float flLengthFraction = 1.0f;

					if (pPlayer->UseFreeaimSWRFraction() && flFALength >= 0.0f)
						flLengthFraction = flFALength / flFreeaim;

					if (flFAPitch > flFAMaxPitch)
					{
						angFreeaim[PITCH] = clamp(angFreeaim[PITCH], -flFAMaxPitch, flFAMaxPitch);
						viewangles[PITCH] += flPitchChange;
					}
					else if (bSmoothFreeaim)
					{
						viewangles[PITCH] += flPitchChange * (flWeaponScreenRelation * flLengthFraction);
					}

					float flYawChange = mouse_x * m_yaw.GetFloat();

					if (flFAYaw > flFAMaxPitch)
					{
						angFreeaim[YAW] = clamp(angFreeaim[YAW], -flFAMaxYaw, flFAMaxYaw);
						viewangles[YAW] -= flYawChange;
					}
					else if (bSmoothFreeaim)
					{
						viewangles[YAW] -= flYawChange * (flWeaponScreenRelation * flLengthFraction);
					}
				}

				pPlayer->SetFreeAimAngles(angFreeaim);
			}
		}
	}

	viewangles[PITCH] = clamp(viewangles[PITCH], -cl_pitchup.GetFloat(), cl_pitchdown.GetFloat());
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

	if ( m_rawinput.GetBool() )
	{
		return;
	}

	int w, h;
	engine->GetScreenSize( w, h );

	// x,y = screen center
	int x = w >> 1;	x;
	int y = h >> 1;	y;

	//only accumulate mouse if we are not moving the camera with the mouse
	if ( !m_fCameraInterceptingMouse && vgui::surface()->IsCursorLocked() )
	{
		//Assert( !vgui::surface()->IsCursorVisible() );
		// By design, we follow the old mouse path even when using SDL for Windows, to retain old mouse behavior.
#if defined( PLATFORM_WINDOWS )
		int current_posx, current_posy;

		GetMousePos(current_posx, current_posy);

		m_flAccumulatedMouseXMovement += current_posx - x;
		m_flAccumulatedMouseYMovement += current_posy - y;
		
#elif defined( USE_SDL )
		int dx, dy;
		engine->GetMouseDelta( dx, dy );
		m_flAccumulatedMouseXMovement += dx;
		m_flAccumulatedMouseYMovement += dy;
#else
#error
#endif
		// force the mouse to the center, so there's room to move
		ResetMouse();
	}
	else if ( m_fMouseActive )
	{
		// Clamp
		int ox, oy;
		GetMousePos( ox, oy );
		ox = clamp( ox, 0, w - 1 );
		oy = clamp( oy, 0, h - 1 );
		SetMousePos( ox, oy );
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

	// Get view angles from engine
	engine->GetViewAngles( viewangles );

	// Validate mouse speed/acceleration settings
	CheckMouseAcclerationVars();

	// Don't drift pitch at all while mouselooking.
	view->StopPitchDrift ();

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

	//jjb - this disables normal mouse control if the user is trying to 
	//      move the camera, or if the mouse cursor is visible 
	if (!m_fCameraInterceptingMouse &&
		!vgui::surface()->IsCursorVisible() &&
		!(pPlayer && (pPlayer->GetFlags() & FL_FROZEN)))
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
	current_posx = MAX( 0, current_posx );
	current_posx = MIN( ScreenWidth(), current_posx );

	current_posy = MAX( 0, current_posy );
	current_posy = MIN( ScreenHeight(), current_posy );

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
		return;

	m_flAccumulatedMouseXMovement = 0;
	m_flAccumulatedMouseYMovement = 0;

	// clear raw mouse accumulated data
	int rawX, rawY;
	inputsystem->GetRawMouseAccumulators(rawX, rawY);
}