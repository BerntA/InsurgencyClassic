//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include <vgui/vgui.h>
#include <vgui/iinput.h>
#include <vgui_controls/button.h>
#include <vgui_controls/combobox.h>
#include <vgui_controls/frame.h>
#include <vgui_controls/label.h>
#include <vgui_controls/panel.h>
#include <vgui_controls/menu.h>
#include <vgui_controls/menuitem.h>
#include "gameuipanel.h"
#include <convar.h>
#include "musiccontrol.h"
#include <keyvalues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern ConVar snd_music_ingame;
extern ConVar snd_music_output;
extern ConVar snd_music_device;

class EventComboBox;

//=========================================================
// OptionsSubAudioMusicDlg
//=========================================================
class OptionsSubAudioMusicDlg : public Frame
{
	DECLARE_CLASS_SIMPLE( OptionsSubAudioMusicDlg, Frame );
public:

	OptionsSubAudioMusicDlg( void );
	virtual ~OptionsSubAudioMusicDlg( void );

	virtual void		OnCommand( const char *pszCommand );
	virtual void		Activate( void );
	virtual void		ApplySchemeSettings( IScheme *pScheme );
	virtual void		OnThink( void );
	void				SetupContents( bool bNoActivateOutput = false );

private:

	EventComboBox	*m_pMusicInGame;
	Panel			*m_pStatusBorder;
	Label			*m_pStatusLabel;

	int				m_iOldInGame;
};

//=========================================================
// EventComboBox
//=========================================================
class EventComboBox : public ComboBox
{
private:
	DECLARE_CLASS_SIMPLE( EventComboBox, ComboBox );


protected:
	MESSAGE_FUNC_PTR( OnMenuItemSelected2, "MenuItemSelected", panel );

public:
	EventComboBox( Panel *pParent, const char *pszPanelName, int iNumLines, bool bAllowEdit );
};

//=========================================================
//=========================================================
EventComboBox::EventComboBox( Panel *pParent, const char *pszPanelName, int iNumLines, bool bAllowEdit ) : ComboBox( pParent, pszPanelName, iNumLines, bAllowEdit )
{
}

//=========================================================
//=========================================================
void EventComboBox::OnMenuItemSelected2( Panel *pPanel )
{
	snd_music_ingame.SetValue( GetActiveItemUserData( )->GetInt( "Value" ) );
}

//=========================================================
// OptionsSubAudioMusicDlg
//=========================================================
OptionsSubAudioMusicDlg::OptionsSubAudioMusicDlg( void ) : Frame( NULL, "OptionsSubAudioMusicDlg" )
{
	SetParent( g_pGameUIPanel->GetVParent( ) );
	SetScheme( "SourceScheme" );
	SetSizeable( false );

	m_pMusicInGame = new EventComboBox( this, "cbx_snd_music_ingame", 3, false );

	m_pStatusBorder = new Panel( this, "StatusBorder" );
	m_pStatusLabel = new Label( this, "StatusLabel", "" );

	LoadControlSettings( "resource/OptionsSubAudioMusicDlg.res" );
}

//=========================================================
//=========================================================
OptionsSubAudioMusicDlg::~OptionsSubAudioMusicDlg( void )
{
	delete m_pMusicInGame;
	delete m_pStatusBorder;
	delete m_pStatusLabel;
}

//=========================================================
//=========================================================
void OptionsSubAudioMusicDlg::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pStatusBorder->SetBorder( pScheme->GetBorder( "BaseBorder" ) );
}

//=========================================================
//=========================================================
void OptionsSubAudioMusicDlg::OnCommand( const char *pszCommand )
{
	if (FStrEq( pszCommand, "Close" ))
		snd_music_ingame.SetValue( m_iOldInGame );

	
	input( )->ReleaseAppModalSurface( );

	SetVisible( false );
	//g_Music.Restart( );

	PostMessage( this, new KeyValues( "Delete" ), 0.0001f );
}

//=========================================================
//=========================================================
void OptionsSubAudioMusicDlg::Activate()
{
	BaseClass::Activate( );

	//g_Music.Restart( );
	
	m_iOldInGame = snd_music_ingame.GetInt( );
	
	SetupContents( );
	input( )->SetAppModalSurface( GetVPanel( ) );
}


void OptionsSubAudioMusicDlg::OnThink( void )
{
	BaseClass::OnThink( );

//	m_pStatusLabel->SetText( g_Music.GetStatusText( ) );
}

//=========================================================
//=========================================================
void OptionsSubAudioMusicDlg::SetupContents(bool bNoActivateOutput)
{
	m_pMusicInGame->RemoveAll( );
	m_pMusicInGame->AddItem( "No In-Game music", new KeyValues( "KV", "Value", 0 ));
	m_pMusicInGame->AddItem( "Limited In-Game music (not between spawn and death)", new KeyValues( "KV", "Value", 1 ) );
	m_pMusicInGame->AddItem( "Full In-Game music", new KeyValues( "KV", "Value", 2 ) );
	m_pMusicInGame->ActivateItem( snd_music_ingame.GetInt( ) );
}

//////////////////////////////////////////////////////////////////////////
// Console Command
//////////////////////////////////////////////////////////////////////////
CON_COMMAND_F( vgui_showmusic, "Show advanced music dialog", FCVAR_CLIENTDLL )
{
	(new OptionsSubAudioMusicDlg())->Activate( );
}