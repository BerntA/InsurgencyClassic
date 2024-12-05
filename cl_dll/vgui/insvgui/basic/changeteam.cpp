//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include <cl_dll/iviewport.h>
#include <vgui/ilocalize.h>
#include <vgui/keycode.h>
#include <vgui_controls/frame.h>
#include <vgui_controls/button.h>

#include "insvgui.h"
#include "ins_gamerules.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#ifndef INSVGUI_FINAL_CHANGETEAM

//=========================================================
//=========================================================
class CChangeTeam : public Panel, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CChangeTeam, Panel );

public:
	CChangeTeam( IViewPort *pViewPort );

	// IViewPortPanel
	const char *GetName( void ) { return PANEL_CHANGETEAM; }
	void SetData( KeyValues *pData ) { }
	void Update( void ) { }
	bool NeedsUpdate( void ) { return false; }
	bool HasInputElements( void ) { return true; }
	void Reset( void ) { }
	void ShowPanel( bool bShow );
	
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible( ) { return BaseClass::IsVisible(); }
  	virtual void SetParent( VPANEL Parent ) { BaseClass::SetParent( Parent ); }
};

//=========================================================
//=========================================================
#define MAIN_SECTION 0

class CChangeTeamMenu : public IINSMenu
{
public:
	const char *GetTitle(void) const
	{
		return "Team Menu";
	}

	bool IgnoreResetHUD( void ) const
	{
		return true;
	}

	void Setup(void)
	{
		IINSMenuManager *pMenuManager = GetINSVGUIHelper( )->GetMenuManager( );

		if( !pMenuManager )
			return;
		
		// add section
		pMenuManager->AddSection( MAIN_SECTION, NULL );

		// add items
		for( int i = 0; i < TEAMSELECT_COUNT; i++ )
			pMenuManager->AddItem( MAIN_SECTION, GetTeamText( i ), NULL, true );
	}

	bool Action( int iItemID )
	{
		CINSVGUIHelper::JoinTeam( iItemID );
		return true;
	}

	const char *GetTeamText( int iTeamSelectID ) const
	{
		switch( iTeamSelectID )
		{
			case TEAMSELECT_ONE:
				return CINSVGUIHelper::GetTeamName( TEAM_ONE );

			case TEAMSELECT_TWO:
				return CINSVGUIHelper::GetTeamName( TEAM_TWO );

			case TEAMSELECT_AUTOASSIGN:
				return "Auto-Assign";

			case TEAMSELECT_SPECTATOR:
				return "Spectator";
		}

		return NULL;
	}

	void Closed( void )
	{
		CChangeTeam *pChangeTeam = ( CChangeTeam* )gViewPortInterface->FindPanelByName( PANEL_CHANGETEAM );

		if( pChangeTeam )
			pChangeTeam->ShowPanel( false );
	}
};

DECLARE_INSMENU( INSMENU_TEAM, CChangeTeamMenu );

//=========================================================
//=========================================================
CREATE_INSVIEWPORT_PANEL( CChangeTeam );

CChangeTeam::CChangeTeam( IViewPort *pViewPort )
	: Panel( NULL, PANEL_CHANGETEAM )
{
	SetVisible( false );
}

//=========================================================
//=========================================================
void CChangeTeam::ShowPanel( bool bShow )
{
	IINSMenuManager *pMenuManager = GetINSVGUIHelper( )->GetMenuManager( );

	if( !pMenuManager )
		return;

	if( bShow )
	{
		pMenuManager->ShowMenu( INSMENU_TEAM );
	}
	else
	{
		if( pMenuManager->GetActiveMenu( ) == INSMENU_TEAM )
			pMenuManager->CloseMenu( );
	}
}

#endif // INSVGUI_FINAL_CHANGETEAM