//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_BOXED_H
#define HUD_BOXED_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/panel.h>

//=========================================================
//=========================================================
enum BoxedEdges_t
{
	BOXEDEDGES_TOPRIGHT = 0,
	BOXEDEDGES_TOPLEFT,
	BOXEDEDGES_BOTTOMRIGHT,
	BOXEDEDGES_BOTTOMLEFT,
	BOXEDEDGES_COUNT
};

//=========================================================
//=========================================================
class CHUDBoxed : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHUDBoxed, Panel);

public:
	CHUDBoxed(const char *pElementName, const char *pszPanelName);

protected:
	void SetAllEdges(void);
	void SetEdgeBit(int iEdgeBit) { m_iEdgeBits |= (1 << iEdgeBit); }

	virtual void Init(void);

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	void PaintBackground(void);

private:
	void DrawEdge(int iEdge);

private:
	int m_iEdgeBits;

	int m_iEdgeTexID[BOXEDEDGES_COUNT];

	int m_iEdgeWide, m_iEdgeTall;
};

#endif // HUD_BOXED_H