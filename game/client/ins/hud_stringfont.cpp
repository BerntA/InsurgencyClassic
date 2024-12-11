//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud_stringfont.h"
#include "iclientmode.h"

#include <color.h>
#include <keyvalues.h>
#include <vgui/isurface.h>
#include <vgui/isystem.h>
#include <vgui/ivgui.h>

#include "vgui_controls/vgui_helper.h"

#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define STRINGFONTS_START() \
	struct { \
		const char *m_pszName; \
		const char *m_pszPath; \
	} g_StringData[STRINGFONT_COUNT] = {

#define DEFINE_STRINGFONT(name, path) \
	{ name, path }

#define STRINGFONTS_END() \
	};

#define OBJECTINDI_PATH "sprites/HUD/objectindi"
#define GAMEINFO_PATH "VGUI/resources/gameinfo"
#define INSHEADER_PATH "VGUI/resources/insheader"

STRINGFONTS_START()
	DEFINE_STRINGFONT("distance", OBJECTINDI_PATH),	// STRINGFONT_DISTANCE
	DEFINE_STRINGFONT("phonetic", OBJECTINDI_PATH),	// STRINGFONT_PHONETIC
	DEFINE_STRINGFONT("mapname", GAMEINFO_PATH),	// STRINGFONT_MAPNAME
	DEFINE_STRINGFONT("version", INSHEADER_PATH)	// STRINGFONT_VERSION
STRINGFONTS_END()

//=========================================================
//=========================================================
DECLARE_HUDELEMENT(CHudStringDisplay);

CHudStringDisplay *g_pHudStringDisplay = NULL;

CHudStringFont::CHudStringFont()
{
	m_iTexID = INVALID_ALPHANUMCHAR;
	m_iOverlayID = INVALID_ALPHANUMCHAR;
	m_i640Scale = 0;
	m_iSpaceSize = 0;
	m_iScaledSpaceSize = 0;
}

//=========================================================
//=========================================================
void CHudStringFont::LoadFont(const char *pszPath, const char *pszFontName)
{
	char szBuffer[256];

	// Load the Font
	Q_snprintf(szBuffer, sizeof(szBuffer), "%s/%s", pszPath, pszFontName);

	if(!CheckVGUIMaterialExists(szBuffer, false))
		return;

	m_iTexID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_iTexID, szBuffer, false, false);

	int iTexWide, iTexTall;
	vgui::surface()->DrawGetTextureSize(m_iTexID, iTexWide, iTexTall);

	// Load the Char Data
	KeyValues *pFontData, *pCharData, *pChar;
	pFontData = new KeyValues("FontData");

	// load the file
	Q_snprintf(szBuffer, sizeof(szBuffer), "materials/%s/%s.txt", pszPath, pszFontName);
	pFontData->LoadFromFile(filesystem, szBuffer, "MOD");

	// take font data
	int iTall = pFontData->GetInt("height", 1);
	m_i640Scale = pFontData->GetInt("640scale", 1);
	m_iSpaceSize = pFontData->GetInt("space", 0);
	m_iWhiteSpaceSize = pFontData->GetInt("whitespace", 0);

	const char *pszOverlayPath = NULL;
	pszOverlayPath = pFontData->GetString("overlay", NULL);

	if(pszOverlayPath && *pszOverlayPath)
	{
		char szOverlayPath[256];

		Q_strncpy(szOverlayPath, pszPath, sizeof(szOverlayPath));
		Q_strncat(szOverlayPath, pszOverlayPath, sizeof(szOverlayPath), COPY_ALL_CHARACTERS);

		if(CheckVGUIMaterialExists(szOverlayPath, false))
		{
			m_iOverlayID = vgui::surface()->CreateNewTextureID();
			vgui::surface()->DrawSetTextureFile(m_iOverlayID, szOverlayPath, false, false);
		}
	}

	pCharData = pFontData->FindKey("CharData");
	if (!pCharData)
	{
		pFontData->deleteThis();
		return;
	}

	// take char data
	pChar = pCharData->GetFirstSubKey();

	while(pChar)
	{
		const char *pszChar = pChar->GetName();
		int iCharNum = LookupChar(pszChar[0]);

		if(iCharNum == INVALID_ALPHANUMCHAR)
			continue;

		HudStringChar_t &Char = m_AlphaNumericChar[iCharNum];
		Char.m_bValid = true;

		Char.m_iXPos = pChar->GetInt("x");
		Char.m_iYPos = pChar->GetInt("y");
		Char.m_iWide = pChar->GetInt("width");
		int iCharTall = pFontData->GetInt("height", 0);

		if(iCharTall != 0)
			Char.m_iTall = iCharTall;
		else
			Char.m_iTall = iTall;

		CreateTexCoord(Char.m_TexCoords,
			Char.m_iXPos, Char.m_iYPos,
			Char.m_iWide, Char.m_iTall,
			iTexWide, iTexTall);

		pChar = pChar->GetNextKey();
	}

	pFontData->deleteThis();
}

