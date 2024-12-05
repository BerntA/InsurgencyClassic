//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "comms.h"
#include "hud_macros.h"
#include "text_message.h"
#include "vguicenterprint.h"
#include "iclientmode.h"
#include <vgui/ilocalize.h>
#include <vgui/ivgui.h>

#include <vgui_controls/button.h>
#include <vgui_controls/textentry.h>
#include <vgui_controls/richtext.h>
#include <vgui_controls/sectionedlistpanel.h>

#include "ins_gamerules.h"
#include "c_playerresource.h"
#include "c_team.h"
#include "voice_status.h"
#include "hud_messages.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CComms *g_pCommsPanel = NULL;

using namespace vgui;

#define MAX_COMMS_STRING TextEntry::MAX_COMPOSITION_STRING

// TODO: "allowprivatemgs" needs to tied into this code

//=========================================================
//=========================================================
class CMessageWindow : public EditablePanel
{
	DECLARE_CLASS_SIMPLE(CMessageWindow, EditablePanel);

private:
	CComms *m_pComms;

	WindowID_t m_WindowID;

	RichText *m_pRichText;

	Button *m_pClose;
	Button *m_pBlock, *m_pUnblock;

	Color m_LocalPlayerText;
	Color m_ForeignPlayerText;
	Color m_ThirdPersonText;
	Color m_AnnouncementText;

public:
	CMessageWindow(CComms *pParent, const char *pszPanelName, const WindowID_t &WindowID)
		: EditablePanel(pParent, pszPanelName)
	{
		m_pComms = pParent;

		SetProportional(true);
		SetPaintBackgroundEnabled(false);

		m_WindowID = WindowID;

		m_pRichText = new RichText(this, "Messages");

		m_pClose = new Button(this, "Close", "Close", this, "Close");
		m_pBlock = new Button(this, "Block", "Block", this, "Block");
		m_pUnblock = new Button(this, "Unblock", "Unblock", this, "Unblock");

		LoadControlSettings("Resource/UI/Frames/Panels/MessageWindow.res");

		//deathz0rz [
		InvalidateLayout(false,true); //we need the scheme settings for the message mode
		//deathz0rz ]
	}

	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		m_LocalPlayerText = GetSchemeColor("CommsMessageWindow.LocalPlayerText", GetFgColor(), pScheme);
		m_ForeignPlayerText = GetSchemeColor("CommsMessageWindow.ForeignPlayerText", GetFgColor(), pScheme);
		m_ThirdPersonText = GetSchemeColor("CommsMessageWindow.ThirdPersonText", GetFgColor(), pScheme);
		m_AnnouncementText = GetSchemeColor("CommsMessageWindow.AnnouncementText", GetFgColor(), pScheme);
	}

	void PrintMsg(CScrollerMsg &PrintMsg, bool bLocalPlayer)
	{
		// NOTE: this needs to work how the HUD handles line, so I
		//		can start removing them after so many

		PrintMsg.SetDefaultColor(bLocalPlayer ? m_LocalPlayerText : m_ForeignPlayerText);
		PrintMsg.AddColorLookup(SCROLLERLOOKUP_THIRDPERSON, m_ThirdPersonText);
		PrintMsg.AddColorLookup(SCROLLERLOOKUP_SERVERTEXT, m_AnnouncementText);

		CScrollerMsg::AddRichText(PrintMsg, m_pRichText);

		m_pRichText->InsertString("\n");
	}

	void PrintAnnouncement(const char *pszString)
	{
		m_pRichText->InsertColorChange(m_AnnouncementText);
		m_pRichText->InsertString(pszString);
		m_pRichText->InsertString("\n");
	}

	const WindowID_t &GetWindowID(void) const { return m_WindowID; }

