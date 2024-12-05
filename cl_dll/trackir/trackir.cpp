//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#define PROTECTED_THINGS_H

#include "cbase.h"

#include <windows.h>
#include <winuser.h>

#undef PROTECTED_THINGS_H
#include "protected_things.h"

#include "tier0/vcrmode.h"

#include "trackir.h"
#include "npclient.h"
#include "npclientwraps.h"
#include "in_buttons.h"

#include "ins_player_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define TRACKIR_DEVELOPER_ID 9401

#define TRACKIR_INTERPOLATION_TIME 0.2f

//=========================================================
//=========================================================
void TrackIRChange( ConVar *pVar, char const *pszOldString );

ConVar trackir( "cl_trackir", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "defines trackir availability", true, 0, true, 1, TrackIRChange );
ConVar trackir_mode( "cl_trackir_mode", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "defines trackir mode", true, 0, true, TMODE_COUNT - 1 );
ConVar trackir_igheadi( "cl_trackir_igheadi", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "ignore viewangles while in ironsights", true, 0, true, 1 );

#ifdef TESTING

ConVar trackir_verbose( "cl_trackir_verbose", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "defines trackir verbosity", true, 0, true, 1 );

#endif

//=========================================================
//=========================================================
void TrackIRChange( ConVar *pVar, char const *pszOldString )
{
	CTrackIRInterface &TrackIR = TrackIRInterface( );

	if( trackir.GetBool( ) == TrackIR.IsInit( ) )
		return;

	if( trackir.GetBool( ) )
		TrackIR.Init( );
	else
		TrackIR.Shutdown( );
}

//=========================================================
//=========================================================
TrackIRData_t::TrackIRData_t( )
{
	m_angHeadAngles.Init( );
	m_flLean = 0.0f;
	m_flLookOver = 0.0f;
	m_flZoom = 0.0f;
}

//=========================================================
//=========================================================
CTrackIRInterface CTrackIRInterface::m_TrackIRInterface;

//=========================================================
//=========================================================
CTrackIRInterface::CTrackIRInterface( )
{
	m_bInit = false;

	m_iNPFrameSignature = 0;

	m_bInterpActive = false;
	m_flStartInterpTime = 0.0f;
}

//=========================================================
//=========================================================
CTrackIRInterface::~CTrackIRInterface( )
{
	if( !IsInit( ) )
		return;

	Shutdown( );
}

//=========================================================
//=========================================================
CTrackIRInterface &CTrackIRInterface::TrackIRInterface( void )
{
	return m_TrackIRInterface;
}

//=========================================================
//=========================================================
CTrackIRInterface &TrackIRInterface( void )
{
	return CTrackIRInterface::TrackIRInterface( );
}

//=========================================================
//=========================================================
bool CTrackIRInterface::Init( void )
{
	if( !trackir.GetBool( ) )
		return true;

    // zero TrackIR SDK related counters
	m_iNPFrameSignature = 0;

    // locate the TrackIR enhanced DLL
	char szDLLPath[ _MAX_PATH ];

    if( !GetDLLPath( szDLLPath ) )
		return false;
	
    // initialize the the TrackIR enhanced DLL
	NPRESULT Result = NPClient_Init( szDLLPath );

	if( Result != NP_OK )
		return false;

    // register your applications window handle 
	HWND ValveWindow = 0;
	ValveWindow = FindWindow( "Valve001", NULL );

	if( ValveWindow == 0 )
		return false;

	Result = NP_RegisterWindowHandle( ValveWindow );

	if( Result != NP_OK )
		Warning( "TrackIR failed to Register HWND\n" );

	// query for the naturalpoint TrackIR software version
	unsigned short iNPClientVer;
	Result = NP_QueryVersion( &iNPClientVer );

	if( Result != NP_OK )
		return false;

	char szMajorVersion[ 128 ], szMinorVersion[ 128 ];
	Q_snprintf( szMajorVersion, sizeof( szMajorVersion ), "%d", ( iNPClientVer >> 8 ) );
	Q_snprintf( szMinorVersion, sizeof( szMinorVersion ), "%02d", ( iNPClientVer & 0x00FF ) );

    // Choose the Axes that you want tracking data for
	unsigned int DataFields = 0;
    
    // rotation axes
	DataFields |= NPPitch;
	DataFields |= NPYaw;
	DataFields |= NPRoll;

    // translation axes
	DataFields |= NPX;
	DataFields |= NPY;
	DataFields |= NPZ;

    // register the axes selection with the TrackIR enhanced interface
	NP_RequestData( DataFields );

	// register program ID
    NP_RegisterProgramProfileID( TRACKIR_DEVELOPER_ID );

    // stop the cursor
	Result = NP_StopCursor( );

	if( Result != NP_OK )
		return false;

    // send the message
	Msg( "TrackIR %s.%s has been Initialized\n", szMajorVersion, szMinorVersion );

	// start tracking when we can
	if( m_bCanTransmit )
		StartDataTransmission( );

	// we have init
	m_bInit = true;

	return true;
}

