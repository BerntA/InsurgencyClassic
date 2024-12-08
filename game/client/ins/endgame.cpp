//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include <game/client/iviewport.h>
#include <vgui_controls/frame.h>

#include "insvgui.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#ifndef INSVGUI_FINAL_ENDGAME

//=========================================================
//=========================================================
class CEndGame : public Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CEndGame, Frame );

public:
	CEndGame( IViewPort *pViewPort );

private:

	// IViewPortPanel [

	const char *GetName( void ) { return PANEL_ENDGAME; }
	void SetData( KeyValues *pData ) { }
	void Update( void ) { }
	bool NeedsUpdate( void ) { return false; }
	bool HasInputElements( void ) { return true; }
	void Reset( void ) { }
	void ShowPanel( bool bShow );

	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel( ); }
	virtual bool IsVisible( ) { return BaseClass::IsVisible( ); }
	virtual void SetParent( VPANEL Parent ) { BaseClass::SetParent( Parent ); }

	// IViewPortPanel ]
};

//=========================================================
//=========================================================
CREATE_INSVIEWPORT_PANEL( CEndGame );

CEndGame::CEndGame( IViewPort *pViewPort )
	: Frame( NULL, PANEL_ENDGAME, false )
{
	// setup panel
	SetScheme( "ClientScheme" );

	SetProportional( true );
	SetMoveable( false );
	SetSizeable( false );
	SetTitleBarVisible( false );

	// load settings
	LoadControlSettings( "resource/ui/basic/endgame.res" );
}

//=========================================================
//=========================================================
void CEndGame::ShowPanel( bool bShow )
{
	if( IsVisible( ) == bShow )
		return;

	if( bShow )
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

#endif // INSVGUI_FINAL_ENDGAME