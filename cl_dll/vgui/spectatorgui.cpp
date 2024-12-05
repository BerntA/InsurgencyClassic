//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <cdll_client_int.h>
#include <globalvars_base.h>
#include <cdll_util.h>
#include <KeyValues.h>

#include "spectatorgui.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IPanel.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/MenuItem.h>

#include <cl_dll/iviewport.h>
#include "commandmenu.h"
#include "hltvcamera.h"

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Menu.h>
#include "IGameUIFuncs.h" // for key bindings
#include <imapoverview.h>
#include <shareddefs.h>
#include <igameresources.h>

#include "basic_colors.h"
#include "inshud.h"
#include "c_playerresource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
CSpectatorGUI *g_pSpectatorGUI = NULL;

#define BLACK_BAR_COLOR Color( 0, 0, 0, 196 )

ConVar cl_spec_mode("cl_spec_mode", "1", FCVAR_ARCHIVE | FCVAR_USERINFO, "spectator mode");

//=========================================================
//=========================================================
class CSpecButton : public Button
{
public:
	CSpecButton( Panel *pParent, const char *pszPanelName )
		: Button( pParent, pszPanelName, "" )
	{
	}

private:
	void ApplySchemeSettings( IScheme *pScheme )
	{
		Button::ApplySchemeSettings( pScheme );
		SetFont( pScheme->GetFont( "Marlett", IsProportional( ) ) );
	}
};

//=========================================================
//=========================================================
class CSpectatorTopbar : public Panel
{
	DECLARE_CLASS_SIMPLE( CSpectatorTopbar, Panel );

public:
	CSpectatorTopbar( Panel *pParent )
		: Panel( pParent, "topbar" )
	{
		m_ReinforcementFont = INVALID_FONT;
	}

private:
	void OnThink( void )
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );
		SetVisible( pPlayer && ( pPlayer->GetTeamID( ) != TEAM_SPECTATOR ) );
	}

	void ApplySchemeSettings( IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );

		m_ReinforcementFont = pScheme->GetFont( "Trebuchet18", true );
	}

	void OnSizeChanged( int iWide, int iTall )
	{
		BaseClass::OnSizeChanged( iWide, iTall );

		InvalidateLayout( );
	}

	void PerformLayout( void )
	{
		if( m_ReinforcementFont == INVALID_FONT )
			return;

		m_iReinforcementYPos = ( GetTall( ) * 0.5f ) - ( surface( )->GetFontTall( m_ReinforcementFont ) * 0.5f );
	}

	void Paint( void )
	{
		char szReinforcementBuffer[ 128 ];

		if( !GetINSHUDHelper( )->CreateReinforcementVGUITimer( szReinforcementBuffer, sizeof( szReinforcementBuffer ) ) )
			return;

		wchar_t wszReinforcementBuffer[ 128 ];
		localize( )->ConvertANSIToUnicode( szReinforcementBuffer, wszReinforcementBuffer, sizeof( wszReinforcementBuffer ) );
		int iReinforcementLength = Q_strlen( szReinforcementBuffer );

		int iReinforcementXPos = ( GetWide( ) * 0.5f ) - ( UTIL_ComputeStringWidth( m_ReinforcementFont, wszReinforcementBuffer ) * 0.5f );

		surface( )->DrawSetTextFont( m_ReinforcementFont );
		surface( )->DrawSetTextPos( iReinforcementXPos, m_iReinforcementYPos );
		surface( )->DrawSetTextColor( COLOR_WHITE );
		surface( )->DrawPrintText( wszReinforcementBuffer, iReinforcementLength );
	}

private:
	HFont m_ReinforcementFont;
	int m_iReinforcementYPos;
};

//=========================================================
//=========================================================
class CSpectatorMenu : public Frame
{
	DECLARE_CLASS_SIMPLE( CSpectatorMenu, Frame );

public:
	CSpectatorMenu( CSpectatorGUI *pParent );
	~CSpectatorMenu() { }