//=========================================================
//=========================================================
bool CTrackIRInterface::GetDLLPath( char *pszBuffer )
{
	// find path to NPClient.dll
	HKEY pKey = NULL;

	// open the registry key 
	if( g_pVCR->Hook_RegOpenKeyEx( HKEY_CURRENT_USER,
			"Software\\NaturalPoint\\NATURALPOINT\\NPClient Location",
			0, KEY_READ, &pKey ) != ERROR_SUCCESS )
		return false;

	// get the value from the key
	unsigned char *pszValue;
	DWORD dwSize;

	// first discover the size of the value
	if( g_pVCR->Hook_RegQueryValueEx(pKey,
		"Path", NULL, NULL, NULL,
		&dwSize) == ERROR_SUCCESS )
	{
		// allocate memory for the buffer for the value
		pszValue = new unsigned char[ dwSize ];

		if( !pszValue )
			return false;

		// now get the value
        if ( g_pVCR->Hook_RegQueryValueEx(pKey,
			"Path", NULL, NULL, pszValue, &dwSize) == ERROR_SUCCESS )
		{
			g_pVCR->Hook_RegCloseKey( pKey  );

			Q_strncpy( pszBuffer, ( char* )pszValue, _MAX_PATH );

			return true;
		}
		else
		{
			return false;
		}
			
	}

	g_pVCR->Hook_RegCloseKey(pKey);
	return false;
}

//=========================================================
//=========================================================
void CTrackIRInterface::Shutdown( void )
{
    StopDataTransmission( );
    
    // un-register your applications Windows Handle
    NP_UnregisterWindowHandle( );

	// send the message
	Msg( "TrackIR has been Shutdown\n" );

	m_bInit = false;
}

//=========================================================
//=========================================================
void CTrackIRInterface::StartDataTransmission( void )
{
	// request that the TrackIR software begins sending Tracking Data
	NP_StartDataTransmission( );

	// mark it
	m_bCanTransmit = true;
}

//=========================================================
//=========================================================
void CTrackIRInterface::StopDataTransmission( void )
{
    // request that the TrackIR software stop sending Tracking Data
	NP_StopDataTransmission( );

	// mark it
	m_bCanTransmit = false;
}

//=========================================================
//=========================================================
void CTrackIRInterface::LevelInitPreEntity( void )
{
	StartDataTransmission( );
}

//=========================================================
//=========================================================
void CTrackIRInterface::LevelShutdownPreEntity( void )
{
	StopDataTransmission( );
}

