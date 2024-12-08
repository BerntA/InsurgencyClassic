//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include <game/client/iviewport.h>
#include <vgui/isurface.h>
#include <vgui/ipanel.h>
#include <vgui/ivgui.h>
#include <vgui/iinput.h>
#include <vgui_controls/button.h>

#include "gameuipanel.h"
#include "ins_loadingpanel.h"

#include "imc_config.h"
#include "ins_player_shared.h"
#include "musicManager.h"

#include <keyvalues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
CLoadElementDimensions::CLoadElementDimensions( )
{
	m_iXPos = m_iYPos = 0;
	m_iWide = m_iTall = 0;
}

//=========================================================
//=========================================================
void CLoadElementDimensions::Init( int iXPos, int iYPos, int iWide, int iTall )
{
	m_iXPos = iXPos;
	m_iYPos = iYPos;
	m_iWide = iWide;
	m_iTall = iTall;
}

//=========================================================
//=========================================================
void CLoadElementDimensions::Load( vgui::VPANEL Panel )
{
	ipanel( )->GetSize( Panel, m_iWide, m_iTall );	
	ipanel( )->GetPos( Panel, m_iXPos, m_iYPos );
}

//=========================================================
//=========================================================
void CLoadElementDimensions::Apply( vgui::VPANEL Panel ) const
{
	ipanel( )->SetSize( Panel, m_iWide, m_iTall );
	ipanel( )->SetPos( Panel, m_iXPos, m_iYPos );
}

//=========================================================
//=========================================================
CLoadElementBase &CLoadElements::Element( int iID )
{
	switch( iID )
	{
		case LOADELEMENT_PARENT:
			return m_Parent;

		case LOADELEMENT_PROGRESS:
			return m_Progress;

		case LOADELEMENT_CANCEL:
			return m_Cancel;

		case LOADELEMENT_INFO:
			return m_Info;
	}

	Assert( false );
	return m_Parent;
}

//=========================================================
//=========================================================
CLoadDialog::CLoadDialog( )
{
	m_bSavedElements = false;
	m_bModified = false;

	ResetPanels( );
}

//=========================================================
//=========================================================
bool CLoadDialog::IsValid( void ) const
{
	return ( m_Panels[ LOADELEMENT_PARENT ] != 0 );
}

//=========================================================
//=========================================================
void CLoadDialog::Init( vgui::VPANEL Parent )
{
	// save itself and its children
	m_Panels[ LOADELEMENT_PARENT ] = Parent;

	VPANEL Progress, CancelButton, InfoLabel;
	Progress = CancelButton = InfoLabel = 0;

	int iChildCount = ipanel( )->GetChildCount( Parent );
	VPANEL Child = 0;
	const char *pszChildName = NULL;

	for( int i = 0; i < iChildCount; i++ )
	{
		Child = ipanel( )->GetChild( Parent, i );
		pszChildName = ipanel( )->GetName( Child );

		if( FStrEq( pszChildName, "progress" ) )
		{
			Progress = Child;
			continue;
		}
		else if( FStrEq( pszChildName, "CancelButton" ) )
		{
			CancelButton = Child;
			continue;
		}
		else if( FStrEq( pszChildName, "InfoLabel" ) )
		{
			InfoLabel = Child;
			continue;
		}
	}

	if( Progress == 0 || CancelButton == 0 || InfoLabel == 0 )
	{
		Assert( false );
		return;
	}

	m_Panels[ LOADELEMENT_PROGRESS ] = Progress;
	m_Panels[ LOADELEMENT_CANCEL ] = CancelButton;
	m_Panels[ LOADELEMENT_INFO ] = InfoLabel;

	// setup elements
	SaveElements( );
}

//=========================================================
//=========================================================
void CLoadDialog::SaveElements( void )
{
	if( m_bSavedElements )
	{
		if( m_bModified )
			ResetElements( );

		return;
	}

	for( int i = 0; i < LOADELEMENT_COUNT; i++ )
		m_DefaultElements.Element( i ).Load( m_Panels[ i ] );

	m_bSavedElements = true;
}

//=========================================================
//=========================================================
void CLoadDialog::Apply( CLoadElements &Elements )
{
	Apply( Elements, false );
}

//=========================================================
//=========================================================
void CLoadDialog::Apply( CLoadElements &Elements, bool bDefault )
{
	for( int i = 0; i < LOADELEMENT_COUNT; i++ )
		Elements.Element( i ).Apply( m_Panels[ i ] );

	m_bModified = !bDefault;
}

//=========================================================
//=========================================================
void CLoadDialog::Reset( void )
{
	ResetPanels( );

	// don't need to reset elements, they don't change
	// because it's not proportional
}

//=========================================================
//=========================================================
void CLoadDialog::ResetPanels( void )
{
	memset( m_Panels, 0, sizeof( m_Panels ) );
}

