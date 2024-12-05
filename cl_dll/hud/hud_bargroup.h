//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_BARGROUP_H
#define HUD_BARGROUP_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/panel.h>

//=========================================================
//=========================================================
class CHudBarElement;

class CHudBarGroup : public CHudElement, public vgui::Panel
{
	friend CHudBarElement;
	DECLARE_CLASS_SIMPLE(CHudBarGroup, vgui::Panel);

public:
	CHudBarGroup(const char *pszElementName);
	virtual ~CHudBarGroup();

	virtual void Init(void);

	static int LoadBarImage(const char *pszSuffix);

protected:
	virtual void Think(void);
	virtual void Paint(void);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	void ShowBar(int iID);
	void StartHideBar(int iID);

	void DrawElement(CHudBarElement *pElement, int iYPos, int iAlpha);

	void GetElementData(CHudBarElement *pElement, int &iYPos, int &iAlpha);
	int GetElementYPos(int iID);

	typedef void (CHudBarGroup::*ModifyElement_t)(int iID,CHudBarElement *pElement);
	void ModifyElements(ModifyElement_t ModifyElement, int iID);
	
	void RaiseElement(int iID, CHudBarElement *pElement);
	void LowerElement(int iID, CHudBarElement *pElement);

private:
	int m_iElementBGTex, m_iElementFGTex, m_iElementBorderTex;
	int m_iElementWide, m_iElementTall;
	TexCoord_t ElementBGTexCoord, ElementIconTexCoord;

	CUtlVector<CHudBarElement*> m_Elements;

	int m_iStartYPos;
	int m_iXPosGap, m_iYPosGap;
	int m_iIconSize;
};

#endif // HUD_BARGROUP_H