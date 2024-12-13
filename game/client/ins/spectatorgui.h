//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SPECTATORGUI_H
#define SPECTATORGUI_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui/keycode.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <game/client/iviewport.h>

class KeyValues;

namespace vgui
{
	class TextEntry;
	class Button;
	class Panel;
	class ImagePanel;
	class ComboBox;
}

class CSpectatorMenu;

//=========================================================
//=========================================================
class CSpectatorGUI : public vgui::Frame, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE( CSpectatorGUI, vgui::Frame );

public:
	CSpectatorGUI( IViewPort *pViewPort );
	~CSpectatorGUI( );

	const char *GetName( void ) { return PANEL_SPECGUI; }
	void SetData( KeyValues *pData ) { }
	void Reset( void );
	void Update( void );
	bool NeedsUpdate( void ) { return true; }
	bool HasInputElements( void ) { return false; }
	void ShowPanel( bool bShow );
	
	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel( ); }
	bool IsVisible( void ) { return BaseClass::IsVisible( ); }
	void SetParent( vgui::VPANEL Parent ) { BaseClass::SetParent( Parent ); }

	bool HandleInput( const char *pszInput );
	void CloseMenu( void );
	
private:
	void PerformLayout(void);
	void ApplySchemeSettings( vgui::IScheme *pScheme );

	void SetLabelText(const char *textEntryName, const char *text);
	void SetLabelText(const char *textEntryName, wchar_t *text);

	bool ShouldShowPlayerLabel( int iSpecMode );

private:
	IViewPort *m_pViewPort;

	vgui::Panel *m_pTopBar, *m_pBottomBar;

	vgui::Label *m_pPlayerLabel;

	CSpectatorMenu *m_pMenu;
};

//=========================================================
//=========================================================
extern CSpectatorGUI *g_pSpectatorGUI;

#endif // SPECTATORGUI_H
