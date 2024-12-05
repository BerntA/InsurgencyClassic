//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#ifdef _WIN32 // no VGUI2 support under linux
#include "vgui_gamedll_int.h"
#include "ienginevgui.h"
#include <vgui/isurface.h>
#include <vgui/ivgui.h>
#include <vgui/iinput.h>
#include "tier0/vprof.h"
#include <vgui_controls/panel.h>
#include <keyvalues.h>

using namespace vgui;

#include <vgui_controls/controls.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_CreateGameDLLRootPanel( void )
{
	// Just using PANEL_ROOT in HL2 right now
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_DestroyGameDLLRootPanel( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Game specific root panel
// Output : vgui::Panel
//-----------------------------------------------------------------------------
vgui::VPANEL VGui_GetGameDLLRootPanel( void )
{
	if ( IsPC() )
	{
		vgui::VPANEL root = enginevgui->GetPanel( PANEL_GAMEDLL );
		return root;
	}
	return NULL;
}



bool VGui_Startup( CreateInterfaceFn appSystemFactory )
{
	if ( !vgui::VGui_InitInterfacesList( "GAMEDLL", &appSystemFactory, 1 ) )
		return false;

	return true;
}

bool VGui_PostInit()
{
	if ( IsPC() )
	{
		// Create any root panels for .dll
		VGUI_CreateGameDLLRootPanel();

		// Make sure we have a panel
		VPANEL root = VGui_GetGameDLLRootPanel();
		if ( !root )
		{
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void VGui_CreateGlobalPanels( void )
{
}

void VGui_Shutdown()
{
	if ( IsPC() )
	{
		VGUI_DestroyGameDLLRootPanel();

		// Make sure anything "marked for deletion"
		//  actually gets deleted before this dll goes away
		vgui::ivgui()->RunFrame();
	}
}

#endif // _WIN32

