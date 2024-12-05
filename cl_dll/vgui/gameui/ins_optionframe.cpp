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

#include <vgui/vgui.h>
#include <vgui/iinput.h>

#include "gameuipanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
IINSOptionField::IINSOptionField( ConVar *pOption )
{
	m_pOption = pOption;
}

//=========================================================
//=========================================================
class CINSCheckOption : public IINSOptionField
{
public:
	CINSCheckOption( ConVar *pOption, CheckButton *pControl )
		: IINSOptionField( pOption )
	{
		m_pControl = pControl;
	}

private:
	void UpdateOption( void )
	{
		m_pOption->SetValue( m_pControl->IsSelected( ) );
	}

	void UpdateControl( void )
	{
		m_pControl->SetSelected( m_pOption->GetBool( ) );
	}

private:
	CheckButton *m_pControl;
};

//=========================================================
//=========================================================
class CINSComboOption : public IINSOptionField
{
public:
	CINSComboOption( ConVar *pOption, ComboBox *pControl )
		: IINSOptionField( pOption )
	{
		m_pControl = pControl;
	}

private:
	void UpdateOption( void )
	{
		m_pOption->SetValue( m_pControl->GetActiveItem( ) );
	}

	void UpdateControl( void )
	{
		m_pControl->ActivateItemByRow( m_pOption->GetInt( ) );
	}

private:
	ComboBox *m_pControl;
};

//=========================================================
//=========================================================
CINSOptionFrame::CINSOptionFrame( const char *pszName )
	: Frame( NULL, pszName )
{
	SetParent( g_pGameUIPanel->GetVParent( ) );
	SetScheme( "SourceScheme" );
	SetSizeable( false );
}

//=========================================================
//=========================================================
CINSOptionFrame::~CINSOptionFrame( )
{
	m_Options.PurgeAndDeleteElements( );
}

//=========================================================
//=========================================================
void CINSOptionFrame::RegisterOption( ConVar *pOption, CheckButton *pControl )
{
	CINSCheckOption *pOptionInterface = new CINSCheckOption( pOption, pControl );
	m_Options.AddToTail( pOptionInterface );
}

//=========================================================
//=========================================================
void CINSOptionFrame::RegisterOption( ConVar *pOption, ComboBox *pControl )
{
	CINSComboOption *pOptionInterface = new CINSComboOption( pOption, pControl );
	m_Options.AddToTail( pOptionInterface );
}

//=========================================================
//=========================================================
void CINSOptionFrame::OnCommand( const char *pszCommand )
{
	if( FStrEq( pszCommand, "OK" ) )
	{
		for( int i = 0; i < m_Options.Count( ); i++ )
			m_Options[ i ]->UpdateOption( );
	}

	input( )->ReleaseAppModalSurface( );
	SetVisible( false );
}

//=========================================================
//=========================================================
void CINSOptionFrame::Activate( void )
{
	BaseClass::Activate( );

	input( )->SetAppModalSurface( GetVPanel( ) );

	for( int i = 0; i < m_Options.Count( ); i++ )
		m_Options[ i ]->UpdateControl( );
}