protected:
	virtual void OnCommand(const char *pszCommand)
	{
		if(!strcmp(pszCommand, "Close"))
		{
			Assert(m_WindowID.m_bPrivateMsg);

			m_pComms->DestroyWindow(m_WindowID);
			return;
		}
		else if(!strcmp(pszCommand, "Block"))
		{
			Assert(m_WindowID.m_bPrivateMsg);

			SetBlocked(true);
			m_pComms->BlockPlayer(m_WindowID.m_iID, true);
			return;
		}
		else if(!strcmp(pszCommand, "Unblock"))
		{
			Assert(m_WindowID.m_bPrivateMsg);

			SetBlocked(false);
			m_pComms->BlockPlayer(m_WindowID.m_iID, false);
			return;
		}

		BaseClass::OnCommand(pszCommand);
	}

	virtual void PerformLayout(void)
	{
		BaseClass::PerformLayout();

		if(m_WindowID.m_bPrivateMsg)
		{
			SetBlocked(m_pComms->IsPlayerBlocked(m_WindowID.m_iID));
		}
		else
		{
			m_pClose->SetEnabled(false);
			m_pBlock->SetEnabled(false);
			m_pUnblock->SetVisible(false);
		}
	}

private:
	void SetBlocked(bool bState)
	{
		m_pBlock->SetVisible(!bState);
		m_pUnblock->SetVisible(bState);
	}
};

//=========================================================
//=========================================================
class CCommsSelection : public EditablePanel, public CCommsComponent
{
	DECLARE_CLASS_SIMPLE(CCommsSelection, Panel);

public:
	CCommsSelection(CComms *pParent, const char *pszPanelName);

	void Reset(void);

protected:
	virtual void ApplySchemeSettings(IScheme *pScheme);
	virtual void InitListSections(void) = 0;

protected:
	CComms *m_pComms;

	enum { NAME_WIDTH = 100 };
	#define SECTION_HEAD 0

	SectionedListPanel *m_pList;
};

CCommsSelection::CCommsSelection(CComms *pParent, const char *pszPanelName)
: EditablePanel(pParent, pszPanelName)
{
	m_pComms = pParent;

	SetPaintBackgroundEnabled(false);

	m_pList = new SectionedListPanel(this, NULL);
}

void CCommsSelection::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// assign the same size
	int iWide, iTall;
	GetSize(iWide, iTall);
	
	m_pList->SetSize(iWide, iTall);
}

void CCommsSelection::Reset(void)
{
	m_pList->DeleteAllItems();
	m_pList->RemoveAllSections();

	InitListSections();
}

//=========================================================
//=========================================================
class CWindowSelection : public CCommsSelection
{
	DECLARE_CLASS_SIMPLE(CWindowSelection, CCommsSelection);

public:
	CWindowSelection(CComms *pParent, const char *pszPanelName);

	void AddWindow(const char *pszName, const WindowID_t &WindowID);
	void RemoveWindow(const WindowID_t &WindowID);

	virtual void SetActiveWindow(const WindowID_t &WindowID);
	virtual void BlockPlayer(int iID, bool bState) { }

protected:
	virtual void ApplySchemeSettings(IScheme *pScheme);

	MESSAGE_FUNC_INT(ItemLeftClick, "ItemLeftClick", itemID);

	void InitListSections(void);

private:
	int FindItemIDForWindowID(const WindowID_t &WindowID);

private:
	Color m_PublicText;

private:
	#define SECTION_WINDOW SECTION_HEAD
};

CWindowSelection::CWindowSelection(CComms *pParent, const char *pszPanelName)
	: CCommsSelection(pParent, pszPanelName)
{
	m_pList->SetIgnoreFocusChange(true);
}

void CWindowSelection::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_PublicText = GetSchemeColor("CommsWindowSelection.PublicText", GetFgColor(), pScheme);
}

void CWindowSelection::AddWindow(const char *pszName, const WindowID_t &WindowID)
{
	KeyValues *pWindowData = new KeyValues("data");
	pWindowData->SetString("name", pszName);
	pWindowData->SetInt("pmsg", WindowID.m_bPrivateMsg);
	pWindowData->SetInt("id", WindowID.m_iID);

	m_pList->AddItem(SECTION_WINDOW, pWindowData);
	//int iItemID = m_pList->AddItem(SECTION_WINDOW, pWindowData);

	//if(!WindowID.m_bPrivateMsg)
	//	m_pList->SetItemFgColor(iItemID, m_PublicText);

	pWindowData->deleteThis();
}

