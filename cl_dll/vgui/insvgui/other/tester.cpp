//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include <vgui_controls/frame.h>
#include <vgui_controls/html.h>
#include <vgui_controls/button.h>

#include <cl_dll/iviewport.h>

#include "insvgui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
class CTesterWindow : public Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CTesterWindow, Frame );

public:
	CTesterWindow( IViewPort *pViewPort );

private:
	const char *GetName( void ) { return PANEL_TESTERNOTICE; }
	void SetData( KeyValues *pData ) { }
	void Reset( void );
	void Update( void ) { }
	bool NeedsUpdate( void ) { return false; }
	bool HasInputElements( void ) { return true; }
	void ShowPanel( bool bShow );

	void Activate( void );
	void OnCommand( const char *pszCommand );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel( ); }
  	virtual bool IsVisible( void ) { return BaseClass::IsVisible( ); }
  	virtual void SetParent( VPANEL Parent ) { BaseClass::SetParent( Parent ); }

protected:	
	IViewPort *m_pViewPort;

	HTML *m_pHTMLMessage;
	Button *m_pOK;
};

//=========================================================
//=========================================================
CREATE_INSVIEWPORT_PANEL( CTesterWindow );

CTesterWindow::CTesterWindow( IViewPort *pViewPort )
	: Frame( NULL, PANEL_TESTERNOTICE )
{
	// initialize dialog
	m_pViewPort = pViewPort;

	// set the title
	SetTitle( "tester notice", false );

	// load the new scheme early
	SetScheme( "ClientScheme" );
	SetMoveable( false );
	SetSizeable( false );
	SetProportional( true );

	// hide the system buttons
	SetTitleBarVisible( false );

	// create controls
	m_pHTMLMessage = new HTML( this,"HTMLMessage" );
	m_pOK = new Button( this, "ok", "#PropertyDialog_OK" );

	m_pOK->SetCommand( "okay" );
	
	// load control settings
	LoadControlSettings( "resource/ui/misc/testernotice.res" );
	
	// reset
	Reset( );
}

//=========================================================
//=========================================================
void CTesterWindow::Reset( void )
{
	Update( );
}

//=========================================================
//=========================================================
#define TESTERNOTICE_URL "http://www.insmod.net/testernotice.php"

void CTesterWindow::Activate( void )
{
	BaseClass::Activate( );

	m_pHTMLMessage->SetVisible( true );
	m_pHTMLMessage->OpenURL( TESTERNOTICE_URL );
}

//=========================================================
//=========================================================
void CTesterWindow::OnCommand( const char *pszCommand )
{
    if( !Q_strcmp( pszCommand, "okay" ) )
		m_pViewPort->ShowPanel( this, false );

	BaseClass::OnCommand( pszCommand );
}

//=========================================================
//=========================================================
void CTesterWindow::ShowPanel( bool bShow )
{
	if( BaseClass::IsVisible( ) == bShow )
		return;

	if ( bShow )
	{
		Activate( );
		SetMouseInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}
}