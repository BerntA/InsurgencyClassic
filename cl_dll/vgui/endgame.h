//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef VGUI_ENDGAME_H
#define VGUI_ENDGAME_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/propertydialog.h>
#include <cl_dll/iviewport.h>
#include <vgui/ischeme.h>

class CGameResult;

struct Result_t
{
	int m_iTeam;
	int m_iType;
	int m_iLength;
	int m_iBalance;
};

// ------------------------------------------------------------------------------------ //
// End Game Panel
// ------------------------------------------------------------------------------------ //
class CEndGame : public vgui::PropertyDialog, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CEndGame, vgui::PropertyDialog);

public:
	CEndGame(IViewPort *pViewPort);

	virtual const char *GetName(void) { return PANEL_ENDGAME; }
	virtual void SetData(KeyValues *data);
	virtual void Update() { }
	virtual bool NeedsUpdate() { return false; };
	virtual bool HasInputElements(void) { return true; }
	virtual void Reset();
	virtual void ShowPanel(bool bShow);
	
	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

protected:
	virtual void OnCommand(const char *command);

private:
	CGameResult *m_pGameResult;
};

#endif // VGUI_ENDGAME_H
