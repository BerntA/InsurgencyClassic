//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HUD_STRINGFONT_H
#define HUD_STRINGFONT_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/panel.h>
#include "hudelement.h"

//=========================================================
//=========================================================
#define INVALID_ALPHANUMCHAR -1 // can't find the -1 in vgui_surface

enum AlphaNumericChar_t
{
	ALPHANUMCHAR_0 = 0,
	ALPHANUMCHAR_1,	ALPHANUMCHAR_2,
	ALPHANUMCHAR_3,	ALPHANUMCHAR_4,
	ALPHANUMCHAR_5,	ALPHANUMCHAR_6,
	ALPHANUMCHAR_7,	ALPHANUMCHAR_8,
	ALPHANUMCHAR_9,	ALPHANUMCHAR_A,
	ALPHANUMCHAR_B,	ALPHANUMCHAR_C,
	ALPHANUMCHAR_D,	ALPHANUMCHAR_E,
	ALPHANUMCHAR_F,	ALPHANUMCHAR_G,
	ALPHANUMCHAR_H,	ALPHANUMCHAR_I,
	ALPHANUMCHAR_J,	ALPHANUMCHAR_K,
	ALPHANUMCHAR_L,	ALPHANUMCHAR_M,
	ALPHANUMCHAR_N,	ALPHANUMCHAR_O,
	ALPHANUMCHAR_P,	ALPHANUMCHAR_Q,
	ALPHANUMCHAR_R,	ALPHANUMCHAR_S,
	ALPHANUMCHAR_T,	ALPHANUMCHAR_U,
	ALPHANUMCHAR_V,	ALPHANUMCHAR_W,
	ALPHANUMCHAR_X,	ALPHANUMCHAR_Y,
	ALPHANUMCHAR_Z,	ALPHANUMCHAR_COUNT
};

struct HudStringChar_t
{
	HudStringChar_t()
	{
		m_bValid = false;
		m_iXPos = m_iYPos = 0;
		m_iWide = 0;
		memset(m_TexCoords, 0.0f, sizeof(m_TexCoords));
	}

	bool m_bValid;

	int m_iXPos, m_iYPos;
	int m_iWide, m_iTall;

	TexCoord_t m_TexCoords;
};

enum HudStringFonts_t
{
	STRINGFONT_DISTANCE = 0,
	STRINGFONT_PHONETIC,
	STRINGFONT_MAPNAME,
	STRINGFONT_VERSION,
	STRINGFONT_COUNT
};

class CHudStringFont
{
public:
	CHudStringFont();

	void LoadFont(const char *pszPath, const char *pszFontName);

	void Update(void);

	static int LookupChar(const char cChar);

public:
	int m_iTexID, m_iOverlayID;
	int m_i640Scale;
	int m_iSpaceSize, m_iScaledSpaceSize;
	int m_iWhiteSpaceSize, m_iScaledWhiteSpaceSize;

	HudStringChar_t m_AlphaNumericChar[ALPHANUMCHAR_COUNT];
};

//=========================================================
//=========================================================
class CHudStringDisplay : public CHudElement, public vgui::Panel
{	
	DECLARE_CLASS_SIMPLE(CHudStringDisplay, vgui::Panel);

public:
	CHudStringDisplay(const char *pszName);
	virtual ~CHudStringDisplay();

	void DrawString(int iXPos, int iYPos, const char *pszString, const Color &StringColor, float flScale, int iFont);
	void DrawStringWithOverlay(int iXPos, int iYPos, const char *pszString, const Color &StringColor, const Color *pOverlayColor, float flScale, int iFont);
	void DrawStringWithOverlay(int iXPos, int iYPos, const char *pszString, const Color &StringColor, const Color &OverlayColor, float flScale, int iFont);
	int DrawChar(int iXPos, int iYPos, char cString, const Color &StringColor, const Color *pOverlayColor, float flScale, int iFont);
	int GetStringLength(const char *pszString, float flScale, int iFont);
	int GetStringHeight(const char *pszString, float flScale, int iFont);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	void LoadFont(int iFont);

private:
	CHudStringFont m_Fonts[STRINGFONT_COUNT];
};

extern CHudStringDisplay *g_pHudStringDisplay;

#endif //HUD_STRINGFONT_H