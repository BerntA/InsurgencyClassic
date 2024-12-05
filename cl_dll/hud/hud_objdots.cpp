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
#include "ins_player_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define OBJDOT_DOT_PATH			"sprites/HUD/objdots/dot"
#define OBJDOT_EMPTYDOT_PATH	"sprites/HUD/objdots/emptydot"
#define OBJDOT_UNNEEDED_PATH	"sprites/HUD/objdots/unneeded"

#define OBJDOT_SIZE 16
#define OBJDOT_XGAP 8

//=========================================================
//=========================================================
class CObjDots : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CObjDots, Panel);

public:
	CObjDots(const char *pElementName);

	void Init(void);
	void VidInit(void);

protected:
	void ApplySchemeSettings(vgui::IScheme *pScheme);

	void Think(void);
	void Paint(void);

private:
	void LoadDot(int &iTexID, const char *pszPath);

private:
	int m_iDotFilledTexID, m_iDotEmptyTexID, m_iDotUnneededTexID;
	int m_iDotSize;

	int m_iDotXGap;

	int m_iDrawCount, m_iRequiredToCapture, m_iNumCapturing;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT(CObjDots);

CObjDots::CObjDots(const char *pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "ObjDots")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetPaintBackgroundEnabled(false);

	SetHiddenBits(HIDEHUD_SPECTATOR);

	m_iDrawCount = m_iRequiredToCapture = m_iNumCapturing = 0;
}

//=========================================================
//=========================================================
void CObjDots::Init(void)
{
	LoadDot(m_iDotFilledTexID, OBJDOT_DOT_PATH);
	LoadDot(m_iDotEmptyTexID, OBJDOT_EMPTYDOT_PATH);
	LoadDot(m_iDotUnneededTexID, OBJDOT_UNNEEDED_PATH);
}

//=========================================================
//=========================================================
void CObjDots::VidInit(void)
{

}

//=========================================================
//=========================================================
void CObjDots::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_iDotSize = scheme()->GetProportionalScaledValue(OBJDOT_SIZE);
	m_iDotXGap = scheme()->GetProportionalScaledValue(OBJDOT_XGAP);
}

//=========================================================
//=========================================================
void CObjDots::Think(void)
{
	C_INSPlayer *pLocalPlayer = C_INSPlayer::GetLocalPlayer();
    
	if(!pLocalPlayer)
		return;

	C_INSObjective *pObjective = pLocalPlayer->GetCurrentObj();

	int iLocalTeam = pLocalPlayer->GetTeamID();

	// not in an objective?
	if(!pObjective ||
		pObjective->GetCapturedTeam() == iLocalTeam ||
		pObjective->GetCaptureTeam() != iLocalTeam)
	{
		m_iDrawCount = 0;
		return;
	}

	int iScreenWide, iScreenHeight, iTotalSize;

	m_iRequiredToCapture = pObjective->GetRequiredPlayers();
	m_iNumCapturing = pObjective->GetPlayersCapturing();

	if(m_iRequiredToCapture == 0)
	{
		m_iDrawCount = 0;
		return;
	}

	int iDrawCount = max(m_iRequiredToCapture, m_iNumCapturing);

	if(m_iDrawCount == iDrawCount)
		return;

	m_iDrawCount = iDrawCount;

	// work out position
	engine->GetScreenSize(iScreenWide, iScreenHeight);
	iTotalSize = iDrawCount * (m_iDotSize + m_iDotXGap) - m_iDotXGap;

	SetBounds((iScreenWide * 0.5) - (iTotalSize * 0.5), GetYPos(), iTotalSize, m_iDotSize);
}

//=========================================================
//=========================================================
void CObjDots::Paint(void)
{
	// now draw the sucka's
	int iDotXPos = 0;

	for(int i = 1; i <= m_iDrawCount; i++)
	{
		int iDotTexID;
		
		if(i > m_iRequiredToCapture)
			iDotTexID = m_iDotUnneededTexID;
		else if(i > m_iNumCapturing)
			iDotTexID = m_iDotEmptyTexID;
		else
			iDotTexID = m_iDotFilledTexID;

		vgui::surface()->DrawSetTexture(iDotTexID);
		vgui::surface()->DrawSetColor(Color(255, 255, 255, 255));
		vgui::surface()->DrawTexturedRect(iDotXPos, 0, iDotXPos + m_iDotSize, m_iDotSize);

		iDotXPos += m_iDotXGap + m_iDotSize;
	}
}

//=========================================================
//=========================================================
void CObjDots::LoadDot(int &iTexID, const char *pszPath)
{
	iTexID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(iTexID, pszPath, false, false);
}
