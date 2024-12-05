//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud_macros.h"
#include "iclientmode.h"

#include <vgui/isurface.h>
#include <vgui/ilocalize.h>
#include <vgui_controls/panel.h>

#include "hud_icontext.h"

#include "inshud.h"
#include "basic_colors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
class CHUDEmergencyDeployments : public CHUDIconText, public IINSReinforcement
{
	DECLARE_CLASS_SIMPLE( CHUDEmergencyDeployments, CHUDIconText );

public:
	CHUDEmergencyDeployments( const char *pszElementName );

private:
	void Init( void );

	int HiddenBits( void ) const { return HIDEHUD_NOTCOMMANDER; }
	const char *IconPath( void ) const { return "edeployments"; }
	int IconID( void ) const { return m_iIconID; }
	void Update( void );

	void EmergencyDeployment( void );

private:
	int m_iIconID;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHUDEmergencyDeployments );

//=========================================================
//=========================================================
void CHUDEmergencyDeployments::Init( void )
{
	BaseClass::Init( );

	m_iIconID = LoadIcon( "icon" );
}

//=========================================================
//=========================================================
CHUDEmergencyDeployments::CHUDEmergencyDeployments( const char *pszElementName )
	: CHUDIconText( pszElementName, "HudEmergencyDeployments" )
{
}

//=========================================================
//=========================================================
void CHUDEmergencyDeployments::EmergencyDeployment( void )
{
	Update( );
}

//=========================================================
//=========================================================
void CHUDEmergencyDeployments::Update( void )
{
	char szBuffer[ ICONTEXT_TEXT_LENGTH ];
	int iLeft;

	// set the text
	if( !GetINSHUDHelper( )->EmergencyDeployments( iLeft ) )
		return;

	Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", iLeft );

	SetText( szBuffer, true );

	// set the text color
	SetColor( ( iLeft <= 1 ) ? COLOR_RED : COLOR_WHITE );
}