void CWindowSelection::RemoveWindow(const WindowID_t &WindowID)
{
	int iItemID = FindItemIDForWindowID(WindowID);
	Assert(iItemID != -1);

	m_pList->RemoveItem(iItemID);
}

void CWindowSelection::SetActiveWindow(const WindowID_t &WindowID)
{
	m_pList->SetSelectedItem(FindItemIDForWindowID(WindowID));
}

void CWindowSelection::ItemLeftClick(int iItemID)
{
	if(iItemID == -1)
		return;

	KeyValues *pItemKV = m_pList->GetItemData(iItemID);
	WindowID_t WindowID = { pItemKV->GetInt("pmsg"), pItemKV->GetInt("id") };

	m_pComms->SetActiveWindow(WindowID);
}

void CWindowSelection::InitListSections(void)
{
	m_pList->AddSection(SECTION_WINDOW, "");
	m_pList->SetSectionAlwaysVisible(SECTION_WINDOW, true);
	m_pList->SetSectionHeaderVisible(SECTION_WINDOW, false);

	m_pList->AddColumnToSection(SECTION_WINDOW, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH));
}

int CWindowSelection::FindItemIDForWindowID(const WindowID_t &WindowID)
{
	for(int i = 0; i <= m_pList->GetHighestItemID(); i++)
	{
		if(m_pList->IsItemIDValid(i))
		{
			KeyValues *pItemKV = m_pList->GetItemData(i);

			if(pItemKV && 
				(pItemKV->GetInt("pmsg") ? true : false) == WindowID.m_bPrivateMsg &&
				pItemKV->GetInt("id") == WindowID.m_iID)
			{
				return i;
			}
		}
	}

	return -1;
}

//=========================================================
//=========================================================
class CPlayerSelection : public CCommsSelection
{
	DECLARE_CLASS_SIMPLE(CPlayerSelection, CCommsSelection);

public:
	CPlayerSelection(CComms *pParent, const char *pszPanelName);

	virtual void SetActiveWindow(const WindowID_t &WindowID) { Update(); }
	virtual void BlockPlayer(int iID, bool bState) { Update(); }

protected:
	virtual void ApplySchemeSettings(IScheme *pScheme);

	MESSAGE_FUNC_INT(ItemDoubleLeftClick, "ItemDoubleLeftClick", itemID);

	virtual void OnTick(void);

	void InitListSections(void);

private:
	static bool StaticPlayerSortFunc(SectionedListPanel *list, int itemID1, int itemID2);

	void Update(void);

	int FindItemIDForPlayerIndex(int iPlayerIndex);

	Color &FindPlayerColour(int iPlayerID);

private:
	#define SECTION_PLAYERS SECTION_HEAD

	Color m_NormalTextColor;
	Color m_OutsideGroup;
	Color m_BlockedColor;
};

CPlayerSelection::CPlayerSelection(CComms *pParent, const char *pszPanelName)
	: CCommsSelection(pParent, pszPanelName)
{
	ivgui()->AddTickSignal(GetVPanel(), 1000);
}

void CPlayerSelection::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_NormalTextColor = GetSchemeColor("SectionedListPanel.TextColor", pScheme);
	m_OutsideGroup = GetSchemeColor("CommsPlayerSelection.OutsideGroup", GetFgColor(), pScheme);
	m_BlockedColor = GetSchemeColor("CommsPlayerSelection.Blocked", GetFgColor(), pScheme);
}

void CPlayerSelection::ItemDoubleLeftClick(int iItemID)
{
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	KeyValues *pItemKV = m_pList->GetItemData(iItemID);

	int iSelectedPlayerIndex = pItemKV->GetInt("playerIndex");

	if(pLocalPlayer->entindex() == iSelectedPlayerIndex)
	{
		m_pList->DeselectCurrentItem();
		return;
	}

	WindowID_t WindowID = { true, iSelectedPlayerIndex };

	m_pComms->SetActiveWindow(WindowID);
}

