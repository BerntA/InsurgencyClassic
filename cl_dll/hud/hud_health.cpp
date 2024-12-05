//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud_macros.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "basic_colors.h"
#include "ins_player_shared.h"
#include "hud_messages.h"

#include <vgui/ischeme.h>
#include <vgui/isurface.h>
#include <vgui_controls/panel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define HEALTH_WIDE 66
#define HEALTH_TALL 88

#define HEALTH_YGAP 8

#define BG_PATH "sprites/hud/health/bg"
#define FG_PATH "sprites/hud/health/fg"

//=========================================================
//=========================================================
class CHudHealth : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudHealth, Panel);

public:
	CHudHealth(const char *pElementName);

private:
	void Init(void);

	void ApplySchemeSettings(IScheme *pScheme);

	void Paint(void);

private:
	int m_iBGTexID, m_iFGTexID;
	int m_iTexWide, m_iTexTall;
	int m_iYGap;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT(CHudHealth);

CHudHealth::CHudHealth(const char *pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "HudHealth")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetPaintBackgroundEnabled(false);

	SetHiddenBits(HIDEHUD_NOSTATUS);
}

//=========================================================
//=========================================================
void CHudHealth::Init(void)
{
	m_iBGTexID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_iBGTexID, BG_PATH, false, false);

	m_iFGTexID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_iFGTexID, FG_PATH, false, false);
}

//=========================================================
//=========================================================
void CHudHealth::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_iTexWide = scheme()->GetProportionalScaledValue(HEALTH_WIDE);
	m_iTexTall = scheme()->GetProportionalScaledValue(HEALTH_TALL);

	m_iYGap = scheme()->GetProportionalScaledValue(HEALTH_YGAP);
}

//=========================================================
//=========================================================
void CHudHealth::Paint(void)
{
	C_INSPlayer *pLocalPlayer = C_INSPlayer::GetLocalPlayer();

	if(!pLocalPlayer)
		return;

	int iYPos = GetTall() - (m_iYGap + g_pHudPlayerMsgs->GetOffsetHeight()) - m_iTexTall;

	vgui::surface()->DrawSetTexture(m_iBGTexID);
	vgui::surface()->DrawSetColor(COLOR_WHITE);
	vgui::surface()->DrawTexturedRect(0, iYPos, m_iTexWide, m_iTexTall + iYPos);

	vgui::surface()->DrawSetTexture(m_iFGTexID);
	vgui::surface()->DrawSetColor(C_INSPlayer::GetHealthColor(pLocalPlayer->GetHealthType()));
	vgui::surface()->DrawTexturedRect(0, iYPos, m_iTexWide, m_iTexTall + iYPos);
}