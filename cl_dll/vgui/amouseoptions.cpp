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
#include <vgui_controls/frame.h>
#include <vgui_controls/checkbutton.h>
#include <vgui_controls/combobox.h>
#include "gameuipanel.h"
#include <convar.h>
#include <keyvalues.h>

#include "ins_utils.h"
#include "command_register.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
extern ConVar trackir;
extern CINSConVar pfiremode;
extern CINSConVar stancemode;
extern CINSConVar ironsighthold;

//=========================================================
//=========================================================
class OptionsSubMouseAdvanced : public Frame
{
	DECLARE_CLASS_SIMPLE( OptionsSubMouseAdvanced, Frame );

public:
	OptionsSubMouseAdvanced( );
	~OptionsSubMouseAdvanced( );

	void OnCommand( const char *pszCommand );
	void Activate( void );

private:
	CheckButton	*m_pTrackIR;
	CheckButton	*m_pStanceMode;
	CheckButton	*m_pIronsightMode;
	ComboBox *m_pPFireMode;
};

//=========================================================
//=========================================================
OptionsSubMouseAdvanced::OptionsSubMouseAdvanced( )
	: Frame( NULL, "OptionsSubMouseAdvancedDlg" )
{
	Assert( g_pGameUIPanel );

	SetParent( g_pGameUIPanel->GetVParent( ) );
	SetScheme( "SourceScheme" );
	SetSizeable( false );

	// add controls
	m_pTrackIR = new CheckButton( this, "cbx_trackir", "" );
	m_pStanceMode = new CheckButton( this, "cbx_stancemode", "" );
	m_pIronsightMode = new CheckButton( this, "cbx_ironsighthold", "" );

	m_pPFireMode = new ComboBox( this, "ecb_pfiremode", FIREMODE_COUNT, true );

	for( int i = 0; i < FIREMODE_COUNT; i++ )
		m_pPFireMode->AddItem( g_pszFireModeNames[ i ], NULL );

	// load resource
	LoadControlSettings( "resource/OptionsSubMouseAdvancedDlg.res" );
}

//=========================================================
//=========================================================
OptionsSubMouseAdvanced::~OptionsSubMouseAdvanced()
{
	delete m_pTrackIR;
	delete m_pStanceMode;
	delete m_pIronsightMode;
	delete m_pPFireMode;
}

//=========================================================
//=========================================================
void OptionsSubMouseAdvanced::OnCommand(const char *pszCommand)
{
	if( FStrEq( pszCommand, "OK" ) )
	{
		trackir.SetValue( m_pTrackIR->IsSelected( ) );
		stancemode.SetValue( m_pStanceMode->IsSelected( ) );
		ironsighthold.SetValue( m_pIronsightMode->IsSelected( ) );
		pfiremode.SetValue( m_pPFireMode->GetActiveItem( ) );
	}

	input()->ReleaseAppModalSurface( );
	SetVisible( false );
}

//=========================================================
//=========================================================
void OptionsSubMouseAdvanced::Activate( void )
{
	BaseClass::Activate( );

	input( )->SetAppModalSurface( GetVPanel( ) );

	m_pTrackIR->SetSelected( trackir.GetBool( ) );
	m_pStanceMode->SetSelected( stancemode.GetBool( ) );
	m_pIronsightMode->SetSelected( ironsighthold.GetBool( ) );
	m_pPFireMode->ActivateItemByRow( pfiremode.GetInt( ) );
}

//=========================================================
//=========================================================
CON_COMMAND_F( vgui_showadvmouse, "Show Advanced Mouse Dialog", FCVAR_CLIENTDLL )
{
	(new OptionsSubMouseAdvanced())->Activate();
}