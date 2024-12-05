//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUIHTMLWINDOW_H
#define VGUIHTMLWINDOW_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/panel.h>
#include <vgui_controls/html.h>

// ------------------------------------------------------------------------------------ //
// a Panel that has a HTML Window
// ------------------------------------------------------------------------------------ //
class CHTMLWindow : public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE(CHTMLWindow, vgui::Panel);

public:
	CHTMLWindow(vgui::Panel *pParent, const char *pszPanelName = NULL);

	void SetSizeParent(bool bSizeParent) { m_bSizeParent = bSizeParent; }

public:
	void LoadFile(const char *pszRelative);
	void LoadURL(const char *pszRelative);

protected:
	virtual void PerformLayout(void);

private:
	bool m_bSizeParent;
	vgui::HTML *m_pMessage;
};

#endif // VGUIHTMLWINDOW_H