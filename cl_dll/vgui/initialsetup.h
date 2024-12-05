//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef VGUI_INITIALSETUP_H
#define VGUI_INITIALSETUP_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/propertydialog.h>
#include <cl_dll/iviewport.h>
#include <vgui/ischeme.h>

class CMOTD;
class CHTMLWindow;
class CMapInfo;

// ------------------------------------------------------------------------------------ //
// Initial Screen for Player Setup
// ------------------------------------------------------------------------------------ //
class CInitialSetup : public vgui::PropertyDialog, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CInitialSetup, vgui::PropertyDialog);

public:
	CInitialSetup(IViewPort *pViewPort);

	virtual const char *GetName(void) { return PANEL_INITIALSETUP; }
	virtual void SetData(KeyValues *pData) { }
	virtual void Update(void) { }
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual void Reset(void);
	virtual void ShowPanel(bool bShow);
	
	vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

protected:
	virtual void OnCommand(const char *pszCommand);

private:
	IViewPort *m_pViewPort;

	CMOTD *m_pMOTD;
	CHTMLWindow *m_pCredits;
	CMapInfo *m_pMapInfo;
};

#endif // VGUI_INITIALSETUP_H
