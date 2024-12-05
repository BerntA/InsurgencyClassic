//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef VGUI_INGAME_H
#define VGUI_INGAME_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/propertydialog.h>
#include <cl_dll/iviewport.h>
#include <vgui/ischeme.h>

class CMOTD;
class CMapInfo;
class CMapOver;
class CScoreboardManager;
class CPlayerObjetives;
class CComms;

// ------------------------------------------------------------------------------------ //
// Initial Screen for Player Setup
// ------------------------------------------------------------------------------------ //
class CGameInfo : public vgui::PropertyDialog, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CGameInfo, vgui::PropertyDialog);

public:
	CGameInfo(IViewPort *pViewPort);
	virtual ~CGameInfo();

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual const char *GetName(void) { return PANEL_GAMEINFO; }
	virtual void SetData(KeyValues *data) { };
	virtual void Update();
	virtual bool NeedsUpdate() { return false; };
	virtual bool HasInputElements(void) { return true; }
	virtual void Reset();
	virtual void ShowPanel(bool bShow);
	
	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

	void HandleMsg(bf_read &Msg);

protected:
	virtual void OnCommand(const char *command);

private:
	IViewPort *m_pViewPort;

	CMOTD *m_pMOTD;
	CMapInfo *m_pMapInfo;
	CMapOver *m_pMapOver;
	CScoreboardManager *m_pScoreboard;
	//CPlayerObjetives *m_pPlayerObjectives;
	CComms *m_pComms;
};

extern CGameInfo *g_pGameInfo;

#endif // VGUI_INGAME_H
