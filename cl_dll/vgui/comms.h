//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef COMMS_H
#define COMMS_H
#ifdef _WIN32
#pragma once
#endif

#include <cl_dll/iviewport.h>
#include <vgui_controls/propertypage.h>
#include <vgui_controls/propertysheet.h>
#include "vgui_basepanel.h"
#include <igameevents.h>

class CGameInfo;
class CWindowSelection;
class CPlayerSelection;
class CMessageWindow;
class CMessageEntry;

//=========================================================
//=========================================================
#define THIRDPERSON_PREFIX "/me "
#define THIRDPERSON_PREFIX_LENGTH 4

enum ChatWindows_t
{
	CHATMODE_GLOBAL = 0,
	CHATMODE_TEAM,
	CHATMODE_SQUAD,
	CHATMODE_COUNT
};

struct WindowID_t
{
	bool m_bPrivateMsg;
	int m_iID;

	bool operator==(WindowID_t &WindowID) const
	{
		return (m_bPrivateMsg == WindowID.m_bPrivateMsg && m_iID == WindowID.m_iID);
	}

	bool operator==(const WindowID_t &WindowID) const
	{
		return (m_bPrivateMsg == WindowID.m_bPrivateMsg && m_iID == WindowID.m_iID);
	}
};

class CCommsComponent
{
public:
	virtual void SetActiveWindow(const WindowID_t &WindowID) = 0;
	virtual void BlockPlayer(int iID, bool bState) = 0;
};

//=========================================================
//=========================================================
class CComms : public vgui::PropertyPage, public IGameEventListener2, public CCommsComponent
{
	DECLARE_CLASS_SIMPLE(CComms, vgui::PropertyPage);

public:
	CComms(CGameInfo *pParent);
	virtual ~CComms();

	void HandleMsg(bf_read &msg);
	void HandleMsg(const char *pszString);
	void PrintMsg(const WindowID_t &TargetWindow, CScrollerMsg PrintMsg, bool bLocalPlayer);

	void SendMsg(void);
	void SendMsg(const char *pszText, int iType);

	void SetActiveWindow(const WindowID_t &WindowID, int iWindowID);
	void SetActiveWindow(const WindowID_t &WindowID);
	void SetActiveWindow(bool bPrivateMsg, int iID);

	CMessageWindow *GetActiveWindow(void);

	int GetWindowID(const WindowID_t &WindowID);
	CMessageWindow *GetWindow(const WindowID_t &WindowID);



	int CreateWindow(const WindowID_t &WindowID);
	int CreateWindow(bool bPrivateMsg, int iID);

	void DestroyWindow(const WindowID_t &WindowID);
	void DestroyWindow(bool bPrivateMsg, int iID);

	bool IsPlayerBlocked(int iID) const;
	void BlockPlayer(int iID, bool bState);

	MESSAGE_FUNC(OnPageShow, "PageShow");
	MESSAGE_FUNC(OnPageHide, "PageHide");
	MESSAGE_FUNC(OnResetData, "ResetData");

protected:
	virtual void FireGameEvent(IGameEvent *pEvent);

private:
	int FindWindow(const WindowID_t &WindowID);

	void SetEntryEnabled(bool bState);


	const char *GetChatModeName(int iMode);

private:
	CGameInfo *m_pGameInfo;

	// Visual Elements
	CWindowSelection *m_pWindowSelection;
	CPlayerSelection *m_pPlayerSelection;

	CUtlVector<CMessageWindow*> m_MsgWindows;

	CMessageEntry *m_pMessageEntry;

	// Stored Information
	int m_iActiveWindow;
	bool m_bBlocked[MAX_PLAYERS+1];

	CUtlVector<WindowID_t> m_ActiveOrder;
};

extern CComms *g_pCommsPanel;

#endif	//COMMS_H
