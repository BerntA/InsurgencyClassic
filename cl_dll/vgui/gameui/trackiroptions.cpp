//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include <vgui_controls/imagepanel.h>

#include "ins_optionframe.h"
#include "trackir.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define LOGO_PATH "gameui/trackir"

//=========================================================
//=========================================================
class COptionsSubMouseTrackIR : public CINSOptionFrame
{
	DECLARE_CLASS_SIMPLE( COptionsSubMouseTrackIR, CINSOptionFrame );

public:
	COptionsSubMouseTrackIR( );
	~COptionsSubMouseTrackIR( );

	void Activate( void );

private:
	MESSAGE_FUNC_PARAMS( OnCheckButtonChecked, "CheckButtonChecked", pParams );

	void EnableUpdate( void );

private:
	CheckButton *m_pEnabled;
	ComboBox *m_pModes;
	CheckButton *m_pIronsightHeadanglesIgnore;

	ImagePanel *m_pImage;
};

//=========================================================
//=========================================================
static const char *g_pszTrackIRModes[ TMODE_COUNT ] = {
	"Press to Enable",			// TMODE_PRESS
	"Press to Disable",			// TMODE_RELEASE
};

//=========================================================
//=========================================================
COptionsSubMouseTrackIR::COptionsSubMouseTrackIR( )
	: CINSOptionFrame( "OptionsSubMouseTrackIR" )
{
	// add controls
	m_pEnabled = new CheckButton( this, "cbx_enabled", "" );
	m_pModes = new ComboBox( this, "ecb_mode", TMODE_COUNT, true );
	m_pIronsightHeadanglesIgnore = new CheckButton( this, "cbx_igheadi", "" );

	m_pImage = new ImagePanel( this, "logo" );
	m_pImage->SetImage( LOGO_PATH );

	// setup controls
	m_pEnabled->AddActionSignalTarget( this );

	for( int i = 0; i < TMODE_COUNT; i++ )
		m_pModes->AddItem( g_pszTrackIRModes[ i ], NULL );

	// register controls
	RegisterOption( &trackir, m_pEnabled );
	RegisterOption( &trackir_mode, m_pModes );
	RegisterOption( &trackir_igheadi, m_pIronsightHeadanglesIgnore );

	// load resource
	LoadControlSettings( "resource/OptionsSubMouseTrackIRDlg.res" );
}

//=========================================================
//=========================================================
COptionsSubMouseTrackIR::~COptionsSubMouseTrackIR( )
{
	delete m_pEnabled;
	delete m_pModes;
}

//=========================================================
//=========================================================
void COptionsSubMouseTrackIR::Activate( void )
{
	BaseClass::Activate( );

	EnableUpdate( );
}

//=========================================================
//=========================================================
void COptionsSubMouseTrackIR::OnCheckButtonChecked( KeyValues *pParams )
{
	EnableUpdate( );
}

//=========================================================
//=========================================================
void COptionsSubMouseTrackIR::EnableUpdate( void )
{
	bool bEnabled = m_pEnabled->IsSelected( );

	m_pModes->SetEnabled( bEnabled );
	m_pIronsightHeadanglesIgnore->SetEnabled( bEnabled );
}

//=========================================================
//=========================================================
CON_COMMAND_F( vgui_showtrackir, "Show TrackIR Dialog", FCVAR_CLIENTDLL )
{
	COptionsSubMouseTrackIR *pTrackIR = new COptionsSubMouseTrackIR;
	Assert( pTrackIR );

	pTrackIR->Activate( );
} 
