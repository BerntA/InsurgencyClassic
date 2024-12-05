//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hudelement.h"
#include <vgui_controls/panel.h>
#include "hud_macros.h"
#include "c_playerresource.h"
#include "vgui_entitypanel.h"
#include "iclientmode.h"
#include "vgui/ilocalize.h"
#include "ins_gamerules.h"
#include "c_ins_obj.h"
#include "basic_colors.h"
#include "ins_player_shared.h"
#include "ins_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
enum CaptureBarParts_t
{
	CAPTUREBAR_BG,
	CAPTUREBAR_FG,
	CAPTUREBAR_COUNT
};

const char *g_pszCaptureBarTexNames[CAPTUREBAR_COUNT] = {
	"capturebar_bg",	// CAPTUREBAR_BG
	"capturebar_fg",	// CAPTUREBAR_FG
};

#define CAPTUREBAR_WIDE 256 // 243
#define CAPTUREBAR_TALL 16 // 11
#define CAPTUREBAR_SCALE 2

#define CAPTUREBAR_ALPHA_MAXLIMIT 0.2f
#define CAPTUREBAR_ALPHA_MINLIMIT 0.98f
#define CAPTUREBAR_ALPHA_MIN 60

#define CAPTUREBAR_TEXT_YPOS 3

struct CaptureBarTex_t
{
	int m_iTexID;
	TexCoord_t m_TexCoords;
};

enum CaptureTextStates_t
{
	CAPTURETEXT_NONE = 0,
	CAPTURETEXT_CAPTURE,
	CAPTURETEXT_CAPTURED,
	CAPTURETEXT_ENEMYBLOCK
};

//=========================================================
//=========================================================
class CCaptureBar : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CCaptureBar, Panel);

public:
	CCaptureBar(const char *pElementName);

	void Init(void);
	void VidInit(void);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Paint(void);

private:
	void DrawCaptureBar(int iCaptureBarID, float flFraction, int iAlpha);

	void UpdateCaptureText(int iNewState);
	void UpdateCaptureText(const char *pszNewText);

private:
	CaptureBarTex_t m_CaptureBar[CAPTUREBAR_COUNT];

	int m_iCaptureBarWide, m_iCaptureBarTall;
	int m_iCaptureBarTexWide, m_iCaptureBarTexTall;

	vgui::HFont	m_hCaptureFont;
	int m_iCaptureTextState, m_iCaptureTextLastObj;
	wchar_t m_wszCaptureText[64];
	int m_iCaptureTextLength;
	int m_iCaptureTextXPos, m_iCaptureTextYPos;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT(CCaptureBar);

CCaptureBar::CCaptureBar(const char *pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "CaptureBar")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetPaintBackgroundEnabled(false);

	m_hCaptureFont = 0;
	m_iCaptureTextState = CAPTURETEXT_NONE;
	m_iCaptureTextLastObj = -1;

	SetHiddenBits(HIDEHUD_SPECTATOR);
}

//=========================================================
//=========================================================
void CCaptureBar::Init(void)
{
	char szTexPath[256];

	for(int i = 0; i < CAPTUREBAR_COUNT; i++)
	{
		CaptureBarTex_t &CaptureBarTex = m_CaptureBar[i];

		Q_snprintf(szTexPath, sizeof(szTexPath), "sprites/hud/capturebar/%s", g_pszCaptureBarTexNames[i]);

		CaptureBarTex.m_iTexID = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(CaptureBarTex.m_iTexID, szTexPath, false, false);
	}

	vgui::surface()->DrawGetTextureSize(m_CaptureBar[CAPTUREBAR_BG].m_iTexID, m_iCaptureBarTexWide, m_iCaptureBarTexTall);
}

//=========================================================
//=========================================================
void CCaptureBar::VidInit(void)
{

}

//=========================================================
//=========================================================
void CCaptureBar::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_iCaptureBarWide = scheme()->GetProportionalScaledValue(CAPTUREBAR_WIDE);
	m_iCaptureBarTall = scheme()->GetProportionalScaledValue(CAPTUREBAR_TALL);

	for(int i = 0; i < CAPTUREBAR_COUNT; i++)
	{
		CaptureBarTex_t &CaptureBarTex = m_CaptureBar[i];

	    CreateTexCoord(CaptureBarTex.m_TexCoords,
			0, 0,
			CAPTUREBAR_WIDE, CAPTUREBAR_TALL,
			m_iCaptureBarTexWide / CAPTUREBAR_SCALE, m_iCaptureBarTexTall / CAPTUREBAR_SCALE);
	}

	m_hCaptureFont = pScheme->GetFont("CaptureFont");
	m_iCaptureTextYPos = scheme()->GetProportionalScaledValue(CAPTUREBAR_TEXT_YPOS);
}