void CPlayerSelection::OnTick(void)
{
	Update();
}

void CPlayerSelection::InitListSections(void)
{
	m_pList->AddSection(SECTION_PLAYERS, "", NULL, StaticPlayerSortFunc);
	m_pList->SetSectionAlwaysVisible(SECTION_PLAYERS, true);
	m_pList->SetSectionHeaderVisible(SECTION_PLAYERS, false);

	m_pList->AddColumnToSection(SECTION_PLAYERS, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH));
}

bool CPlayerSelection::StaticPlayerSortFunc(SectionedListPanel *list, int itemID1, int itemID2)
{
	KeyValues *it1 = list->GetItemData(itemID1);
	KeyValues *it2 = list->GetItemData(itemID2);
	Assert(it1 && it2);

	// compare names
	return (strcmp(it1->GetString("name"), it2->GetString("name")) <= 0);
}

void CPlayerSelection::Update(void)
{
	if(g_Teams.Count() == 0)
		return;

	IGameResources *gr = GameResources();

	for(int i = 1; i < MAX_PLAYERS; i++)
	{
		int iItemID = FindItemIDForPlayerIndex(i);

		if(g_PR->IsConnected(i))
		{
			char szName[MAX_PLAYER_NAME_LENGTH];
			UTIL_MakeSafeName(gr->GetPlayerName(i), szName);

			KeyValues *pPlayerData = new KeyValues("data");
			pPlayerData->SetString("name", szName);
			pPlayerData->SetInt("playerIndex", i);

			if(iItemID == -1)
				iItemID = m_pList->AddItem(SECTION_PLAYERS, pPlayerData);
			else
				m_pList->ModifyItem(iItemID, SECTION_PLAYERS, pPlayerData);

			pPlayerData->deleteThis();

			m_pList->SetItemFgColor(iItemID, FindPlayerColour(i));
		}
		else
		{
			if(iItemID != -1)
				m_pList->RemoveItem(iItemID);
		}
	}
}

int CPlayerSelection::FindItemIDForPlayerIndex(int iPlayerIndex)
{
	for(int i = 0; i <= m_pList->GetHighestItemID(); i++)
	{
		if(m_pList->IsItemIDValid(i))
		{
			KeyValues *pItemKV = m_pList->GetItemData(i);

			if(pItemKV && (pItemKV->GetInt("playerIndex") == iPlayerIndex))
				return i;
		}
	}

	return -1;
}

Color &CPlayerSelection::FindPlayerColour(int iPlayerID)
{
	if(m_pComms->IsPlayerBlocked(iPlayerID))
		return m_BlockedColor;

	const WindowID_t &WindowID = m_pComms->GetActiveWindow()->GetWindowID();

	if(WindowID.m_bPrivateMsg)
	{
		if(WindowID.m_iID == iPlayerID)
			return m_NormalTextColor;
		else
			return m_OutsideGroup;
	}

	switch(WindowID.m_iID)
	{
		case CHATMODE_GLOBAL:
		{
			return m_NormalTextColor;
		}
        case CHATMODE_TEAM:
		{
			if(g_PR && g_PR->GetTeamID(WindowID.m_iID) == g_PR->GetTeamID(iPlayerID))
				return m_NormalTextColor;
			else
				return m_OutsideGroup;
		}
		case CHATMODE_SQUAD:
		{
			// NOTE: Finish This
			return m_NormalTextColor;
		}
	}

	return m_NormalTextColor;
}

//=========================================================
//=========================================================
class CMessageEntry : public TextEntry
{
	DECLARE_CLASS_SIMPLE(CMessageEntry, TextEntry);

private:
	CComms *m_pComms;

public:
	CMessageEntry(CComms *pParent, const char *pszPanelName)
		: BaseClass(pParent, pszPanelName)
	{
		m_pComms = pParent;

		SetCatchEnterKey(true);
		SetAllowNonAsciiCharacters(true);
		SetDrawLanguageIDAtLeft(true);
	}

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);
		SetPaintBorderEnabled(true);
	}

	void ClearEntry(void)
	{
		SetText(L"");
	}

	virtual void OnKeyCodeTyped(vgui::KeyCode code)
	{
		if(code == vgui::KEY_ENTER || code == vgui::KEY_PAD_ENTER)
			m_pComms->SendMsg();
		else if(code == vgui::KEY_TAB)
			return;
		else
			BaseClass::OnKeyCodeTyped( code );
	}
};

