//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client DLL VGUI2 Viewport
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include <vgui_controls/panel.h>
#include <vgui/isurface.h>
#include <keyvalues.h>
#include <vgui/cursor.h>
#include <vgui/ischeme.h>
#include <vgui/ivgui.h>
#include <vgui/ilocalize.h>
#include <vgui/vgui.h>

#include "insviewport.h"
#include "insvgui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CON_COMMAND( changeteam, "Change your Team" )
{
	if( !GetINSVGUIHelper( )->CanShowTeamPanel( ) )
		return;

	gViewPortInterface->ShowPanel( PANEL_CHANGETEAM, true );
}

CON_COMMAND( changesquad, "Change your Squad" )
{
	if( !GetINSVGUIHelper( )->CanShowSquadPanel( ) )
		return;

	gViewPortInterface->ShowPanel( PANEL_SQUADSELECT, true );
}

CON_COMMAND( deathinfo, "Show Last Deathinfo" )
{
	GetINSVGUIHelper( )->ShowDeathPanel( );
}

//=========================================================
//=========================================================
void INSViewport::LevelInitPostEntity( void )
{
	for( int i = 0; i < m_Panels.Count( ); i++ )
	{
		IViewPortPanel *pViewPanel = m_Panels[ i ];

		if( !pViewPanel )
		{
			Assert( false );
			continue;
		}

		pViewPanel->Reset( );

		Panel *pPanel = dynamic_cast< Panel* >( pViewPanel );

		if( !pPanel )
		{
			Assert( false );
			continue;
		}

		pPanel->OnLevelInit( );
	}	
}

//=========================================================
//=========================================================
void INSViewport::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	gHUD.InitColors( pScheme );

	SetPaintBackgroundEnabled( false );
}

//=========================================================
//=========================================================
void INSViewport::CreateDefaultPanels( void )
{
	BaseClass::CreateDefaultPanels( );

	CINSViewportHelper::CreateAllElements( this );
}

//=========================================================
//=========================================================
void INSViewport::HideAllPanels( void )
{
	for( int i = 0; i < m_Panels.Count( ); i++ )
	{
		IViewPortPanel *pPanel = m_Panels[ i ];

		if( pPanel )
			gViewPortInterface->ShowPanel( pPanel, false );
	}
}

//=========================================================
//=========================================================
void INSViewport::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	BaseClass::OnScreenSizeChanged( iOldWide, iOldTall );

	// http://developer.valvesoftware.com/wiki/SDK_Known_Issues_List#Resizing_game_window_glitches_HUD_elements
	ReloadScheme( NULL );
}