//=========================================================
//=========================================================
void CTrackIRInterface::Update( float flFrametime )
{
	if( !IsInit( ) )
		return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	// find if TrackIR is being used
	bool bTrackIRPressed, bTrackIRState;

	bTrackIRPressed = ( pPlayer->m_nButtons & IN_TRACKIR );
	bTrackIRState = false;

	switch( trackir_mode.GetInt( ) )
	{
		case TMODE_PRESS:
		{
			if( bTrackIRPressed )
				bTrackIRState = true;

			break;
		}

		case TMODE_RELEASE:
		{
			if( !bTrackIRPressed )
				bTrackIRState = true;

			break;
		}
	}

	if( bTrackIRState )
	{
		TRACKIRDATA tID;

		// query the TrackIR Enhanced Interface for the latest data
		// ... if the call succeeded, then we have data to process
		if( NP_GetData( &tID ) != NP_OK )
			return;

		// make sure the remote interface is active
		if( tID.wNPStatus != NPSTATUS_REMOTEACTIVE )
			return;

		// compare the last frame signature to the current one if 
		// they are not the same then the data is new
		if( tID.wPFrameSignature == m_iNPFrameSignature )
			return;

		// print out verbose if requested
	#ifdef TESTING

		if( trackir_verbose.GetBool( ) )
		{
			Msg( "Rotation : NPPitch = %04.02f, NPYaw = %04.02f, \nTranslation : NPY = %04.02f\nInformation NPStatus = %d, Frame = %d\n", 
					tID.fNPPitch, 
					tID.fNPYaw, 
					tID.fNPY, 
					tID.wNPStatus, 
					tID.wPFrameSignature );
		}

	#endif

		// take values
		m_Data.m_angHeadAngles[ 0 ] = ( tID.fNPPitch / 180.0f );
		m_Data.m_angHeadAngles[ 1 ] = ( tID.fNPYaw / 180.0f );
		m_Data.m_angHeadAngles[ 2 ] = -( tID.fNPRoll / 180.0f );

		m_Data.m_flLean = clamp( ( tID.fNPX / 16383 ), -1.0f, 1.0f );
		m_Data.m_flLookOver = clamp( ( tID.fNPY / 16383 ), 0.0f, 1.0f );
		m_Data.m_flZoom = clamp( ( tID.fNPZ / 16383 ), -1.0f, 1.0f );

		// mark current frame
		m_iNPFrameSignature = tID.wPFrameSignature;

		/*if( m_bInterpActive )
			m_flStartInterpolationTime = gpGlobals->curtime;

		m_bInterpActive = false;

		if( m_flStartInterpolationTime != 0.0f )
		{
			if(gpGlobals->curtime < (m_flStartInterpolationTime + INTERPOLATION_TIME))
				{
					float flFraction = (gpGlobals->curtime - m_flStartInterpolationTime) / INTERPOLATION_TIME;

					m_flLean = m_flLean * flFraction;

					m_HeadAngles[0] = m_HeadAngles[0] * flFraction;
					m_HeadAngles[1] = m_HeadAngles[1] * flFraction;
					m_HeadAngles[2] = m_HeadAngles[2] * flFraction;
				}
				else
				{
					m_flStartInterpolationTime = 0.0f;
				}
		}*/
	}
	else
	{
		/*(m_bInterpActive = true;

		if( m_flStartInterpTime == 0.0f || m_flStartInterpTime > gpGlobals->curtime )
		{
			m_flStartInterpolationTime = gpGlobals->curtime;
			m_flInterpLean = m_flLean;
			m_InterpHeadAngles = m_HeadAngles;
		}

		if(gpGlobals->curtime >= (m_flStartInterpolationTime + INTERPOLATION_TIME))
		{
			m_flLean = 0.0f;
			m_HeadAngles = vec3_angle;

			m_flStartInterpolationTime = 0.0f;





			return;



		}

		float flFraction = 1.0f - ((gpGlobals->curtime - m_flStartInterpolationTime) / INTERPOLATION_TIME);

		m_flLean = m_flInterpLean * flFraction;
		m_HeadAngles[0] = m_InterpHeadAngles[0] * flFraction;
		m_HeadAngles[1] = m_InterpHeadAngles[1] * flFraction;
		m_HeadAngles[2] = m_InterpHeadAngles[2] * flFraction;

		return;*/
	}
}