	void Reset( void );
	void OnThink( void );
	void Update( void );
	void ShowPanel( bool bState );

private:
	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pData );
	void OnCommand( const char *pszCommand );
	void OnKeyCodePressed( KeyCode code );
	void ApplySchemeSettings( IScheme *pScheme );
	void PerformLayout( void );

	void SetPlayerNameText( const wchar_t *pwszText ); 
	void SetPlayerFgColor( Color PlayerColor );
	int PlayerAddItem( int iItemID, wchar_t *pwszName, KeyValues *pKV );

private:
	ComboBox *m_pPlayerList;
	ComboBox *m_pViewOptions;
	ComboBox *m_pConfigSettings;

	Button *m_pLeftButton;
	Button *m_pRightButton;

	int m_iHideKey;
};

//=========================================================
//=========================================================
CSpectatorMenu::CSpectatorMenu( CSpectatorGUI *pParent )
	: Frame( pParent, "specmenu" )
{
	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	SetTitleBarVisible( false );
	SetMoveable( false );
	SetSizeable( false );
	SetProportional( true );

	SetScheme( "ClientScheme" );

	m_iHideKey = -1;

	m_pPlayerList = new ComboBox( this, "playercombo", 10 , false );
	m_pViewOptions = new ComboBox( this, "viewcombo", 10 , false );
	m_pConfigSettings = new ComboBox( this, "settingscombo", 10 , false );	

	m_pLeftButton = new CSpecButton( this, "specprev" );
	m_pLeftButton->SetText( "3" );
	m_pRightButton = new CSpecButton( this, "specnext" );
	m_pRightButton->SetText( "4" );

	m_pPlayerList->SetText( "" );
	m_pViewOptions->SetText( "#Spec_Modes" );
	m_pConfigSettings->SetText( "#Spec_Options" );

	m_pPlayerList->SetOpenDirection( Menu::MenuDirection_e::UP );
	m_pViewOptions->SetOpenDirection( Menu::MenuDirection_e::UP );
	m_pConfigSettings->SetOpenDirection( Menu::MenuDirection_e::UP );

	// create view config menu
	CommandMenu *pMenu = NULL;
	
	pMenu = new CommandMenu( m_pViewOptions, "spectatormenu", gViewPortInterface );
	pMenu->LoadFromFile( "resource/ui/misc/spectatormenu.res" );
	m_pConfigSettings->SetMenu( pMenu );

	// create view mode menu
	pMenu = new CommandMenu( m_pViewOptions, "spectatormodes", gViewPortInterface );
	pMenu->LoadFromFile( "resource/ui/misc/spectatormodes.res" );
	m_pViewOptions->SetMenu( pMenu );

	LoadControlSettings( "resource/ui/misc/spectatorcontrols.res" );
}

//=========================================================
//=========================================================
void CSpectatorMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// need to MakeReadyForUse() on the menus so we can set their bg color before they are displayed
	m_pConfigSettings->GetMenu()->MakeReadyForUse( );
	m_pViewOptions->GetMenu()->MakeReadyForUse( );
	m_pPlayerList->GetMenu()->MakeReadyForUse( );

	m_pConfigSettings->GetMenu()->SetBgColor( BLACK_BAR_COLOR );
	m_pViewOptions->GetMenu()->SetBgColor( BLACK_BAR_COLOR );
	m_pPlayerList->GetMenu()->SetBgColor( BLACK_BAR_COLOR );
}

//=========================================================
//=========================================================
void CSpectatorMenu::PerformLayout( void )
{
	int iW, iH;
	GetHudSize( iW, iH );

	// fill the screen
	SetSize( iW, GetTall( ) );
}

//=========================================================
//=========================================================
void CSpectatorMenu::Reset( void )
{
	m_pPlayerList->DeleteAllItems( );
}