//=========================================================
//=========================================================
/*void __MsgFunc_TextMsg(bf_read &msg)
{
	char szString[2048];
	int iMsgDest = msg.ReadByte();
	
	msg.ReadString(szString, sizeof(szString));

	switch(iMsgDest)
	{
	case TEXTMSG_CENTER:
		internalCenterPrint->Print(szString);
		break;

	case TEXTMSG_CONSOLE:
		Msg(szString);
		break;

	case TEXTMSG_HUD:
		g_pCommsPanel->HandleMsg(szString);
		break;
	}
}

void __MsgFunc_SayText(bf_read &msg)
{
	if(!g_pCommsPanel)
		return;

	g_pCommsPanel->HandleMsg(msg);
}*/

//=========================================================
//=========================================================
CComms::CComms(CGameInfo *pParent)
	: PropertyPage((Panel*)pParent, PANEL_COMMS)
{
	// setup the global pointer
	Assert(!g_pCommsPanel);
	g_pCommsPanel = this;

	// set the scheme
	SetScheme("ClientScheme");

	// set the gameinfo
	m_pGameInfo = pParent;

	// setup the messages
	HOOK_MESSAGE(SayText);
	HOOK_MESSAGE(TextMsg);

	// listen for players disconnecting
	gameeventmanager->AddListener(this, "player_changename", false);
	gameeventmanager->AddListener(this, "player_disconnect", false);

	// setup blocked players
	memset(m_bBlocked, false, sizeof(m_bBlocked));

	// setup the selection windows
	m_pWindowSelection = new CWindowSelection(this, "WindowSelection");
	m_pPlayerSelection = new CPlayerSelection(this, "PlayerSelection");

	m_pWindowSelection->Reset();
	m_pPlayerSelection->Reset();

	// setup basic chat windows
	for(int i = 0; i < CHATMODE_COUNT; i++)
	{
		CreateWindow(false, i);
	}

	// setup the message entry
	m_pMessageEntry = new CMessageEntry(this, "MsgEntry");

	// set the default window
	SetActiveWindow(false, CHATMODE_GLOBAL);

	// load resource
	LoadControlSettings("Resource/UI/Frames/Panels/Comms.res");
}

//=========================================================
//=========================================================
CComms::~CComms()
{
	g_pCommsPanel = NULL;

	gameeventmanager->RemoveListener(this);
}

//=========================================================
//=========================================================
void CComms::HandleMsg(bf_read &Msg)
{
	char szString[MAX_CHATMSG_LENGTH];
	int iType, iSenderClient;

	iType = iSenderClient = 0;

	// collect information
	iType = Msg.ReadByte();

	if(iType & ~CHATFLAG_SERVER)
		iSenderClient = Msg.ReadByte();

	Msg.ReadString(szString, sizeof(szString));

	// find local player
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	// parse information
	if(!szString[0])
		return;

	WindowID_t TargetWindow;
	TargetWindow.m_bPrivateMsg = false;

	bool bThirdPerson = (iType & CHATFLAG_THIRDPERSON);
	bool bAnnouncement = (iType & CHATFLAG_SERVER);
	bool bLocalPlayer = (iSenderClient == pPlayer->entindex());

	if(bThirdPerson && bAnnouncement)
	{
		Assert(0);
		return;
	}

	if(iType & CHATFLAG_SERVER || iType & CHATFLAG_GLOBAL)
	{
		TargetWindow.m_iID = CHATMODE_GLOBAL;
	}
	else if(iType & CHATFLAG_TEAM)
	{
		TargetWindow.m_iID = CHATMODE_TEAM;
	}
	else if(iType & CHATFLAG_SQUAD)
	{
		TargetWindow.m_iID = CHATMODE_SQUAD;
	}
	else if(iType & CHATFLAG_PRIVATE)
	{
		TargetWindow.m_bPrivateMsg = true;
		TargetWindow.m_iID = iSenderClient;
	}

	CScrollerMsg PrintMsg;
	ParseTextMsg(PrintMsg, iSenderClient, iType, szString);

	// send it to both the VGUI and the HUD
	CComms::PrintMsg(TargetWindow, PrintMsg, bLocalPlayer);
	g_pHudPlayerMsgs->PrintMsg(PrintMsg, iType);
}