//=========================================================
//=========================================================
void CHudStringFont::Update(void)
{
	m_iScaledSpaceSize = scheme()->GetProportionalScaledValue(m_iSpaceSize);
	m_iScaledWhiteSpaceSize = scheme()->GetProportionalScaledValue(m_iWhiteSpaceSize);
}

//=========================================================
//=========================================================
int CHudStringFont::LookupChar(const char cChar)
{
	// check for a number first
	if(cChar >= 48 && cChar <= 57)
		return (cChar - 48);

	if(cChar >= 65 && cChar <= 90)
		return (cChar - 65 + ALPHANUMCHAR_A);

	if(cChar >= 97 && cChar <= 122)
		return (cChar - 97 + ALPHANUMCHAR_A);

	return INVALID_ALPHANUMCHAR;
}

//=========================================================
//=========================================================
CHudStringDisplay::CHudStringDisplay(const char *pszName)
	: CHudElement(pszName), Panel(NULL, "HudStringFont")
{
	// setup global pointer
	g_pHudStringDisplay = this;

	// set the screen as the parent
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	// Set Properties
	SetPaintBackgroundEnabled(false);

	// Set Scheme
	HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	SetScheme(scheme);

	// Init Fonts
	for(int i = 0; i < STRINGFONT_COUNT; i++)
		LoadFont(i);
}

//=========================================================
//=========================================================
CHudStringDisplay::~CHudStringDisplay()
{
	g_pHudStringDisplay = NULL;
}

//=========================================================
//=========================================================
void CHudStringDisplay::DrawStringWithOverlay(int iXPos, int iYPos,
											  const char *pszString,
											  const Color &StringColor, const Color *pOverlayColor,
											  float flScale, int iFont)
{
	int iStartXPos = 0;

	for(const char *i = pszString; *i != '\0'; i++)
		iStartXPos += DrawChar(iStartXPos + iXPos, iYPos, *i, StringColor, pOverlayColor, flScale, iFont);
}

//=========================================================
//=========================================================
void CHudStringDisplay::DrawString(int iXPos, int iYPos,
											  const char *pszString,
											  const Color &StringColor,
											  float flScale, int iFont)
{
	DrawStringWithOverlay(iXPos, iYPos, pszString, StringColor, NULL, flScale, iFont);
}

//=========================================================
//=========================================================
void CHudStringDisplay::DrawStringWithOverlay(int iXPos, int iYPos,
											  const char *pszString, const Color &StringColor,
											  const Color &OverlayColor,
											  float flScale, int iFont)
{
	DrawStringWithOverlay(iXPos, iYPos, pszString, StringColor, &OverlayColor, flScale, iFont);
}

