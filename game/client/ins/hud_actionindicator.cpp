//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "in_buttons.h"
#include "iclientmode.h"
#include "hud_boxed.h"
#include "action_helper.h"
#include "basic_colors.h"

#include <vgui/vgui.h>
#include <vgui/ivgui.h>
#include <vgui/ischeme.h>
#include <vgui/isurface.h>
#include <vgui/ilocalize.h>
#include <vgui_controls/panel.h>

#include <vgui_controls/AnimationController.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define ACTION_TEXTURE_PATH "HUD/action_indicators/"

const char *g_szActionTexture[ ACTION_COUNT ] = {
	"door",			// ACTION_DOOR
    "bipod",		// ACTION_BIPOD
    "ammo",			// ACTION_WEAPON
    "cache",		// ACTION_WCACHE
	"cache"			// ACTION_AMMOBOX
};

extern ConVar showactionindicators;

//=========================================================
//=========================================================
class CHudActionIndicator : public CHudElement, public vgui::Panel, public IActionListener
{
    DECLARE_CLASS_SIMPLE( CHudActionIndicator, Panel );

public:
    CHudActionIndicator( const char *pElementName );

private:
    void Init( void );

    void Paint( void );
    bool ShouldDraw( void );

	void OnAction( int iActionID );
	void ClearAction( void );

	void Reset( void )
	{
		m_iCurrentAction = ACTION_INVALID;
		m_flAlpha = 0.0f;
	}

private:
    int m_iTextureIDs[ACTION_COUNT];
	int m_iCurrentAction;
	int m_iOldAction;

	bool IsActionMoreImportant( int iAction ) const;
	
	CPanelAnimationVar( float, m_flAlpha, "ActionIndicatorAlpha", "0" );
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHudActionIndicator );

//=========================================================
//=========================================================
CHudActionIndicator::CHudActionIndicator( const char *pElementName )
	: CHudElement(pElementName), BaseClass(NULL, "HudActionIndicator")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	vgui::HScheme scheme = vgui::scheme( )->LoadSchemeFromFile( "resource/ClientScheme.res", "ClientScheme" );
	SetScheme( scheme );

	memset( m_iTextureIDs, 0, sizeof( m_iTextureIDs ) );

	SetPaintBackgroundEnabled( false );
	SetHiddenBits( HIDEHUD_DEAD );

	m_iCurrentAction = ACTION_INVALID;
	m_iOldAction = ACTION_INVALID;
	m_flAlpha = 0.0f;
}

//=========================================================
//=========================================================
void CHudActionIndicator::Init( void )
{
	char szPath[128];

	// Load textures
    for(int i = 0; i < ACTION_COUNT; i++)
    {
        if(!g_szActionTexture[i])
			continue;

		Q_strncpy( szPath, ACTION_TEXTURE_PATH, sizeof( szPath ) );
		Q_strncat( szPath, g_szActionTexture[i], sizeof( szPath ), COPY_ALL_CHARACTERS );

		m_iTextureIDs[i] = vgui::surface( )->CreateNewTextureID( );
        vgui::surface( )->DrawSetTextureFile( m_iTextureIDs[i], szPath, false, false );
    }
}

//=========================================================
//=========================================================
bool CHudActionIndicator::ShouldDraw( void )
{
	return ( CHudElement::ShouldDraw( ) && m_iCurrentAction != ACTION_INVALID && m_flAlpha != 0.0f );
}

//=========================================================
//=========================================================
void CHudActionIndicator::Paint( void )
{
    int iWide, iTall;
    GetSize( iWide, iTall );

	int iAction = m_iCurrentAction;

	if(iAction == ACTION_INVALID) // Then we are fading out and we need to draw the old action
		iAction = m_iOldAction;

	// Draw image
    surface( )->DrawSetColor( Color( 255, 255, 255, m_flAlpha ) );
	surface( )->DrawSetTexture( m_iTextureIDs[ iAction ] );
	surface( )->DrawTexturedRect( 0, 0, iWide, iTall );
}

//=========================================================
//=========================================================
void CHudActionIndicator::OnAction( int iActionID )
{
	if(iActionID == m_iCurrentAction)
		return;

	// if there is no action, just reset
	if(iActionID == ACTION_INVALID)
	{
		ClearAction( );
		return;
	}

	// Check if the new action is more important than the old one
	if(!IsActionMoreImportant( iActionID ))
		return;

	// set new action
	m_iCurrentAction = iActionID;

	// Lets bring the hud panel
	g_pClientMode->GetViewportAnimationController( )->StartAnimationSequence( "ShowActionIndicator" );
}

//=========================================================
//=========================================================
void CHudActionIndicator::ClearAction( void )
{
	// Cancel the current action
	m_iOldAction = m_iCurrentAction;
	m_iCurrentAction = ACTION_INVALID;
	
	// And hide the hud panel
	g_pClientMode->GetViewportAnimationController( )->StartAnimationSequence( "HideActionIndicator" );
}

//=========================================================
//=========================================================
bool CHudActionIndicator::IsActionMoreImportant( int iAction ) const
{
	return iAction > m_iCurrentAction;
}
