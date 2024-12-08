//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ins_optionframe.h"
#include "ins_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
class COptionsSubMultiplayerINS : public CINSOptionFrame
{
	DECLARE_CLASS_SIMPLE( COptionsSubMultiplayerINS, CINSOptionFrame );

public:
	COptionsSubMultiplayerINS( );
	~COptionsSubMultiplayerINS( );

private:
	CheckButton *m_pAltStance;
	CheckButton *m_pAltIronsights;
	ComboBox *m_pFireMode;
	CheckButton *m_p3DScopes;
	CheckButton *m_pSwitchThrow;
};

//=========================================================
//=========================================================
COptionsSubMultiplayerINS::COptionsSubMultiplayerINS( )
	: CINSOptionFrame( "OptionsSubMultiplayerINS" )
{
	// add controls
	m_pAltStance = new CheckButton( this, "cbx_altstance", "" );
	m_pAltIronsights = new CheckButton( this, "cbx_altironsights", "" );
	m_pFireMode = new ComboBox( this, "ecb_firemode", FIREMODE_COUNT + 1, true );
	m_p3DScopes = new CheckButton( this, "cbx_3dscopes", "" );
	m_pSwitchThrow = new CheckButton( this, "cbx_switchthrow", "" );

	// setup controls
	m_pFireMode->AddItem( "None", NULL );

	for( int i = 0; i < FIREMODE_COUNT; i++ )
		m_pFireMode->AddItem( g_pszFireModeNames[ i ], NULL );

	// register controls
	RegisterOption( UTIL_CommandRegisterCommand( CMDREGISTER_STANCE ), m_pAltStance );
	RegisterOption( UTIL_CommandRegisterCommand( CMDREGISTER_IRONSIGHTHOLD ), m_pAltIronsights );
	RegisterOption( UTIL_CommandRegisterCommand( CMDREGISTER_PFIREMODE ), m_pFireMode );
	RegisterOption( UTIL_CommandRegisterCommand( CMDREGISTER_3DSCOPE ), m_p3DScopes );
	RegisterOption( UTIL_CommandRegisterCommand( CMDREGISTER_SWITCHDRAW ), m_pSwitchThrow );

	// load resource
	LoadControlSettings( "resource/OptionsSubMultiplayerINSDlg.res" );
}

//=========================================================
//=========================================================
COptionsSubMultiplayerINS::~COptionsSubMultiplayerINS( )
{
	delete m_pAltStance;
	delete m_pAltIronsights;
	delete m_pFireMode;
	delete m_p3DScopes;
	delete m_pSwitchThrow;
}

//=========================================================
//=========================================================
CON_COMMAND_F( vgui_showins, "Show INS Dialog", FCVAR_CLIENTDLL )
{
	COptionsSubMultiplayerINS *pINS = new COptionsSubMultiplayerINS;
	Assert( pINS );

	pINS->Activate( );
} 