//=========================================================
//=========================================================
void CSpectatorMenu::Update( void )
{
	int itemID = 0;

	Reset();

	if( m_iHideKey < 0 )
		m_iHideKey = gameuifuncs->GetEngineKeyCodeForBind( "crouch" );

	IGameResources *gr = GameResources();
	
	if ( !gr )
		return;

	int iPlayerIndex;
	for ( iPlayerIndex = 1 ; iPlayerIndex <= gpGlobals->maxClients; iPlayerIndex++ )
	{

		// does this slot in the array have a name?
		if ( !gr->IsConnected( iPlayerIndex ) )
			continue;
			
		if ( gr->IsLocalPlayer( iPlayerIndex ) )
			continue;

		if ( !gr->IsAlive( iPlayerIndex ) )
			continue;

		wchar_t playerText[ 80 ], playerName[ 64 ], *team, teamText[ 64 ];
		char localizeTeamName[64];
		localize()->ConvertANSIToUnicode( UTIL_SafeName( gr->GetPlayerName(iPlayerIndex) ), playerName, sizeof( playerName ) );
		const char * teamname = gr->GetTeamName( gr->GetTeam(iPlayerIndex) );
		if ( teamname )
		{	
			Q_snprintf( localizeTeamName, sizeof( localizeTeamName ), "#%s", teamname );
			team=localize()->Find( localizeTeamName );

			if ( !team ) 
			{
				localize()->ConvertANSIToUnicode( teamname , teamText, sizeof( teamText ) );
				team = teamText;
			}

			localize()->ConstructString( playerText, sizeof( playerText ), localize()->Find( "#Spec_PlayerItem_Team" ), 2, playerName, team );
		}
		else
		{
			localize()->ConstructString( playerText, sizeof( playerText ), localize()->Find( "#Spec_PlayerItem" ), 1, playerName );
		}

		KeyValues *kv = new KeyValues("UserData", "player", gr->GetPlayerName( iPlayerIndex ) );
		itemID = PlayerAddItem( itemID, playerText, kv ); // -1 means a new slot
		kv->deleteThis();
	}

	// make sure the player combo box is up to date
	int playernum = GetSpectatorTarget();
	const char *selectedPlayerName = gr->GetPlayerName( playernum );
	for ( iPlayerIndex=0; iPlayerIndex<m_pPlayerList->GetItemCount(); ++iPlayerIndex )
	{
		KeyValues *kv = m_pPlayerList->GetItemUserData( iPlayerIndex );
		if ( kv && FStrEq( kv->GetString( "player" ), selectedPlayerName ) )
		{
			m_pPlayerList->ActivateItemByRow( iPlayerIndex );
			m_pPlayerList->SetText( selectedPlayerName );
			break;
		}
	}
}

//=========================================================
//=========================================================
void CSpectatorMenu::OnThink()
{
	BaseClass::OnThink();

	IGameResources *gr = GameResources();
	if ( !gr )
		return;

	// make sure the player combo box is up to date
	int playernum = GetSpectatorTarget();
	const char *selectedPlayerName = gr->GetPlayerName( playernum );
	const char *currentPlayerName = "";
	KeyValues *kv = m_pPlayerList->GetActiveItemUserData();
	if ( kv )
	{
		currentPlayerName = kv->GetString("player");
	}
	if ( !FStrEq( currentPlayerName, selectedPlayerName ) )
	{
		for ( int i=0; i<m_pPlayerList->GetItemCount(); ++i )
		{
			KeyValues *kv = m_pPlayerList->GetItemUserData( i );
			if ( kv && FStrEq( kv->GetString( "player" ), selectedPlayerName ) )
			{
				m_pPlayerList->ActivateItemByRow( i );
				m_pPlayerList->SetText( selectedPlayerName );
				break;
			}
		}
	}
}

//=========================================================
//=========================================================
void CSpectatorMenu::OnTextChanged( KeyValues *pData )
{
	Panel *pPanel = reinterpret_cast< vgui::Panel* >( pData->GetPtr( "panel" ) );

	vgui::ComboBox *pBox = dynamic_cast< vgui::ComboBox* >( pPanel );

	// don't change the text in the config setting combo
	if( pBox == m_pConfigSettings )
	{
		m_pConfigSettings->SetText( "#Spec_Options" );
	}
	else if ( pBox == m_pPlayerList )
	{
		// PNOTE: this gets updated whenever, and causes weirdness
		// when it comes to clicking players, always keeps swapping etc
		/*KeyValues *pKV = pBox->GetActiveItemUserData( );

		if ( pKV && GameResources( ) )
		{
			const char *pszPlayer = pKV->GetString( "player" );

			int iCurrentPlayerNum = GetSpectatorTarget( );
			const char *pszCurrentPlayerName = GameResources( )->GetPlayerName( iCurrentPlayerNum );

			if ( !FStrEq( pszCurrentPlayerName, pszPlayer ) )
			{
				char szCommand[ 128 ];
				Q_snprintf( szCommand, sizeof( szCommand ), "spec_player \"%s\"", pszPlayer );
				engine->ClientCmd( szCommand );
			}
		}*/
	}
}