//=========================================================
//=========================================================
int CHudStringDisplay::DrawChar(int iXPos, int iYPos,
											char cChar,
											const Color &StringColor, const Color *pOverlayColor,
											float flScale, int iFont)
{
	CHudStringFont &Font = m_Fonts[iFont];

	if(cChar == ' ')
		return Font.m_iScaledWhiteSpaceSize;

	int i640Scale = Font.m_i640Scale;

	if(i640Scale == 0)
		i640Scale = 1;

	int iCharID = CHudStringFont::LookupChar(cChar);

	if(iCharID == INVALID_ALPHANUMCHAR)
		return 0;

	HudStringChar_t &StringChar = Font.m_AlphaNumericChar[iCharID];
	//Assert(StringChar.m_bValid);

	int iScaledWidth = scheme()->GetProportionalScaledValue(RoundFloatToInt((float)(StringChar.m_iWide / i640Scale) * flScale));
	int iScaledTall = scheme()->GetProportionalScaledValue(RoundFloatToInt((float)(StringChar.m_iTall / i640Scale) * flScale));

	vgui::surface()->DrawSetTexture(Font.m_iTexID);
	vgui::surface()->DrawSetColor(StringColor);
	vgui::surface()->DrawTexturedSubRect(iXPos, iYPos, iXPos + iScaledWidth, iYPos + iScaledTall,
		StringChar.m_TexCoords[0], StringChar.m_TexCoords[1], StringChar.m_TexCoords[2], StringChar.m_TexCoords[3]);

	if(pOverlayColor && Font.m_iOverlayID != INVALID_ALPHANUMCHAR)
	{
		vgui::surface()->DrawSetTexture(Font.m_iOverlayID);
		vgui::surface()->DrawSetColor(*pOverlayColor);
		vgui::surface()->DrawTexturedSubRect(iXPos, iYPos, iXPos + iScaledWidth, iYPos + iScaledTall,
			StringChar.m_TexCoords[0], StringChar.m_TexCoords[1], StringChar.m_TexCoords[2], StringChar.m_TexCoords[3]);
	}

	return iScaledWidth + Font.m_iScaledSpaceSize;
}

//=========================================================
//=========================================================
int CHudStringDisplay::GetStringLength(const char *pszString, float flScale, int iFont)
{
	CHudStringFont &Font = m_Fonts[iFont];
	int iStringWidth = 0;

	for(const char *i = pszString; *i != '\0'; i++)
	{
		char cCurrentChar = *i;
		int iCharID = CHudStringFont::LookupChar(cCurrentChar);
		Assert(iCharID != INVALID_ALPHANUMCHAR);

		HudStringChar_t &StringChar = Font.m_AlphaNumericChar[iCharID];

		Assert(StringChar.m_bValid);

		int i640Scale = Font.m_i640Scale;

		if(i640Scale == 0)
			i640Scale = 1;

		int iScaledWidth = scheme()->GetProportionalScaledValue(RoundFloatToInt((float)(StringChar.m_iWide / i640Scale) * flScale));
		iStringWidth += iScaledWidth + Font.m_iScaledSpaceSize;
	}

	return iStringWidth;
}

//=========================================================
//=========================================================
int CHudStringDisplay::GetStringHeight(const char *pszString, float flScale, int iFont)
{
	CHudStringFont &Font = m_Fonts[iFont];
	int iStringHeight = 0;

	for(const char *i = pszString; *i != '\0'; i++)
	{
		char cCurrentChar = *i;
		int iCharID = CHudStringFont::LookupChar(cCurrentChar);
		Assert(iCharID != INVALID_ALPHANUMCHAR);

		HudStringChar_t &StringChar = Font.m_AlphaNumericChar[iCharID];

		Assert(StringChar.m_bValid);

		int i640Scale = Font.m_i640Scale;

		if(i640Scale == 0)
			i640Scale = 1;

		int iScaledHeight = scheme()->GetProportionalScaledValue(RoundFloatToInt((float)(StringChar.m_iTall / i640Scale) * flScale));

		if(iScaledHeight > iStringHeight)
			iStringHeight = iScaledHeight;
	}

	return iStringHeight;
}

//=========================================================
//=========================================================
void CHudStringDisplay::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	for(int i = 0; i < STRINGFONT_COUNT; i++)
		m_Fonts[i].Update();
}

//=========================================================
//=========================================================
void CHudStringDisplay::LoadFont(int iFont)
{
	m_Fonts[iFont].LoadFont(g_StringData[iFont].m_pszPath, g_StringData[iFont].m_pszName);
}