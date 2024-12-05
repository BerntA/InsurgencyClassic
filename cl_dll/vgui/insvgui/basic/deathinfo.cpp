//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include <cl_dll/iviewport.h>
#include <vgui_controls/frame.h>
#include <vgui_controls/button.h>
#include <vgui_controls/label.h>

#include "insvgui.h"
#include "c_playerresource.h"
#include "weapon_defines.h"
#include "ins_player_shared.h"
#include "basic_colors.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#ifndef INSVGUI_FINAL_DEATHINFO

//=========================================================
//=========================================================
class CDeathInfo : public Frame, public IViewPortPanel, public IINSDeathInfo
{
private:
	DECLARE_CLASS_SIMPLE( CDeathInfo, Frame );

public:
	CDeathInfo( IViewPort *pViewPort );

private:

	// IViewPortPanel [

	const char *GetName( void ) { return PANEL_DEATHINFO; }
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

	void ApplySchemeSettings( IScheme *pScheme );
	void OnCommand( const char *pszCommand );

	// IINSDeathInfo [

	void ShowDeathInfo( void );
	void UpdateDeathInfo( void );
	void ResetDeathInfo( void );

	// IINSDeathInfo ]

	void UpdateDetails( void );

	bool IsValidDeathInfo( void ) const;

private:
	IViewPort *m_pViewport;

	Label *m_pDetails;

	Button *m_pClose;
};

//=========================================================
//=========================================================
CREATE_INSVIEWPORT_PANEL( CDeathInfo );

CDeathInfo::CDeathInfo( IViewPort *pViewPort )
	: Frame( NULL, PANEL_DEATHINFO, false )
{
	m_pViewport = pViewPort;

	// setup panel
	SetScheme( "ClientScheme" );

	SetMoveable( false );
	SetSizeable( false );
	SetTitleBarVisible( false );

	// setup settings
	m_pDetails = new Label( this, "Details", "" );

	m_pClose = new Button( this, "OKButton", "#PropertyDialog_OK" );
	m_pClose->SetCommand( "okay" );

	// load settings
	LoadControlSettings( "resource/ui/basic/deathinfo.res" );
}

//=========================================================
//=========================================================
void CDeathInfo::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pDetails->SetFgColor( COLOR_WHITE );
}

//=========================================================
//=========================================================
void CDeathInfo::OnCommand( const char *pszCommand )
{
	if( FStrEq( pszCommand, "okay" ) )
	{
		Close( );

		GetINSVGUIHelper( )->FinishDeathInfo( );

		return;
	}

	BaseClass::OnCommand( pszCommand );
}

//=========================================================
//=========================================================
void CDeathInfo::ShowDeathInfo( void )
{
	if( IsValidDeathInfo( ) )
		m_pViewport->ShowPanel( this, true );
}

//=========================================================
//=========================================================
void CDeathInfo::UpdateDeathInfo( void )
{
	UpdateDetails( );
}

//=========================================================
//=========================================================
void CDeathInfo::UpdateDetails( void )
{
	const DeathInfoData_t &DeathInfoData = GetINSVGUIHelper( )->GetDeathData( );

	const char *pszDeathInfo;
	char szDeathInfo[ 128 ];

	pszDeathInfo = NULL;
	szDeathInfo[ 0 ] = '\0';

	// generate why
	if( DeathInfoData.m_iType != PDEATHTYPE_SELF )
	{
		Q_strncpy( szDeathInfo, "you were killed", sizeof( szDeathInfo ) );
		
		switch( DeathInfoData.m_iType )
		{
			case PDEATHTYPE_KIA:
			case PDEATHTYPE_FF:
			{
				char szBuffer[ 128 ];
				szBuffer[ 0 ] = '\0';

				pszDeathInfo = szDeathInfo;

				if( DeathInfoData.m_bDeathInfoFull )
				{
					char szBuffer[ 128 ];
					szBuffer[ 0 ] = '\0';

					Q_snprintf( szBuffer, sizeof( szBuffer ), "by %s with a %s", PlayerResource( )->GetPlayerName( DeathInfoData.m_iAttackerID ), "unknown" );
					Q_strncat( szDeathInfo, szBuffer, sizeof( szDeathInfo ), COPY_ALL_CHARACTERS );

					if( DeathInfoData.m_iDistance != 0 )
					{
						Q_snprintf( szBuffer, sizeof( szBuffer ), " at %im", DeathInfoData.m_iDistance );
						Q_strncat( szDeathInfo, szBuffer, sizeof( szDeathInfo ), COPY_ALL_CHARACTERS );
					}
				}
				else
				{
					Q_snprintf( szDeathInfo, sizeof( szDeathInfo ), "by %s", "unknown" );
				}

				break;
			}

			case PDEATHTYPE_TELEFRAG:
			{
				pszDeathInfo = "by telefragging";
				break;
			}

			case PDEATHTYPE_SOULSTOLEN:
			{
				pszDeathInfo = "because a developer stole your soul";
				break;
			}
		}
	}
	else
	{
		pszDeathInfo = "you killed yourself";
	}

	// set the text
	m_pDetails->SetText( pszDeathInfo );
}

//=========================================================
//=========================================================
void CDeathInfo::ShowPanel( bool bShow )
{
	if( IsVisible( ) == bShow )
		return;

	if( bShow )
	{
		if( IsValidDeathInfo( ) )
		{
			Activate( );
			SetMouseInputEnabled( true );
		}
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}
}

//=========================================================
//=========================================================
bool CDeathInfo::IsValidDeathInfo( void ) const
{
	return GetINSVGUIHelper( )->IsValidDeathData( );
}

#endif // INSVGUI_FINAL_SQUADSELECTION