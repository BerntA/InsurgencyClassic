//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "hud_macros.h"
#include "hudelement.h"
#include "action_helper.h"
#include "hint_helper.h"
#include "basic_colors.h"
#include "iclientmode.h"

#include <vgui/vgui.h>
#include <vgui/isurface.h>
#include <vgui/ilocalize.h>
#include <vgui_controls/panel.h>

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
class CHudHint;

//=========================================================
//=========================================================
#define HINT_TIME 3.0f
#define HINT_FADETIME 1.5f

#define HINT_YPOS_GAP 2

//=========================================================
//=========================================================
ConVar hideactionindicators( "cl_hideactionindicators", "0", FCVAR_ARCHIVE, "Hide the Action Indicators" );

//=========================================================
//=========================================================
class CHintLine
{
public:
	CHintLine( );

	void Init( CHudHint *pHint );

	void Reset( void );

	void SetFont( HFont hFont );
	void SetYPos( int iYPos );
	void Set( const char *pszText );

	void Draw( int iAlpha );

private:
	CHudHint *m_pHudHint;

	HFont m_hFont;
	int m_iYPos;

	wchar_t m_wszText[ HINT_MAXLENGTH ];
	int m_iLength;

	int m_iXPos;
};

//=========================================================
//=========================================================
class CHudHint : public CHudElement, public Panel, public IHintListener, public IActionListener
{
	DECLARE_CLASS_SIMPLE( CHudHint, Panel );

public:
	CHudHint( const char *pszElementName );

private:
	void Reset( void );
	void LevelInit( void );

	void ApplySchemeSettings( IScheme *pScheme );

    bool ShouldDraw( void );
	void Paint( void );

	void OnAction( int iActionID );
	void OnHint( int iHintID );

	void ResetHint( void );
	void ShowHint( const char *pszText, const char *pszTextAdditional );

private:
	int m_iActionID, m_iHintID;
	float m_flDrawFinish;

	int m_iTextMainYGap;

	CHintLine m_TextMain, m_TextAdditional;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHudHint );

CHudHint::CHudHint( const char *pszElementName )
	: CHudElement( pszElementName ), Panel( NULL, "HudHint" )
{
	Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	SetPaintBackgroundEnabled( false );
	SetHiddenBits( HIDEHUD_DEAD );

	m_TextMain.Init( this );
	m_TextAdditional.Init( this );
}

//=========================================================
//=========================================================
void CHudHint::Reset( void )
{
	ResetHint( );

	m_TextMain.Reset( );
	m_TextAdditional.Reset( );
}

//=========================================================
//=========================================================
void CHudHint::LevelInit( void )
{
	Reset( );
}

//=========================================================
//=========================================================
void CHudHint::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	HFont hMainFont, hAdditionalFont;
	hMainFont = pScheme->GetFont( "DefaultBig" );
	hAdditionalFont = pScheme->GetFont( "Default" );

	m_TextMain.SetFont( hMainFont );
	m_TextAdditional.SetFont( hAdditionalFont );

	m_iTextMainYGap = scheme( )->GetProportionalScaledValue( HINT_YPOS_GAP );

	m_TextAdditional.SetYPos( m_iTextMainYGap + surface( )->GetFontTall( hMainFont ) );
}

//=========================================================
//=========================================================
bool CHudHint::ShouldDraw( void )
{
	if( !CHudElement::ShouldDraw( ) )
		return false;

	// never draw if past draw time
	if( gpGlobals->curtime > m_flDrawFinish )
		return false;

	// don't draw actions when hiding hints
	if( hideactionindicators.GetBool( ) && m_iActionID != ACTION_INVALID )
		return false;

	// don't draw hints when hiding hints
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return false;

	if( pPlayer->GetCmdValue( CMDREGISTER_HIDEHINTS ) && m_iHintID != HINT_INVALID )
		return false;

	return true;
}

//=========================================================
//=========================================================
void CHudHint::Paint( void )
{
	int iAlpha = 255;

	if( ( m_flDrawFinish - gpGlobals->curtime ) <= HINT_FADETIME )
	{
		float flFadeFrac = ( m_flDrawFinish - gpGlobals->curtime ) / HINT_FADETIME;
		iAlpha = clamp( RoundFloatToInt( 255 * flFadeFrac ), 0, 255 );
	}

	m_TextMain.Draw( iAlpha );
	m_TextAdditional.Draw( iAlpha );
}