//=========================================================
//=========================================================
void CComms::HandleMsg(const char *pszString)
{
	WindowID_t TargetWindow = { false, CHATMODE_GLOBAL };

	CScrollerMsg PrintMsg;
	PrintMsg.AddMessage(pszString);

	// send it to both the VGUI and the HUD
	CComms::PrintMsg(TargetWindow, PrintMsg, false);
	g_pHudPlayerMsgs->PrintMsg(PrintMsg, CHATFLAG_SERVER);
}

//=========================================================
//=========================================================
void CComms::PrintMsg(const WindowID_t &TargetWindow, CScrollerMsg PrintMsg, bool bLocalPlayer)
{
	CMessageWindow *pTargetWindow = GetWindow(TargetWindow);
	pTargetWindow->PrintMsg(PrintMsg, bLocalPlayer);
}

//=========================================================
//=========================================================
void CComms::SendMsg(void)
{
	int iLength;
	wchar_t wszUnicodeMsg[128];
	char szANSIMsg[128];

	const char *pszText = szANSIMsg;

	// convert it to ANSI
	m_pMessageEntry->GetText(wszUnicodeMsg, sizeof(wszUnicodeMsg));
	vgui::localize()->ConvertUnicodeToANSI(wszUnicodeMsg, szANSIMsg, sizeof(szANSIMsg));

	// caculate length
	iLength = Q_strlen(szANSIMsg);

	if(iLength <= 0)
		return;

	// put together the message
	int iType = 0;

	WindowID_t ActiveWindowID = GetActiveWindow()->GetWindowID();

	if(ActiveWindowID.m_bPrivateMsg)
	{
		iType |= CHATFLAG_PRIVATE;
	}
	else
	{
		switch(ActiveWindowID.m_iID)
		{
		case CHATMODE_GLOBAL:
			iType |= CHATFLAG_GLOBAL;
			break;
		case CHATMODE_TEAM:
			iType |= CHATFLAG_TEAM;
			break;
		case CHATMODE_SQUAD:
			iType |= CHATFLAG_SQUAD;
			break;
		}
	}

	// work out if we're using thirdperson chat
	if(IsThirdPersonMsg(szANSIMsg, iLength))
	{
		iType |= CHATFLAG_THIRDPERSON;
		pszText += THIRDPERSON_PREFIX_LENGTH;
	}

	// send the message
	SendMsg(pszText, iType);

	// clear the message entry
	m_pMessageEntry->ClearEntry();
}

//=========================================================
//=========================================================
void CComms::SendMsg(const char *pszText, int iType)
{
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	//WindowID_t ActiveWindowID = GetActiveWindow()->GetWindowID();

	if(!pLocalPlayer)
		return;

	int iPlayerID = pLocalPlayer->entindex();
	
	// (don't) print it locally
	//PrintMsg(ActiveWindowID, iPlayerID, iType & CHATFLAG_THIRDPERSON, pszText);
	//g_pHudPlayerMsgs->PrintMsg(iPlayerID, iType, pszText);

	// send message
	char szSendText[MAX_CHATMSG_LENGTH*2];
	
	Q_snprintf(szSendText, sizeof(szSendText), "say2 %i ", iType);

	if(iType & CHATFLAG_PRIVATE)
	{
		char *pszSendText = szSendText;
		int iBufferShift = Q_strlen(szSendText);

		pszSendText += iBufferShift;

		Q_snprintf(pszSendText, sizeof(szSendText) - iBufferShift, "%i ", iPlayerID);
	}

	Q_strncat(szSendText, pszText, sizeof(szSendText), COPY_ALL_CHARACTERS);

	// send to the server
	engine->ServerCmd(szSendText);
}

