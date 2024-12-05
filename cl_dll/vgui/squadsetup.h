//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef VGUI_SQUADSETUP_H
#define VGUI_SQUADSETUP_H
#ifdef _WIN32
#pragma once
#endif

//#include <vgui_controls/propertyprogress.h>
#include <vgui_controls/insframe.h>
#include <cl_dll/iviewport.h>
#include <vgui/ischeme.h>

#include <vgui_controls/button.h>

class CPlayerOrder;
class CChangeSquad;
class CSquadMsg;
class CScoreboard;

// ------------------------------------------------------------------------------------ //
// Squad Setup - Ordered Choice
// ------------------------------------------------------------------------------------ //
class CSquadSetup : public vgui::INSFrame, public IViewPortPanel
{
	DECLARE_CLASS_SIMPLE(CSquadSetup, vgui::INSFrame);

	friend CSquadMsg;
	friend CPlayerOrder;

public:
	CSquadSetup(IViewPort *pViewPort);

	virtual const char *GetName(void) { return PANEL_SQUADSETUP; }
	virtual void SetData(KeyValues *data) { }
	virtual void Update(void) { }
	virtual bool NeedsUpdate() { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual void Reset(void);
	virtual void ShowPanel(bool bShow);
	
	vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

	void HandleMsg(bf_read &msg);
	void CurrentPlayerUpdate(void);
	//int GetCurrentPlayer(void) { return m_iCurrentPlayer; }
	bool IsActive(void) const { return m_bActive; }

protected:
	virtual void PerformLayout(void);
	virtual void OnThink(void);

private:
	void StartSelection(void);
	void ShowPlayerSelect(void);

	void ShowChangeSquadPanel(bool bState);

private:
	IViewPort *m_pViewPort;

	CSquadMsg *m_pSquadMsg;

	vgui::ImagePanel *m_pSelectionOrder;
	CPlayerOrder *m_pPlayerOrder;
	CScoreboard *m_pScoreboard;

	bool m_bActive;

	float m_flShowSquadTime;
	float m_flSquadCloseTime;

	bool m_bEnding;
	float m_flEndTime;
};

extern CSquadSetup *g_pSquadSetup;

#endif // VGUI_SQUADSETUP_H