//=========================================================
//=========================================================
void CLoadDialog::ResetElements( void )
{
	Apply( m_DefaultElements, true );
}

//=========================================================
//=========================================================
VPANEL CLoadDialog::GetPanel( int iID ) const
{
	return m_Panels[ iID ];
}

//=========================================================
//=========================================================
CGameUIPanel *g_pGameUIPanel;

CGameUIPanel *GameUIPanel( void )
{
	Assert( g_pGameUIPanel );
	return g_pGameUIPanel;
}

//=========================================================
//=========================================================
CGameUIPanel::CGameUIPanel( VPANEL Parent )
	: Panel( NULL, "CDLLGameUIPanel" )
{
	// do basic panel setup
	VPANEL ThisPanel = GetVPanel( );
	ipanel( )->SetParent( ThisPanel, Parent );
	ipanel( )->SetZPos( ThisPanel, -0x7FFFFFFF - 1 );
	ipanel( )->SetVisible( ThisPanel, true );

	ivgui( )->AddTickSignal( GetVPanel( ) );

	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );

	// init vars
	m_BaseGameUIPanel = 0;
	m_iLastUIChildCount = 0;

	m_bFoundOptions = false;
	m_OptionsDialog = 0;
	m_OptionsMultiplayer = m_OptionsMouse = m_OptionsAudio = 0;
	ResetButtons( );

	m_bWaitingJoinGame = false;
	m_bWaitingShowPanel = false;

	m_pLoadPanel = NULL;
}

//=========================================================
//=========================================================
void CGameUIPanel::OnThink( void )
{
	// find and store various panels and dialogs

	// ... base
	if( !m_BaseGameUIPanel )
		m_BaseGameUIPanel = UTIL_FindVChildByName( GetVParent( ), "BaseGameUIPanel", false );

	if( !m_BaseGameUIPanel )
	{
		Assert( false );
		return;
	}

	int iUIChildCount = ipanel( )->GetChildCount( m_BaseGameUIPanel );

	if( m_iLastUIChildCount != iUIChildCount )
	{
		m_iLastUIChildCount = iUIChildCount;

		// find the options dialog
		FindOptions( );
	}

	// find the buttons etc
	if( m_bFoundOptions )
	{
		HackButton( m_pStats, m_OptionsMultiplayer, "StatsButton" );
		HackButton( m_pINS, m_OptionsMultiplayer, "INSButton" );
		HackButton( m_pMusic, m_OptionsAudio, "MusicButton" );
	}

	// TODO: a little unstable, but not prioirty right now

	// call think on children
	LoadingDialogThink( );
}

//=========================================================
//=========================================================
void CGameUIPanel::FindOptions( void )
{
	VPANEL OptionsDialog = UTIL_FindVChildByName( m_BaseGameUIPanel, "OptionsDialog", false );

	if( m_OptionsDialog == OptionsDialog )
		return;

	if( m_bFoundOptions )
		ResetButtons( );

	m_bFoundOptions = false;
	m_OptionsDialog = OptionsDialog;

	if( !m_OptionsDialog )
	{
		if( m_pStats )
			m_pStats->DeletePanel( );

		if( m_pINS )
			m_pINS->DeletePanel( );

		if( m_pMusic )
			m_pMusic->DeletePanel( );

		return;
	}

	VPANEL OptionsSheetDialog = UTIL_FindVChildByName( m_OptionsDialog, "Sheet", false );

	if( !OptionsSheetDialog )
		return;

	m_pStats = new Button( NULL, "StatsButton2", "Setup", this, "ShowStats" );
	m_pINS = new Button( NULL, "INSButton2", "Setup", this, "ShowINS" );
	m_pMusic = new Button( NULL, "MusicButton2", "Music Setup ...", this , "ShowMusic" );

	m_pStats->SetEnabled( false );

	m_pStats->SetVisible( false );
	m_pINS->SetVisible( false );
	m_pMusic->SetVisible( false );

	m_OptionsMultiplayer = FindOption( OptionsSheetDialog, "ImportSprayImage" );
	m_OptionsMouse = FindOption( OptionsSheetDialog, "Mouse filter" );
	m_OptionsAudio = FindOption( OptionsSheetDialog, "TestSpeakers" );

	m_bFoundOptions = true;
}

//=========================================================
//=========================================================
vgui::VPANEL CGameUIPanel::FindOption( vgui::VPANEL Parent, const char *pszUniqueElement )
{
	VPANEL Child = UTIL_FindVChildByName( Parent, pszUniqueElement, true );
	return ( Child ? ipanel( )->GetParent( Child ) : 0 );
}

//=========================================================
//=========================================================
void CGameUIPanel::ResetButtons( void )
{
	m_pStats = m_pINS = m_pMusic = NULL;
}