//=========================================================
//=========================================================
void CComms::SetActiveWindow(const WindowID_t &WindowID, int iWindowID)
{
	// don't bother if its invalid
	if(iWindowID == -1)
		return;

	// move it to the front (move in active order if already present)
	int iActiveOrderID = m_ActiveOrder.Find(WindowID);

	if(iActiveOrderID != 0)
	{
		if(!m_ActiveOrder.InvalidIndex())
			m_ActiveOrder.Remove(iActiveOrderID);

		m_ActiveOrder.AddToHead(WindowID);
	}

	/*
#ifdef _DEBUG
	char szBuffer[256];
	WindowID_t GlobalMode = { false, CHATMODE_GLOBAL };

	PrintAnnouncement(GlobalMode, "-------------------");

	for(int i = 0; i < m_ActiveOrder.Count(); i++)
	{
		Q_snprintf(szBuffer, sizeof(szBuffer), "m_ActiveOrder[%i] :", i);
		PrintAnnouncement(GlobalMode, szBuffer);

		Q_snprintf(szBuffer, sizeof(szBuffer), "          - m_bPrivateMsg: %i", m_ActiveOrder[i].m_bPrivateMsg ? 1 : 0);
		PrintAnnouncement(GlobalMode, szBuffer);

		Q_snprintf(szBuffer, sizeof(szBuffer), "          - m_iID: %i", m_ActiveOrder[i].m_iID);
		PrintAnnouncement(GlobalMode, szBuffer);

		PrintAnnouncement(GlobalMode, "");
	}

	PrintAnnouncement(GlobalMode, "-------------------");

	PrintAnnouncement(GlobalMode, "");
#endif#
	*/

	// refresh visability
	m_iActiveWindow = iWindowID;

	for(int i = 0; i < m_MsgWindows.Count(); i++)
		m_MsgWindows[i]->SetVisible(false);

	GetActiveWindow()->SetVisible(true);

	m_pWindowSelection->SetActiveWindow(WindowID);
	m_pPlayerSelection->SetActiveWindow(WindowID);

	// enable input
	SetEntryEnabled(true);
}

//=========================================================
//=========================================================
void CComms::SetActiveWindow(const WindowID_t &WindowID)
{
	// find the window
	SetActiveWindow(WindowID, GetWindowID(WindowID));
}

//=========================================================
//=========================================================
void CComms::SetActiveWindow(bool bPrivateMsg, int iID)
{
	WindowID_t WindowID = { bPrivateMsg, iID };
	SetActiveWindow(WindowID);
}

//=========================================================
//=========================================================
CMessageWindow *CComms::GetActiveWindow(void)
{
	return m_MsgWindows[m_iActiveWindow];
}

//=========================================================
//=========================================================
int CComms::GetWindowID(const WindowID_t &WindowID)
{
	// find the window
	int iFoundID = FindWindow(WindowID);

	// create one if it doesn't exist (and player isn't blocked)
	if(iFoundID == -1 && !m_bBlocked[WindowID.m_iID])
		iFoundID = CreateWindow(WindowID);

	return iFoundID;
}

//=========================================================
//=========================================================
CMessageWindow *CComms::GetWindow(const WindowID_t &WindowID)
{
	int iFoundID = GetWindowID(WindowID);

	if(iFoundID == -1)
		return NULL;

	return m_MsgWindows[iFoundID];
}

//=========================================================
//=========================================================
void CComms::OnPageShow()
{
	m_pMessageEntry->RequestFocus();
	m_pMessageEntry->Repaint();
}

//=========================================================
//=========================================================
void CComms::OnPageHide()
{
	//SetActiveMode(CHAT_GLOBAL);
}

//=========================================================
//=========================================================
void CComms::OnResetData()
{
	//m_pMessages->ResetAllData();
}

