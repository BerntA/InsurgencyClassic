//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef VGUI_DEVPANEL_H
#define VGUI_DEVPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <cl_dll/iviewport.h>
#include <vgui_controls/insframe.h>

// ------------------------------------------------------------------------------------ //
// Dev Panel
// ------------------------------------------------------------------------------------ //
class CDevPanel : public vgui::INSFrame, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE(CDevPanel, vgui::INSFrame);

public:
	CDevPanel(IViewPort *pViewPort);

	virtual const char *GetName(void) { return PANEL_DEVPANEL; }
	virtual void SetData(KeyValues *pData) { }
	virtual void Update(void) { }
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual void Reset(void) { }
	virtual void ShowPanel(bool bShow);
	
	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

};

#endif // VGUI_DEVPANEL_H