//=========================================================
//=========================================================
void CGameUIPanel::HackButton( Panel *pButton, VPANEL Parent, const char *pszChildName )
{
	if( !ipanel( )->IsVisible( Parent ) )
		return;

	vgui::VPANEL Child = UTIL_FindVChildByName( Parent, pszChildName, false );

	if( !Child )
		return;

	if( pButton->GetVParent( ) == 0 )
	{
		int iXPos, iYPos;
		ipanel( )->GetPos( Child, iXPos, iYPos );

		int iWide, iTall;
		ipanel( )->GetSize( Child, iWide, iTall );

		pButton->SetParent( Parent );
		pButton->SetVisible( true );
		pButton->SetPos( iXPos, iYPos );
		pButton->SetSize( iWide, iTall );
	}

	ipanel( )->SetVisible( Child, false );
}

//=========================================================
//=========================================================
void CGameUIPanel::OnTick( void )
{
	// TODO: this needs movin
	IMusicManager::GetSingleton( ).Update( 0.0f );


	// don't do anything when we're not waiting for
	// a join-game or we are not in-game
	if( !engine->IsInGame( ) )
		return;

	// handle the state of the ldialog when in-game
	bool bIsVisible = ipanel( )->IsFullyVisible( GetVPanel( ) );


	if( m_bWaitingJoinGame )
	{
		// join server when the the loading panel didn't get hacked
		m_bWaitingJoinGame = false;

		if( !m_LoadDialog.IsModified( ) )
			JoinServer( );
	}
	if (m_bWaitingShowPanel&&!bIsVisible)
	{
		if (!m_pLoadPanel)
			m_pLoadPanel=new INSLoadingDialog( );
		m_pLoadPanel->LoadedUpdate( );
		m_bWaitingShowPanel=false;
	}
}

//=========================================================
//=========================================================
void CGameUIPanel::OnCommand( const char *pszCommand )
{
	if( FStrEq( pszCommand, "ShowStats" ) )
	{
		engine->ClientCmd( "vgui_showstats" );
	}
	else if( FStrEq( pszCommand, "ShowINS" ) )
	{
		engine->ClientCmd( "vgui_showins" );
	}
	else if( FStrEq( pszCommand, "ShowMusic" ) )
	{
		engine->ClientCmd( "vgui_showmusic" );
	}
}

//=========================================================
//=========================================================
void CGameUIPanel::LoadingDialogThink( void )
{
	// don't do anything when in-game
	if( engine->IsInGame( ) )
	{
		if( m_LoadDialog.IsValid( ) )
			m_LoadDialog.Reset( );

		return;
	}

	VPANEL LoadingDialog = UTIL_FindVChildByName( m_BaseGameUIPanel, "LoadingDialog", false );

	if( LoadingDialog == 0 )
		return;

	// handle when the load dialog is invalid
	if( !m_LoadDialog.IsValid( ) || LoadingDialog != m_LoadDialog.GetPanel( LOADELEMENT_PARENT ) )
	{
		// NOTE: "LoadingDialog" is not created until the first game load

		// init
		m_LoadDialog.Init( LoadingDialog );
	}

	if ( !ipanel( )->IsVisible( m_LoadDialog.GetPanel( LOADELEMENT_PROGRESS ) ) )
	{
		VPANEL cancel=m_LoadDialog.GetPanel(LOADELEMENT_CANCEL);
		int w1,w2,t1,t2;
		ipanel()->GetSize(LoadingDialog,w1,t1);
		ipanel()->GetSize(cancel,w2,t2);
		ipanel()->SetPos(cancel,(w1-w2)>>1,t1-(t2+(t2>>1)));
		DeleteLoadPanel();
		return;
	}

	// not loading when not visible
	if( !ipanel( )->IsFullyVisible( LoadingDialog ) )
		return;

	// create loading panel
	if( !m_pLoadPanel )
		m_pLoadPanel = new INSLoadingDialog( );

	// we know when it's ready to hack the loading panel
	// when the levelname is filled
	const char *pszLevelName = engine->GetLevelName( );

	if( pszLevelName && *pszLevelName != '\0' )
		m_pLoadPanel->LoadingUpdate( );
}

//=========================================================
//=========================================================
void CGameUIPanel::JoinServer( void )
{
	// don't show loading frame
	//m_pLoadPanel->SetVisible( false );

	// BUGBUG: if the load stuff isn't shown, a player won't be logged on
	CINSStats *pStats = GetINSStats( );
	Assert( pStats );

	if( pStats )
		pStats->SetupPlayer( );

	// tell the server to fade in etc
	C_INSPlayer::InitalSpawn( );

	// update command registers
	UTIL_UpdateCommandRegisters( );

	// don't need todo this again
	m_bWaitingJoinGame = false;
}

void CGameUIPanel::DeleteLoadPanel()
{
	if (m_pLoadPanel)
	{
		m_pLoadPanel->PostMessage(m_pLoadPanel,new KeyValues("Delete"),0.0001f);
		m_pLoadPanel=NULL;
	}
	m_LoadDialog.Reset();
}