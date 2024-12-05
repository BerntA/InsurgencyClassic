//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud_boxed.h"
#include "iclientmode.h"
#include <vgui/ischeme.h>
#include <vgui/isurface.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define BOXED_SIZE 8
#define BOXED_WIDE BOXED_SIZE
#define BOXED_TALL BOXED_SIZE

#define BACKGROUND_COLOR Color(52, 52, 52, 192)

//=========================================================
//=========================================================
CHUDBoxed::CHUDBoxed(const char *pElementName, const char *pszPanelName)
	: CHudElement(pElementName), BaseClass(NULL, pszPanelName)
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	SetScheme(scheme);

	m_iEdgeBits = 0;
}

//=========================================================
//=========================================================
void CHUDBoxed::SetAllEdges(void)
{
	for(int i = 0; i < BOXEDEDGES_COUNT; i++)
		SetEdgeBit(i);
}

//=========================================================
//=========================================================
void CHUDBoxed::Init(void)
{
	char szTexPath[256];

	for(int i = 0; i < BOXEDEDGES_COUNT; i++)
	{
		Q_snprintf(szTexPath, sizeof(szTexPath), "HUD/boxed/edge%02i", i + 1);

		m_iEdgeTexID[i] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_iEdgeTexID[i], szTexPath, false, false);
	}
}

//=========================================================
//=========================================================
void CHUDBoxed::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_iEdgeWide = scheme()->GetProportionalScaledValue(BOXED_WIDE);
	m_iEdgeTall = scheme()->GetProportionalScaledValue(BOXED_TALL);
}

//=========================================================
//=========================================================
void CHUDBoxed::PaintBackground(void)
{
	int iWide, iTall;
	GetSize(iWide, iTall);

	surface()->DrawSetColor(BACKGROUND_COLOR);
	surface()->DrawFilledRect(m_iEdgeWide, 0, iWide - m_iEdgeWide, m_iEdgeTall);
	surface()->DrawFilledRect(0, m_iEdgeTall, iWide, iTall - m_iEdgeTall);
	surface()->DrawFilledRect(m_iEdgeWide, iTall - m_iEdgeTall,	iWide - m_iEdgeWide, iTall);

	for(int i = 0; i < BOXEDEDGES_COUNT; i++)
		DrawEdge(i);
}

//=========================================================
//=========================================================
void CHUDBoxed::DrawEdge(int iEdge)
{
	int iWide, iTall;
	GetSize(iWide, iTall);

	int iEdgeX1, iEdgeY1, iEdgeX2, iEdgeY2;
	iEdgeX1 = iEdgeY1 = iEdgeX2 = iEdgeY2 = 0;

	switch(iEdge)
	{
	case BOXEDEDGES_TOPRIGHT:
		iEdgeX2 = m_iEdgeWide;
		iEdgeY2 = m_iEdgeTall;
		break;
	case BOXEDEDGES_TOPLEFT:
		iEdgeX1 = iWide - m_iEdgeWide;
		iEdgeX2 = iWide;
		iEdgeY2 = m_iEdgeTall;
		break;
	case BOXEDEDGES_BOTTOMRIGHT:
		iEdgeX1 = iWide - m_iEdgeWide;
		iEdgeY1 = iTall - m_iEdgeTall;
		iEdgeX2 = iWide;
		iEdgeY2 = iTall;
		break;
	case BOXEDEDGES_BOTTOMLEFT:
		iEdgeY1 = iTall - m_iEdgeTall;
		iEdgeX2 = m_iEdgeWide;
		iEdgeY2 = iTall;
		break;
	}

	if(m_iEdgeBits & (1 << iEdge))
	{
		surface()->DrawSetColor(Color(255, 255, 255, 192));
		surface()->DrawSetTexture(m_iEdgeTexID[iEdge]);
		surface()->DrawTexturedRect(iEdgeX1, iEdgeY1, iEdgeX2, iEdgeY2);
	}
	else
	{
		surface()->DrawSetColor(BACKGROUND_COLOR);
		surface()->DrawFilledRect(iEdgeX1, iEdgeY1, iEdgeX2, iEdgeY2);
	}
}