//=========================================================
//=========================================================
void CSpectatorMenu::OnCommand( const char *pszCommand )
{
	if( !stricmp( pszCommand, "specnext" ) )
		engine->ClientCmd("spec_next");
	else if( !stricmp( pszCommand, "specprev" ) )
		engine->ClientCmd("spec_prev");
}

//=========================================================
//=========================================================
void CSpectatorMenu::OnKeyCodePressed( KeyCode code )
{
	// we can't compare the keycode to a known code, because translation from bound keys
	// to vgui key codes is not 1:1. Get the engine version of the key for the binding
	// and the actual pressed key, and compare those..
	int iLastTrappedKey = engine->GetLastPressedEngineKey();	// the enginekey version of the code param

	if( iLastTrappedKey == m_iHideKey )
		ShowPanel( false );
}

//=========================================================
//=========================================================
void CSpectatorMenu::ShowPanel( bool bShow )
{
	if ( BaseClass::IsVisible( ) == bShow )
		return;

	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );
		SetKeyBoardInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}

	// during HLTV live broadcast, some interactive elements are disabled
	bool bIsEnabled = ( !engine->IsHLTV() || engine->IsPlayingDemo() );
	
	m_pLeftButton->SetVisible( bIsEnabled );
	m_pRightButton->SetVisible( bIsEnabled );
	m_pPlayerList->SetVisible( bIsEnabled );
	m_pViewOptions->SetVisible( bIsEnabled );
}

//=========================================================
//=========================================================
int CSpectatorMenu::PlayerAddItem( int iItemID, wchar_t *pwszName, KeyValues *pData ) 
{
	if ( m_pPlayerList->IsItemIDValid( iItemID ) )
	{	
		m_pPlayerList->UpdateItem( iItemID, pwszName, pData );
		return iItemID + 1;
	}
	else
	{
		return m_pPlayerList->AddItem( pwszName, pData ) + 1; 
	}
}

//=========================================================
//=========================================================
void CSpectatorMenu::SetPlayerNameText( const wchar_t *pwszName )
{
	char *pszANSIText = ( char* ) _alloca( ( wcslen( pwszName ) + 1 ) * sizeof( char ) );

	if ( pszANSIText )
	{
		localize()->ConvertUnicodeToANSI( pwszName, pszANSIText, wcslen( pwszName ) + 1 );
		m_pPlayerList->SetText( pszANSIText );
	}
}

//=========================================================
//=========================================================
void CSpectatorMenu::SetPlayerFgColor( Color PlayerColor )
{
	m_pPlayerList->SetFgColor( PlayerColor );
}

//=========================================================
//=========================================================
CSpectatorGUI::CSpectatorGUI( IViewPort *pViewPort )
	: Frame( NULL, PANEL_SPECGUI )
{
	m_pViewPort = pViewPort;
	g_pSpectatorGUI = this;

	// initialize dialog
	SetVisible( false );
	SetProportional( true );
	SetTitleBarVisible( false );

	// load the new scheme early!!
	SetScheme( "ClientScheme");
	SetMoveable( false );
	SetSizeable( false );
	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( false );

	// create elements
	m_pTopBar = new CSpectatorTopbar( this );
 	m_pBottomBar = new Panel( this, "bottombar" );

	m_pPlayerLabel = new Label( this, "playerlabel", "" );
	m_pPlayerLabel->SetVisible( false );

	m_pMenu = new CSpectatorMenu( this );

	LoadControlSettings( "resource/ui/misc/spectator.res" );
	
	SetPaintBorderEnabled( false );
	SetPaintBackgroundEnabled( false );

	m_pBottomBar->SetVisible( true );
	m_pTopBar->SetVisible( true );
	m_pMenu->SetVisible( false );

	InvalidateLayout();
}

//=========================================================
//=========================================================
CSpectatorGUI::~CSpectatorGUI( )
{
	g_pSpectatorGUI = NULL;
}