//=========================================================
//=========================================================
void CHudHint::OnAction( int iActionID )
{
	if( m_iActionID == iActionID )
		return;

	// reset the current hint
	ResetHint( );

	// set new action
	m_iActionID = iActionID;

	// when valid action - show hint
	if( m_iActionID != ACTION_INVALID )
		ShowHint( g_ActionHelper.Action( m_iActionID ), NULL );
}

//=========================================================
//=========================================================
void CHudHint::OnHint( int iHintID )
{
	if( m_iActionID != ACTION_INVALID )
		return;

	if( m_iHintID == iHintID )
		return;

	IHint *pHint = g_HintHelper.GetHint( iHintID );

	if( !pHint )
		return;

	m_iHintID = iHintID;

	// get text
	const char *pszText, *pszTextAdditional;
	pszText = pHint->Text( );
	pszTextAdditional = pHint->TextAdditional( );
	
	// apply binding if not NULL
	const char *pszBinding = pHint->Binding( );

	if( pszBinding )
	{
		char szNewText[ HINT_MAXLENGTH ];
		int iNewTextPoint = 0;

		for( const char *pszTextPoint = pszText; *pszTextPoint != '\0'; pszTextPoint++ )
		{
			char TextElement = *pszTextPoint;

			if( TextElement == '£' )
			{
				const char *pszButton = engine->Key_LookupBinding( pszBinding );

				if( !pszButton )
					return;

				for( const char *pszButtonPoint = pszButton; *pszButtonPoint != '\0'; pszButtonPoint++ )
				{
					szNewText[ iNewTextPoint ] = *pszButtonPoint;
					iNewTextPoint++;
				}

				continue;
			}

			szNewText[ iNewTextPoint ] = TextElement;
			iNewTextPoint++;
		}

		szNewText[ iNewTextPoint ] = '\0';

		ShowHint( szNewText, pszTextAdditional );
	}
	else
	{
		ShowHint( pszText, pszTextAdditional );
	}
}

//=========================================================
//=========================================================
void CHudHint::ResetHint( void )
{
	m_iActionID = ACTION_INVALID;
	m_iHintID = HINT_INVALID;

	m_flDrawFinish = 0.0f;
}

//=========================================================
//=========================================================
void CHudHint::ShowHint( const char *pszText, const char *pszTextAdditional )
{
	m_TextMain.Set( pszText );
	m_TextAdditional.Set( pszTextAdditional );

	m_flDrawFinish = gpGlobals->curtime + HINT_TIME + HINT_FADETIME;
}

//=========================================================
//=========================================================
CHintLine::CHintLine( )
{
	m_pHudHint = NULL;

	m_hFont = INVALID_FONT;
	m_iYPos = 0;
}

//=========================================================
//=========================================================
void CHintLine::Init( CHudHint *pHint )
{
	m_pHudHint = pHint;
}

//=========================================================
//=========================================================
void CHintLine::Reset( void )
{
	m_wszText[ 0 ] = '\0';
	m_iLength = 0;

	m_iXPos = 0;
}

//=========================================================
//=========================================================
void CHintLine::SetFont( HFont hFont )
{
	m_hFont = hFont;
}

//=========================================================
//=========================================================
void CHintLine::SetYPos( int iYPos )
{
	m_iYPos = iYPos;
}

//=========================================================
//=========================================================
void CHintLine::Set( const char *pszText )
{
	static const char *pszEmptyText = "";

	if( !pszText )
		pszText = pszEmptyText;

	localize( )->ConvertANSIToUnicode( pszText, m_wszText, sizeof( m_wszText ) );
	m_iLength = Q_strlen( pszText );

	m_iXPos = ( m_pHudHint->GetWide( ) * 0.5f ) - ( UTIL_ComputeStringWidth( m_hFont, pszText ) * 0.5f );
}

//=========================================================
//=========================================================
void CHintLine::Draw( int iAlpha )
{
	if( m_iLength == 0 )
		return;

	Color TextColor = COLOR_WHITE;
	TextColor[ 3 ] = iAlpha;

	surface( )->DrawSetTextFont( m_hFont );
	surface( )->DrawSetTextColor( TextColor );
	surface( )->DrawSetTextPos( m_iXPos, m_iYPos );
	surface( )->DrawPrintText( m_wszText, m_iLength );
}