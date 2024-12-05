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
CHUDIconText::CHUDIconText( const char *pszElementName, const char *pszHUDName ) :
	CHudElement( pszElementName ), BaseClass( NULL, pszHUDName )
{
	Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	SetPaintBackgroundEnabled( false );

	gameeventmanager->AddListener( this, "round_unfrozen", false );
}

//=========================================================
//=========================================================
void CHUDIconText::Init( void )
{
	Assert( IconPath( ) );

	SetHiddenBits( HIDEHUD_DEAD | HIDEHUD_ROUNDCOLD | ExtraHiddenBits( ) );
}

//=========================================================
//=========================================================
void CHUDIconText::Reset( void )
{
	m_iTextLength = 0;
	m_flHighlightThreshold = 0.0f;
}

//=========================================================
//=========================================================
void CHUDIconText::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_iIconSize = scheme( )->GetProportionalScaledValue( ICONTEXT_ICON_SIZE );

	m_iTextXGap = scheme( )->GetProportionalScaledValue( TextXGap( ) );
	m_iTextXPos = scheme( )->GetProportionalScaledValue( ICONTEXT_TEXT_XPOS );
	m_iTextYPos = scheme( )->GetProportionalScaledValue( ICONTEXT_TEXT_YPOS );

	m_hTextFont = pScheme->GetFont( ICONTEXT_TEXT_FONT, true );
}

//=========================================================
//=========================================================
void CHUDIconText::FireGameEvent( IGameEvent *pEvent )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( pPlayer && pPlayer->IsRunningAround( ) )
	{
		Update( );

		HighlightText( );
	}
}

//=========================================================
//=========================================================
int CHUDIconText::LoadIcon( const char *pszFile )
{
	char szPath[ 128 ];
	Q_snprintf( szPath, sizeof( szPath ), "HUD/%s/%s", IconPath( ), pszFile );

	int iIconID = surface( )->CreateNewTextureID( );
	surface( )->DrawSetTextureFile( iIconID, szPath, false, false );
	
	return iIconID;
}

//=========================================================
//=========================================================
void CHUDIconText::SetText( const char *pszText, bool bHighlight )
{
	localize( )->ConvertANSIToUnicode( pszText, m_wszText, sizeof( m_wszText ) );
	m_iTextLength = wcslen( m_wszText );

	if( bHighlight )
		HighlightText( );
}

//=========================================================
//=========================================================
void CHUDIconText::HighlightText( void )
{
	m_flHighlightThreshold = gpGlobals->curtime + ICONTEXT_FLASH_TIME_HIGHLIGHT;
}

//=========================================================
//=========================================================
void CHUDIconText::SetColor( const Color &theColor )
{
	m_Color = theColor;
}

//=========================================================
//=========================================================
void CHUDIconText::OnThink( void )
{
	if( ShouldDraw( ) && OnThinkUpdate( ) )
		Update( );
}

//=========================================================
//=========================================================
void CHUDIconText::Paint( void )
{
	if( m_iTextLength == 0 )
		return;

	static Color DrawColor = COLOR_WHITE;

	// set alpha
	if( m_flHighlightThreshold > gpGlobals->curtime )
	{
		DrawColor[ 3 ] = 255;
	}
	else
	{
		float flFadeThreshold = m_flHighlightThreshold + ICONTEXT_FLASH_TIME_FADE;

		if( flFadeThreshold > gpGlobals->curtime )
		{
			static int iAlpha = 255 - ICONTEXT_FLASH_FADED;
			DrawColor[ 3 ] = ICONTEXT_FLASH_FADED + ( ( ( flFadeThreshold - gpGlobals->curtime )  / ICONTEXT_FLASH_TIME_FADE ) * iAlpha );
		}
		else
		{
			DrawColor[ 3 ] = ICONTEXT_FLASH_FADED;
		}
	}

	// draw background
	surface( )->DrawSetTexture( IconID( ) );
	surface( )->DrawSetColor( DrawColor );
	surface( )->DrawTexturedRect( 0, 0, m_iIconSize, m_iIconSize );

	// draw the text
	surface( )->DrawSetTextFont( m_hTextFont );
	surface( )->DrawSetTextColor( DrawColor );
	surface( )->DrawSetTextPos( m_iTextXGap + m_iTextXPos, m_iTextYPos );
	surface( )->DrawPrintText( m_wszText, m_iTextLength );
}