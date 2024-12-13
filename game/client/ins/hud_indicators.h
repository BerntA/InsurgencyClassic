//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_INDICATORS_H
#define HUD_INDICATORS_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/panel.h>

//=========================================================
//=========================================================
class CHUDIndicators : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHUDIndicators, Panel);

public:
	CHUDIndicators(const char *pElementName, const char *pszPanelName);

protected:
	void DrawArrow(Vector &vecOrigin, Color &Color);

private:
	int m_iArrowTexID;
};

#endif // HUD_INDICATORS_H