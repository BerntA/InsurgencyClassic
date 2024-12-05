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

#include "lglcd.h"
#include "g15lcd.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
bool IsValidLCDHandle( DWORD &hLCD )
{
	return ( hLCD == ERROR_SUCCESS );
}

//=========================================================
//=========================================================
CG15LCDInterface g_G15LCDInterface;

CG15LCDInterface::CG15LCDInterface( )
{
	m_bInit = false;
}

//=========================================================
//=========================================================
bool CG15LCDInterface::Init( void )
{
    DWORD hLCD;

    // initialize the library
    hLCD = lgLcdInit( );

	if( !IsValidLCDHandle( hLCD ) )
		return true;

	lgLcdConnectContext ConnectContext;
    ZeroMemory( &ConnectContext, sizeof( ConnectContext ) );
    ConnectContext.appFriendlyName = _T( "Insurgency" );
    ConnectContext.isAutostartable = TRUE;
    ConnectContext.isPersistent = TRUE;

    // we don't have a configuration screen
    ConnectContext.onConfigure.configCallback = NULL;
    ConnectContext.onConfigure.configContext = NULL;

    // the "connection" member will be returned upon return
    ConnectContext.connection = LGLCD_INVALID_CONNECTION;

    // ... and connect
    hLCD = lgLcdConnect( &ConnectContext );

	if( !IsValidLCDHandle( hLCD ) )
		return true;

    // now we are connected (and have a connection handle returned),
    // let's enumerate an LCD (the first one, index = 0)
    lgLcdDeviceDesc DeviceDescription;
    hLCD = lgLcdEnumerate( ConnectContext.connection, 0, &DeviceDescription );

	if( !IsValidLCDHandle( hLCD ) )
		return true;

    // at this point, we have an LCD
	Msg( "LG LCD Found: %dx%d Pixels, %d Bits per Pixel and %d Soft Buttons\n",
        DeviceDescription.Width, DeviceDescription.Height, DeviceDescription.Bpp,
        DeviceDescription.NumSoftButtons );

	m_bInit = true;

	return true;
}

//=========================================================
//=========================================================
void CG15LCDInterface::Shutdown( void )
{
	if( m_bInit )
		lgLcdDeInit( );
}