//=========================================================
//=========================================================
void CCaptureBar::Paint(void)
{
	C_INSPlayer *pLocalPlayer = C_INSPlayer::GetLocalPlayer();
    
	if(!pLocalPlayer)
		return;

	C_INSObjective *pObjective = pLocalPlayer->GetCurrentObj();

	if(!pObjective)
		return;

	// work out alpha first
	int iAlpha;
	float flDistance = UTIL_3DCaculateDistance(pObjective->GetMarker(), pLocalPlayer) * METERS_PER_INCH;
	int iRadius = pObjective->GetRadius();

	int iCaptureBarAlphaMaxLimit = (int)((float)iRadius * CAPTUREBAR_ALPHA_MAXLIMIT);
	int iCaptureBarAlphaMinLimit = (int)((float)iRadius * CAPTUREBAR_ALPHA_MINLIMIT);

	if(flDistance < iCaptureBarAlphaMaxLimit)
		iAlpha = 255;
	else if(flDistance > iCaptureBarAlphaMinLimit)
		iAlpha = CAPTUREBAR_ALPHA_MIN;
	else
		iAlpha = (int)((CAPTUREBAR_ALPHA_MIN - 255.0f) / (iCaptureBarAlphaMinLimit - iCaptureBarAlphaMaxLimit)) * (flDistance - iCaptureBarAlphaMaxLimit) + 255.0f;

	SetAlpha(iAlpha);

	// draw background
	DrawCaptureBar(CAPTUREBAR_BG, 1.0f, 255);

	int iPlayerTeamID = pLocalPlayer->GetTeamID();
	int iCapturingTeam = pObjective->GetCaptureTeam();
	int iCapturedTeam = pObjective->GetCapturedTeam();

	float flScrollFraction = 0.0f;
	int iFgAlpha = 255;

	int iNewCaptureText = CAPTURETEXT_NONE;

	if(iPlayerTeamID == iCapturedTeam)
	{
		flScrollFraction = 1.0f;

		iNewCaptureText = CAPTURETEXT_CAPTURED;
	}
	else if(iCapturingTeam != iPlayerTeamID)
	{
		flScrollFraction = 1.0f;

		float flAlphaFraction = sin(gpGlobals->curtime * 4.0f) + 1.5f;

		if(flAlphaFraction != 0.0f)
			iFgAlpha = 255.0f * (flAlphaFraction / 2.5f);
		else
			iFgAlpha = 0;

		iNewCaptureText = CAPTURETEXT_ENEMYBLOCK;
	}
	else if(pObjective->IsCapturing())
	{
		flScrollFraction = pObjective->GetCaptureFraction();

		iNewCaptureText = CAPTURETEXT_CAPTURE;
	}

	// set new capture text
	UpdateCaptureText(iNewCaptureText);

	// now draw the foreground
	DrawCaptureBar(CAPTUREBAR_FG, flScrollFraction, iFgAlpha);

	// paint the text
	if(m_iCaptureTextLength != 0)
	{
		Assert(m_wszCaptureText[0] != '\0');

		surface()->DrawSetTextColor(COLOR_WHITE);
		surface()->DrawSetTextFont(m_hCaptureFont);
		surface()->DrawSetTextPos(m_iCaptureTextXPos, m_iCaptureTextYPos);
		surface()->DrawPrintText(m_wszCaptureText, m_iCaptureTextLength, FONT_DRAW_NONADDITIVE);
	}
}

//=========================================================
//=========================================================
void CCaptureBar::DrawCaptureBar(int iCaptureBarID, float flFraction, int iAlpha)
{
	// check fraction
	if(flFraction <= 0.0f)
		return;

	// draw bar
	CaptureBarTex_t &CaptureBarTex = m_CaptureBar[iCaptureBarID];

	vgui::surface()->DrawSetTexture(CaptureBarTex.m_iTexID);
	vgui::surface()->DrawSetColor(Color(255, 255, 255, iAlpha));

	TexCoord_t TexCoords;

	if(flFraction != 1.0f)
	{
	    CreateTexCoord(TexCoords,
			0, 0,
			CAPTUREBAR_WIDE * flFraction, CAPTUREBAR_TALL,
			m_iCaptureBarTexWide / CAPTUREBAR_SCALE, m_iCaptureBarTexTall / CAPTUREBAR_SCALE);
	}
	else
	{
		memcpy(TexCoords, CaptureBarTex.m_TexCoords, sizeof(TexCoord_t));
	}

	vgui::surface()->DrawTexturedSubRect(0, 0, m_iCaptureBarWide * flFraction, m_iCaptureBarTall,
		TexCoords[0], TexCoords[1], TexCoords[2], TexCoords[3]);
}

//=========================================================
//=========================================================
void CCaptureBar::UpdateCaptureText(int iNewState)
{
	C_INSPlayer *pLocalPlayer = C_INSPlayer::GetLocalPlayer();
    
	if(!pLocalPlayer)
		return;

	C_INSObjective *pObjective = pLocalPlayer->GetCurrentObj();
	int iObjID = pObjective->GetOrderID();

	if(m_iCaptureTextLastObj == iObjID && m_iCaptureTextState == iNewState)
		return;

	m_iCaptureTextState = iNewState;
	m_iCaptureTextLastObj = iObjID;

	if(m_iCaptureTextState != CAPTURETEXT_NONE)
		Assert(pObjective);

	char szBuffer[256];
	szBuffer[0] = '\0';

	switch(m_iCaptureTextState)
	{
		case CAPTURETEXT_NONE:
			break;
		case CAPTURETEXT_CAPTURE:
			Q_snprintf(szBuffer, sizeof(szBuffer), "Securing Obj. %s (%s)", pObjective->GetPhonetischName(), pObjective->GetName());
			break;
		case CAPTURETEXT_CAPTURED:
			Q_snprintf(szBuffer, sizeof(szBuffer), "Secured Obj. %s (%s)", pObjective->GetPhonetischName(), pObjective->GetName());
			break;
		case CAPTURETEXT_ENEMYBLOCK:
			Q_snprintf(szBuffer, sizeof(szBuffer), "Obj. %s (%s) is being Defended", pObjective->GetPhonetischName(), pObjective->GetName());
			break;
	}

	UpdateCaptureText(szBuffer);
}

//=========================================================
//=========================================================
void CCaptureBar::UpdateCaptureText(const char *pszNewText)
{
	vgui::localize()->ConvertANSIToUnicode(pszNewText, m_wszCaptureText, sizeof(m_wszCaptureText));

	m_iCaptureTextLength = Q_strlen(pszNewText);
	m_iCaptureTextXPos = (m_iCaptureBarWide / 2) - (UTIL_ComputeStringWidth(m_hCaptureFont, pszNewText) / 2);
}