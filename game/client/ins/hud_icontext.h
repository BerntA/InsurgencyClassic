//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_ICONTEXT_H
#define HUD_ICONTEXT_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/panel.h>

//=========================================================
//=========================================================
#define ICONTEXT_ICON_SIZE 32

#define ICONTEXT_TEXT_XPOS 26
#define ICONTEXT_TEXT_YPOS 8
#define ICONTEXT_TEXT_FONT "PlayerStatus"
#define ICONTEXT_TEXT_LENGTH 16

#define ICONTEXT_FLASH_TIME_HIGHLIGHT 4.0f
#define ICONTEXT_FLASH_TIME_FADE 0.75f
#define ICONTEXT_FLASH_FADED 128

//=========================================================
//=========================================================
class CHUDIconText : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHUDIconText, vgui::Panel );

public:
	CHUDIconText( const char *pszElementName, const char *pszHUDName );

protected:
	virtual void Init( void );

	virtual void FireGameEvent( IGameEvent *pEvent );

	int LoadIcon( const char *pszFile );
	void SetText( const char *pszText, bool bHighlight );
	void HighlightText( void );
	void SetColor( const Color &theColor );

	virtual int ExtraHiddenBits( void ) const { return 0; }
	virtual const char *IconPath( void ) const = 0;
	virtual int IconID( void ) const = 0;
	virtual int TextXGap( void ) const { return 0; }
	virtual void Update( void ) = 0;
	virtual bool OnThinkUpdate( void ) const { return false; }

private:
	void Reset( void );

	void ApplySchemeSettings( vgui::IScheme *pScheme );

	void OnThink( void );
	void Paint( void );

private:
	int m_iIconSize;

	Color m_Color;

	int m_iTextXGap, m_iTextXPos, m_iTextYPos;
	vgui::HFont m_hTextFont;
	wchar_t m_wszText[ ICONTEXT_TEXT_LENGTH ];
	int m_iTextLength;

	float m_flHighlightThreshold;
};

#endif // HUD_ICONTEXT_H