//=========================================================
//=========================================================
int CComms::CreateWindow(const WindowID_t &WindowID)
{
	const char *pszName = NULL;

	if(WindowID.m_bPrivateMsg)
		pszName = g_PR->GetPlayerName(WindowID.m_iID);
	else
		pszName = GetChatModeName(WindowID.m_iID);		
		
	CMessageWindow *pMessageWindow = new CMessageWindow(this, "MessageWindow", WindowID);
	int iWindowID = m_MsgWindows.AddToTail(pMessageWindow);

	m_pWindowSelection->AddWindow(pszName, WindowID);

	if(!WindowID.m_bPrivateMsg)
		m_ActiveOrder.AddToTail(WindowID);

	return iWindowID;
}

//=========================================================
//=========================================================
int CComms::CreateWindow(bool bPrivateMsg, int iID)
{
	WindowID_t WindowID = { bPrivateMsg, iID };
	return CreateWindow(WindowID);
}

//=========================================================
//=========================================================
void CComms::DestroyWindow(const WindowID_t &WindowID)
{
	int iFoundID = FindWindow(WindowID);
	Assert(iFoundID != -1);

	// make a copy of the WindowID
	WindowID_t WindowIDCopy = WindowID;

	// delete the window
	delete m_MsgWindows[iFoundID];
	m_MsgWindows.Remove(iFoundID);

	// remove the selection of the window
	m_pWindowSelection->RemoveWindow(WindowIDCopy);

	// remove from show order
	if(m_iActiveWindow == iFoundID)
	{
		// ensure if its the active window, set the next one
		m_ActiveOrder.Remove(0);

		WindowID_t &NextActiveWindow = m_ActiveOrder[0];
		SetActiveWindow(NextActiveWindow);
	}
	else
	{
		m_ActiveOrder.FindAndRemove(WindowIDCopy);
	}
}

//=========================================================
//=========================================================
int CComms::FindWindow(const WindowID_t &WindowID)
{
	int iFoundID = -1;

	for(int i = 0; i < m_MsgWindows.Count(); i++)
	{
		CMessageWindow *pMessageWindow = m_MsgWindows[i];

		if(pMessageWindow->GetWindowID() == WindowID)
		{
			iFoundID = i;
			break;
		}
	}

	// when its not a private window, it should always exist
	if(iFoundID == -1)
		Assert(WindowID.m_bPrivateMsg);

	return iFoundID;
}

//=========================================================
//=========================================================
void CComms::BlockPlayer(int iID, bool bState)
{
	// block the player
	m_bBlocked[iID] = bState;

	// make sure the player can't hear him either
	GetClientVoiceMgr()->SetPlayerBlockedState(iID, bState);

	m_pWindowSelection->BlockPlayer(iID, bState);
	m_pPlayerSelection->BlockPlayer(iID, bState);

	// ensure he can't/can type messages to blocked/unblocked players
	SetEntryEnabled(!bState);
}

//=========================================================
//=========================================================
bool CComms::IsPlayerBlocked(int iID) const
{
	return m_bBlocked[iID];
}

//=========================================================
//=========================================================
const char *CComms::GetChatModeName(int iMode)
{
	Assert(iMode >= CHATMODE_GLOBAL && iMode <= CHATMODE_SQUAD);

	switch(iMode)
	{
	case CHATMODE_GLOBAL:
		return "> Global";
	case CHATMODE_TEAM:
		return "> Team";
	case CHATMODE_SQUAD:
		return "> Squad";
	default:
		return "";
	}
}

//=========================================================
//=========================================================
void CComms::SetEntryEnabled(bool bState)
{
	m_pMessageEntry->SetEnabled(bState);

	if(!bState)
		m_pMessageEntry->SetText("");
}

//=========================================================
//=========================================================
void CComms::FireGameEvent(IGameEvent *pEvent)
{
	const char *pszName = pEvent->GetName();

	if(Q_strcmp(pszName, "player_disconnect") == 0)
	{
		// TODO: alert the private msg system thats hes gone (write in any open windows, and change blocked etc)
	}
	else if(Q_strcmp(pszName, "player_changename") == 0)
	{
		// TODO: update the selection window and alert any open windows
	}
}