//=========================================================
//=========================================================
void CSpectatorGUI::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBorder( NULL );
	SetPaintBorderEnabled( false );

	m_pTopBar->SetBgColor( BLACK_BAR_COLOR );
	m_pBottomBar->SetBgColor( BLACK_BAR_COLOR );
}

//=========================================================
//=========================================================
void CSpectatorGUI::PerformLayout( void )
{
	int iW, iH, iX, iY;
	GetHudSize( iW, iH );
	
	// fill the screen
	SetBounds( 0, 0, iW, iH );

	// stretch the bottom bar across the screen
	m_pBottomBar->GetPos( iX, iY );
	m_pBottomBar->SetSize( iW, iH - iY );
}

//=========================================================
//=========================================================
void CSpectatorGUI::ShowPanel( bool bShow )
{
	SetVisible( bShow );

	// show the sub-menu of the stuff
	if( !bShow )
		m_pMenu->ShowPanel( false );
}

//=========================================================
//=========================================================
void CSpectatorGUI::Reset( void )
{
	m_pMenu->Reset( );
}

//=========================================================
//=========================================================
void CSpectatorGUI::Update( void )
{
	if( !ValidPlayerResource( ) )
		return;

	int iWide, iTall;
	int iBX, iBY, iBWide, iBTall;

	GetHudSize( iWide, iTall );
	m_pTopBar->GetBounds( iBX, iBY, iBWide, iBTall );

	IGameResources *gr = GameResources( );
	int iSpecMode = GetSpectatorMode( );
	int iPlayerNum = GetSpectatorTarget( );

	IViewPortPanel *pOverview = gViewPortInterface->FindPanelByName( PANEL_OVERVIEW );

	if ( pOverview && pOverview->IsVisible() )
	{
		int iMX, iMY, iMWide, iMTall;

		VPANEL p = pOverview->GetVPanel();
		vgui::ipanel()->GetPos( p, iMX, iMY );
		vgui::ipanel()->GetSize( p, iMWide, iMTall );
				
		if ( iMY < iMTall )
		{
			// reduce to bar 
			m_pTopBar->SetSize( iWide - ( iMX + iMWide ), iBTall );
			m_pTopBar->SetPos( ( iMX + iMWide ), 0 );
		}
		else
		{
			// full top bar
			m_pTopBar->SetSize( iWide, iBTall );
			m_pTopBar->SetPos( 0, 0 );
		}
	}
	else
	{
		// full top bar
		m_pTopBar->SetSize( iWide, iBTall );
		m_pTopBar->SetPos( 0, 0 );
	}

	m_pPlayerLabel->SetVisible( ShouldShowPlayerLabel( iSpecMode ) );

	// update player name, text and color
	if ( gr && iPlayerNum > 0 && iPlayerNum <= gpGlobals->maxClients )
	{
		// player's team color
		Color PlayerColor = gr->GetTeamColor( gr->GetTeam( iPlayerNum ) );

		m_pPlayerLabel->SetFgColor( PlayerColor );
		
		wchar_t wszPlayerText[ 80 ], wszPlayerName[ 64 ], wszHealth[ 10 ];

		memset( wszPlayerText, 0, sizeof( wszPlayerText ) );
		memset( wszPlayerName, 0, sizeof( wszPlayerName ) );

		localize()->ConvertANSIToUnicode( UTIL_SafeName( gr->GetPlayerName( iPlayerNum ) ), wszPlayerName, sizeof( wszPlayerName ) );

		int iHealth = gr->GetHealth( iPlayerNum );

		if ( iHealth > 0 && gr->IsAlive( iPlayerNum ) )
		{
			_snwprintf( wszHealth, sizeof( wszHealth ), L"%i", iHealth );
			localize()->ConstructString( wszPlayerText, sizeof( wszPlayerText ), localize()->Find( "#Spec_PlayerItem_Team" ), 2, wszPlayerName, wszHealth );
		}
		else
		{
			localize()->ConstructString( wszPlayerText, sizeof( wszPlayerText ), localize()->Find( "#Spec_PlayerItem" ), 1, wszPlayerName );
		}

		m_pPlayerLabel->SetText( wszPlayerText );
	}
	else
	{
		m_pPlayerLabel->SetText( L"" );
	}

	// update menu
	m_pMenu->Update( );
}

