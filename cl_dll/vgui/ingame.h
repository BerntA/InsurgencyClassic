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
class CScoreBoard;
class CPlayerObjetives;
class CComms;

// ------------------------------------------------------------------------------------ //
// Initial Screen for Player Setup
// ------------------------------------------------------------------------------------ //
class CInGame : public vgui::PropertyDialog, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CInGame, vgui::PropertyDialog);

public:
	CInGame(IViewPort *pViewPort);
	virtual ~CInGame();

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual const char *GetName(void) { return PANEL_IGPLAYER; }
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

protected:
	virtual void OnCommand(const char *command);

private:
	IViewPort *m_pViewPort;

	CMOTD *m_pMOTD;
	CMapInfo *m_pMapInfo;
	CScoreBoard *m_pScoreBoard;
	//CPlayerObjetives *m_pPlayerObjectives;
	CComms *m_pComms;
};

#endif // VGUI_INGAME_H
