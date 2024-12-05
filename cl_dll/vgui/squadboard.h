//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef VGUI_SQUADBOARD_H
#define VGUI_SQUADBOARD_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/insframe.h>
#include <cl_dll/iviewport.h>

class CScoreboard;

// ------------------------------------------------------------------------------------ //
// Initial Screen for Player Setup
// ------------------------------------------------------------------------------------ //
class CSquadBoard : public vgui::INSFrame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CSquadBoard, vgui::INSFrame);

public:
	CSquadBoard(IViewPort *pViewPort);

	virtual const char *GetName(void) { return PANEL_QSCOREBOARD; }
	virtual void SetData(KeyValues *pData) { };
	virtual void Update(void) { }
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return false; }
	virtual void Reset(void) { }
	virtual void ShowPanel(bool bShow);
	
	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

protected:
	virtual void PerformLayout(void);

private:
	CScoreboard *m_pScoreboard;
};

#endif // VGUI_SQUADBOARD_H