//=========================================================
//=========================================================
bool CSpectatorGUI::HandleInput( const char *pszInput )
{
	if( Q_strcmp( pszInput, "+attack" ) == 0 )
	{
		engine->ClientCmd( "spec_next" );
		return false;
	}
	else if( Q_strcmp( pszInput, "+special2" ) == 0)
	{
		engine->ClientCmd( "spec_prev" );
		return false;
	}
	else if( Q_strcmp( pszInput, "+jump" ) == 0)
	{
		engine->ClientCmd( "spec_mode" );
		return false;
	}
	else if( Q_strcmp( pszInput, "+crouch" ) == 0)
	{
		m_pMenu->ShowPanel( true );
		return false;
	}

	return true;
}

//=========================================================
//=========================================================
void CSpectatorGUI::CloseMenu( void )
{
	if( m_pMenu->IsVisible( ) )
		m_pMenu->ShowPanel( false );
}

//=========================================================
//=========================================================
bool CSpectatorGUI::ShouldShowPlayerLabel( int iSpecMode )
{
	return ( ( iSpecMode == OBS_MODE_IN_EYE ) || ( iSpecMode == OBS_MODE_CHASE ) );
}

//=========================================================
//=========================================================
static void ForwardSpecCmdToServer( void )
{
	if( engine->IsPlayingDemo( ) )
		return;

	if( engine->Cmd_Argc() == 1 )
	{
		// just forward the command without parameters
		engine->ServerCmd( engine->Cmd_Argv(0) );
	}
	else if( engine->Cmd_Argc() == 2 )
	{
		// forward the command with parameter
		char szCommand[128];
		Q_snprintf( szCommand, sizeof( szCommand ), "%s \"%s\"", engine->Cmd_Argv( 0 ), engine->Cmd_Argv( 1 ) );
		engine->ServerCmd( szCommand );
	}
}

//=========================================================
//=========================================================
CON_COMMAND( spec_next, "Spectate next player" )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( engine->IsHLTV() && engine->IsPlayingDemo() )
	{
		// handle the command clientside
		HLTVCamera()->SpecNextPlayer( false );
	}
	else
	{
		ForwardSpecCmdToServer();
	}
}

//=========================================================
//=========================================================
CON_COMMAND(spec_menu, "close spec menu")
{
	if( !g_pSpectatorGUI )
		return;

	g_pSpectatorGUI->CloseMenu( );
}

//=========================================================
//=========================================================
CON_COMMAND( spec_prev, "Spectate previous player" )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( engine->IsHLTV() && engine->IsPlayingDemo() )
	{
		// handle the command clientside
		HLTVCamera()->SpecNextPlayer( true );
	}
	else
	{
		ForwardSpecCmdToServer();
	}
}

//=========================================================
//=========================================================
CON_COMMAND( spec_mode, "Set spectator mode" )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( engine->IsHLTV() )
	{
		int mode;

		if ( engine->Cmd_Argc() == 2 )
		{
			mode = Q_atoi( engine->Cmd_Argv(1) );
			HLTVCamera()->SetMode( mode );
		}
		else if ( engine->IsPlayingDemo() )
		{
			// during HLTV demo playback we all all spectator modes
			mode = HLTVCamera()->GetMode()+1;

			if ( mode > OBS_MODE_ROAMING )
				mode = OBS_MODE_IN_EYE;
			
			// handle the command clientside
			HLTVCamera()->SetMode( mode );
		}
		else
		{
			HLTVCamera()->ToggleChaseAsFirstPerson();
		}
	}
	else
	{
		ForwardSpecCmdToServer();
	}
}

//=========================================================
//=========================================================
CON_COMMAND( spec_player, "Spectate player by name" )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( engine->Cmd_Argc() != 2 )
		return;

	if ( engine->IsHLTV() && engine->IsPlayingDemo() )
	{
		// handle the command clientside
		HLTVCamera()->SpecNamedPlayer( engine->Cmd_Argv(1) );
	}
	else
	{
		ForwardSpecCmdToServer();
	}
}