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
#include <vgui_controls/sectionedlistpanel.h>
#include <vgui_controls/menu.h>
#include "vguicenterprint.h"

#include "igameresources.h"

#include "insvgui.h"
#include "ins_gamerules.h"
#include "ins_player_shared.h"
#include "play_team_shared.h"
#include "insvgui.h"
#include "play_team_shared.h"
#include "ins_squad_shared.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#ifndef INSVGUI_FINAL_SQUADSELECTION
//=========================================================
//=========================================================
class CChangeSquadMenu;
//=========================================================
//=========================================================
class CSquadSelection : public Panel, public IViewPortPanel, public IINSSquadSelection
{
private:
	DECLARE_CLASS_SIMPLE( CSquadSelection, Panel );

public:
	CSquadSelection( IViewPort *pViewPort );

	int GetLastTeamID( void ) const { return m_iLastTeamID; }

	// IViewPortPanel
	const char *GetName( void ) { return PANEL_SQUADSELECT; }
	void SetData( KeyValues *pData ) { }
	void Update( void ) { }
	bool NeedsUpdate( void ) { return false; }
	bool HasInputElements( void ) { return true; }
	void Reset( void );
	void ShowPanel( bool bShow );
	
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible( void ) { return BaseClass::IsVisible(); }
  	virtual void SetParent( VPANEL Parent ) { BaseClass::SetParent( Parent ); }

private:
	void TeamUpdate( int iTeamID );
	void SquadUpdate( void );

	void ShowMenu( int iMenuID, bool bState );
	void ShowSquadMenu( bool bState );

	void UpdateMenu( int iMenuID, bool bForce );

private:
	int m_iLastTeamID;
};

//=========================================================
//=========================================================
bool SquadDataLess( const int &iLeft, const int &iRight )
{
	return ( iLeft > iRight );
}

//=========================================================
//=========================================================
class CChangeSquadMenu : public IINSMenu
{
private:
	CUtlMap< int, EncodedSquadData_t > m_SquadData;

public:
	CChangeSquadMenu( )
	{
		m_SquadData.SetLessFunc( SquadDataLess );
	}

	const char *GetTitle(void) const
	{
		return "Squad Menu";
	}

	bool IgnoreResetHUD( void ) const
	{
		return true;
	}

	void Setup(void)
	{
		IINSMenuManager *pMenuManager = GetINSVGUIHelper( )->GetMenuManager( );
		IGameResources *gr = GameResources();

		if( !pMenuManager || !gr )
			return;

		CSquadSelection *pSquadSelection = ( CSquadSelection* )gViewPortInterface->FindPanelByName( PANEL_SQUADSELECT );

		if( !pSquadSelection )
			return;

		C_PlayTeam *pTeam = GetGlobalPlayTeam( pSquadSelection->GetLastTeamID( ) );
		
		if( !pTeam )
			return;

		for( int i = 0; i < pTeam->GetSquadCount( ); i++ )
		{
			C_INSSquad *pSquad = pTeam->GetSquad( i );

			if( !pSquad )
				continue;

			pMenuManager->AddSection( i, pSquad->GetName( ) );

			const char *pszName;
			char szName[ 64 ];

			for( int j = 0; j < MAX_SQUAD_SLOTS; j++ )
			{
				bool bSelectable = true;

				if( !pSquad->IsSlotEmpty( j ) )
				{
					bSelectable = false;
					pszName = gr->GetPlayerName( pSquad->GetPlayerID( j ) );
				}
				else
				{
					CPlayerClass *pClass = pSquad->GetClass( j );
					Q_snprintf( szName, sizeof( szName ), "Empty ( %s )", ( pClass ? pClass->GetName( ) : "None" ) );

					pszName = szName;
				}

				int iItemID = pMenuManager->AddItem( i, pszName, NULL, bSelectable );

				SquadData_t SquadData( i, j );

				m_SquadData.Insert( iItemID, SquadData.GetEncodedData( ) );
			}
		}
	}

	bool Action( int iItemID )
	{
		CINSVGUIHelper::JoinSquad( m_SquadData[ iItemID ], true );
		return true;
	}

	void Closed( void )
	{
		CSquadSelection *pSquadSelection = ( CSquadSelection* )gViewPortInterface->FindPanelByName( PANEL_SQUADSELECT );

		if( pSquadSelection )
			pSquadSelection->ShowPanel( false );
	}
};

DECLARE_INSMENU( INSMENU_SQUADSELECTION, CChangeSquadMenu );

//=========================================================
//=========================================================
CREATE_INSVIEWPORT_PANEL( CSquadSelection );

CSquadSelection::CSquadSelection( IViewPort *pViewPort )
	: Panel( NULL, PANEL_SQUADSELECT )
{
	Reset( );

	SetPaintBackgroundEnabled( false );
}

//=========================================================
//=========================================================
void CSquadSelection::Reset( void )
{
	m_iLastTeamID = TEAM_UNASSIGNED;
}

//=========================================================
//=========================================================
void CSquadSelection::TeamUpdate( int iTeamID )
{
	m_iLastTeamID = iTeamID;
}

//=========================================================
//=========================================================
void CSquadSelection::SquadUpdate( void )
{
	UpdateMenu( INSMENU_SQUADSELECTION, false );
}

//=========================================================
//=========================================================
void CSquadSelection::ShowMenu( int iActionID, bool bState )
{
	IINSMenuManager *pMenuManager = GetINSVGUIHelper( )->GetMenuManager( );

	if( !pMenuManager )
		return;

	if( bState )
		pMenuManager->ShowMenu( iActionID );
	else if( pMenuManager->GetActiveMenu( ) == iActionID )
		pMenuManager->CloseMenu( );
}

//=========================================================
//=========================================================
void CSquadSelection::ShowSquadMenu( bool bState )
{
	ShowMenu( INSMENU_SQUADSELECTION, bState );
}

//=========================================================
//=========================================================
void CSquadSelection::UpdateMenu( int iMenuID, bool bForce )
{
	IINSMenuManager *pMenuManager = GetINSVGUIHelper( )->GetMenuManager( );

	if( !pMenuManager )
		return;

	bool bNotShown = pMenuManager->GetActiveMenu( ) != iMenuID;

	if( !pMenuManager || ( !bForce && bNotShown ) )
		return;

	if( bNotShown )
		pMenuManager->ShowMenu( iMenuID );
	else
		pMenuManager->UpdateMenu( );
}

//=========================================================
//=========================================================
void CSquadSelection::ShowPanel( bool bShow )
{
	SetVisible( bShow );
	ShowSquadMenu( bShow );
}
#endif
