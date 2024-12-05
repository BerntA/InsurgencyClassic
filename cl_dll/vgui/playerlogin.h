//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef VGUI_PLAYERLOGIN_H
#define VGUI_PLAYERLOGIN_H
#ifdef _WIN32
#pragma once
#endif

#include "stats_protection.h"

#include <vgui_controls/insframe.h>
#include <cl_dll/iviewport.h>

class CLoginImage;
class CUsernameInfo;
class CPasswordInfo;
class CSavePassword;
class CLoginTicker;

// ------------------------------------------------------------------------------------ //
// Login Panel
// ------------------------------------------------------------------------------------ //
class CPlayerLogin : public vgui::INSFrame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CPlayerLogin, vgui::INSFrame);

public:
	CPlayerLogin(IViewPort *pViewPort);
	virtual ~CPlayerLogin();

	void LevelShutdown(void) { Reset(); }

	virtual const char *GetName(void) { return PANEL_PLAYERLOGIN; }
	virtual void SetData(KeyValues *pData) { }
	virtual void Update(void) { }
	virtual bool NeedsUpdate(void) { return false; };
	virtual bool HasInputElements(void) { return true; }
	virtual void Reset(void);
	virtual void ShowPanel(bool bShow);
	
	vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

	void HandleMsg(bf_read &Msg);
	void UpdateCanLogin(void);

protected:
	virtual void OnCommand(const char *pszCommand);

private:
	CUsernameInfo *m_pUsername;
	CPasswordInfo *m_pPassword;
	CSavePassword *m_pSavePassword;
	CLoginImage *m_pBackground;
	CLoginTicker *m_pTicker;

	int m_iButtonLogin, m_iButtonClose, m_iButtonReset;

#ifdef STATS_PROTECTION
public:
	bool m_bLoginSuccess;
#endif
};

extern CPlayerLogin *g_pPlayerLogin;

#endif // VGUI_PLAYERLOGIN_H
