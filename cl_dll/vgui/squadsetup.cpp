//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "filesystem.h"
#include "hud_macros.h"
#include <vgui/ischeme.h>
#include <vgui/isurface.h>
#include <vgui/ipanel.h>
#include <vgui/ivgui.h>
#include <keyvalues.h>

#include <vgui_controls/imagepanel.h>
#include <vgui_controls/sectionedlistpanel.h>
#include <cl_dll/iviewport.h>

#include "squadsetup.h"
#include "squadchoice.h"

#include "ins_player_shared.h"
#include "play_team_shared.h"
#include "ins_gamerules.h"
#include "igameresources.h"
#include "c_team.h"
#include "ins_squad_shared.h"
#include "scoreboard.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
class CSquadSetupHeader : public INSTopHeader
{
	DECLARE_CLASS_SIMPLE(CSquadSetupHeader, INSTopHeader);

public:
	CSquadSetupHeader(INSFrame *pParent, const char *pszName)
		: INSTopHeader(pParent, pszName)
	{
		LoadControlSettings("Resource/UI/Frames/Headers/SquadSetup.res");
	}

private:
	virtual bool UseDefaultTall(void) const { return false; }
};

//=========================================================
//=========================================================
enum SquadMsgStates_t
{
	SQUADMSG_STATE_NONE = 0,
	SQUADMSG_STATE_PLAYERWAIT,
	SQUADMSG_STATE_PLAYERSELECT,
	SQUADMSG_STATE_CLIENTSELECT,
	SQUADMSG_STATE_TEAM,
	SQUADMSG_STATE_STARTGAME
};

class CSquadMsg : public vgui::INSTextHeader
{
	DECLARE_CLASS_SIMPLE(CSquadMsg, INSTextHeader);

private:
	CSquadSetup *m_pSquadSetup;

	int m_iState;
	void (CSquadMsg::*UpdateState)(bool bFirstUpdate);
	bool m_bNeedsUpdate;

	float m_flStartTime;

public:
	CSquadMsg(CSquadSetup *pParent)
		: INSTextHeader(pParent, NULL)
	{
		m_pSquadSetup = pParent;
		Reset();

		ivgui()->AddTickSignal(GetVPanel(), 500);

		SetState(SQUADMSG_STATE_NONE);
	}

	virtual void Reset(void)
	{
		BaseClass::Reset();

		m_flStartTime = 0.0f;
		m_bNeedsUpdate = false;
	}

	void SetState(int iState, float flStartTime = 0.0f)
	{
		m_iState = iState;

		Reset();

		m_flStartTime = flStartTime == 0.0f ? gpGlobals->curtime : flStartTime;

		switch(m_iState)
		{
		case SQUADMSG_STATE_NONE:
			UpdateState = NULL;
			break;
		case SQUADMSG_STATE_PLAYERWAIT:
			UpdateState = UpdatePlayerWait;
			break;
		case SQUADMSG_STATE_PLAYERSELECT:
			UpdateState = UpdatePlayerSelect;
			break;
		case SQUADMSG_STATE_CLIENTSELECT:
			UpdateState = UpdateClientSelect;
			break;
		case SQUADMSG_STATE_TEAM:
			UpdateState = UpdateTeam;
			break;
		case SQUADMSG_STATE_STARTGAME:
			UpdateState = UpdateStartGame;
			break;
		}

		CallUpdateState(true);
	}

protected:
	virtual void OnTick(void)
	{
		if(g_Teams.Count() == 0)
			return;

		if(!m_pSquadSetup->IsActive())
			return;

		CallUpdateState(false);
	}

private:
	void UpdatePlayerWait(bool bFirstUpdate)
	{
		int iTimeLeft = (INSRules()->GetSquadMode()->GetPlayerWaitTime() - (int)(gpGlobals->curtime - m_flStartTime));

		if(iTimeLeft < 0)
		{
			SetState(SQUADMSG_STATE_NONE);
			return;
		}

		char szPlayerWait[MAX_TEXTHEADER_LENGTH];
		Q_snprintf(szPlayerWait, sizeof(szPlayerWait), "Waiting for Players (0:%02i)", iTimeLeft);
		SetText(szPlayerWait);
	}

	void UpdatePlayerSelect(bool bFirstUpdate)
	{
		IGameResources *gr = GameResources();
		C_SquadMode *pSquadMode = INSRules()->GetSquadMode();

		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();
		int iPlayerTeam = pPlayer->GetTeamID();

		int iPlayerIndex = pSquadMode->GetPlayer(pSquadMode->GetCurrentPlayer(iPlayerTeam), iPlayerTeam);

		char szName[MAX_PLAYER_NAME_LENGTH];
		UTIL_MakeSafeName(gr->GetPlayerName(iPlayerIndex), szName);

		int iSelectionTimeOut = INSRules()->GetSquadMode()->GetSelectionTimeOut();

		int iTimeLeft;
		int iTimeAgo = (int)(gpGlobals->curtime - m_flStartTime);

		if(iTimeAgo > SQUAD_CHOICEDELAY)
			iTimeLeft = iSelectionTimeOut + SQUAD_CHOICEDELAY - iTimeAgo;
		else
			iTimeLeft = iSelectionTimeOut;

		char szPlayerSelect[MAX_TEXTHEADER_LENGTH];
		Q_snprintf(szPlayerSelect, sizeof(szPlayerSelect), "%s has 0:%02i to Choose their Squad", szName, iTimeLeft);
		SetText(szPlayerSelect);
	}

	void UpdateClientSelect(bool bFirstUpdate)
	{
		if(!bFirstUpdate && !m_bNeedsUpdate)
			return;

		SetText("You will now Choose your Squad");
	}

	void UpdateTeam(bool bFirstUpdate)
	{
		if(!bFirstUpdate)
			return;

		SetText("Waiting for Opposing Team");
	}

	void UpdateStartGame(bool bFirstUpdate)
	{
		int iTimeLeft = (SQUAD_ENDTIME - (int)(gpGlobals->curtime - m_flStartTime));

		char szStartGame[MAX_TEXTHEADER_LENGTH];
		Q_snprintf(szStartGame, sizeof(szStartGame), "Prepare for Combat in %i", iTimeLeft);
		SetText(szStartGame);
	}

	void CallUpdateState(bool bFirstUpdate)
	{
		C_INSRules *pINSRules = INSRules();

		if(UpdateState == NULL)
			return;

		if(pINSRules && !pINSRules->IsStatusSquad())
		{
			m_bNeedsUpdate = true;
			return;
		}

		(*this.*UpdateState)(bFirstUpdate);
	}
};

//=========================================================
//=========================================================
class CPlayerOrder : public Panel
{
	DECLARE_CLASS_SIMPLE(CPlayerOrder, Panel);

private:
	CSquadSetup *m_pSquadChoice;

	enum { ID_WIDTH = 20, NAME_WIDTH = 100 };
	#define SECTION_PLAYERS 0

	SectionedListPanel *m_pList;
	int m_iPlayerIndexSymbol;

	Color m_AssignedPlayerColor;

public:
	CPlayerOrder(Panel *pParent, CSquadSetup *pSquadOrder, const char *pszPanelName)
		: Panel(pParent, pszPanelName)
	{
		m_pSquadChoice = pSquadOrder;

		m_pList = new SectionedListPanel(this, NULL);
		m_pList->SetClientSelectable(false);

		ivgui()->AddTickSignal(GetVPanel(), 750);
		m_iPlayerIndexSymbol = KeyValuesSystem()->GetSymbolForString("playerIndex");

		SetPaintBackgroundEnabled(false);
	}

	void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		// assign the same size
		int iWide, iTall;
		GetSize(iWide, iTall);
	
		m_pList->SetSize(iWide, iTall);

		// set the color
		m_AssignedPlayerColor = GetSchemeColor("SquadPlayerOrder.AssignedTextColor", GetBgColor(), pScheme);
	}

	void Reset(void)
	{
		m_pList->DeleteAllItems();
		m_pList->RemoveAllSections();

		InitPlayerListSections();
	}

	void UpdateActivePlayer(void)
	{
		C_SquadMode *pSquadMode = INSRules()->GetSquadMode();
		int iCurrentPlayer = pSquadMode->GetCurrentPlayer(GetLocalTeam());

		Update();

		if(iCurrentPlayer != -1 && !m_pSquadChoice->m_bEnding)
		{
			int iLocalTeam = GetLocalTeam();
			Assert(IsPlayTeam(iLocalTeam));

			m_pList->SetSelectedItem(FindItemIDForPlayerIndex(pSquadMode->GetPlayer(iCurrentPlayer, iLocalTeam)));
		}
		else
		{
			m_pList->DeselectCurrentItem();
		}
	}

	virtual void OnTick(void)
	{
		Update();
	}

	void Update(void)
	{
		if(!IsValid())
			return;

		//m_pList->DeleteAllItems();

		C_SquadMode *pSquadMode = INSRules()->GetSquadMode();
		IGameResources *gr = GameResources();

		int iPlayerTeam = GetLocalTeam();

		if(!IsPlayTeam(iPlayerTeam))
			return;

		for(int i = 0; i < pSquadMode->GetPlayerCount(iPlayerTeam); i++)
		{
			int iPlayerIndex = pSquadMode->GetPlayer(i, iPlayerTeam);

			char szName[MAX_PLAYER_NAME_LENGTH];
			UTIL_MakeSafeName(gr->GetPlayerName(iPlayerIndex), szName);

			char szID[8];
			Q_snprintf(szID, sizeof(szID), "%i. ", i + 1);

			KeyValues *pPlayerData = new KeyValues("data");
			pPlayerData->SetString("id", szID);
			pPlayerData->SetString("name", szName);
			pPlayerData->SetInt("playerIndex", iPlayerIndex);

			int iItemID = FindItemIDForPlayerIndex(iPlayerIndex);

			if(iItemID == -1)
				iItemID = m_pList->AddItem(SECTION_PLAYERS, pPlayerData);
			else
				m_pList->ModifyItem(iItemID, SECTION_PLAYERS, pPlayerData);

			const Color &ItemIDColor = ((i <= pSquadMode->GetCurrentPlayer(iPlayerTeam)) ? m_AssignedPlayerColor : GetColorFromTeam(iPlayerTeam));
			m_pList->SetItemFgColor(iItemID, ItemIDColor);

			pPlayerData->deleteThis();
		}

		for(int i = 0; i <= m_pList->GetHighestItemID(); i++)
		{
			if(!m_pList->IsItemIDValid(i))
				continue;

			KeyValues *kv = m_pList->GetItemData(i);
			kv = kv->FindKey(m_iPlayerIndexSymbol);
			Assert(kv);

			if(!pSquadMode->IsPlayerListed(kv->GetInt(), iPlayerTeam))
				m_pList->RemoveItem(i);
		}
	}

private:
	void InitPlayerListSections()
	{
		m_pList->AddSection(SECTION_PLAYERS, "");
		m_pList->SetSectionAlwaysVisible(SECTION_PLAYERS, true);
		m_pList->SetSectionHeaderVisible(SECTION_PLAYERS, false);

		m_pList->AddColumnToSection(SECTION_PLAYERS, "id", "", 0, scheme()->GetProportionalScaledValue(ID_WIDTH));
		m_pList->AddColumnToSection(SECTION_PLAYERS, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH));
	}

	bool IsValid(void)
	{
		C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		return (m_pSquadChoice->IsActive() && INSRules() && INSRules()->IsStatusSquad() && pLocalPlayer);
	}

	int GetLocalTeam(void)
	{
		if(!IsValid())
			return -1;

		C_INSPlayer *pLocalPlayer = C_INSPlayer::GetLocalPlayer();
		return pLocalPlayer->GetTeamID();
	}

	int FindItemIDForPlayerIndex(int iPlayerIndex)
	{
		for(int i = 0; i <= m_pList->GetHighestItemID(); i++)
		{
			if (m_pList->IsItemIDValid(i))
			{
				KeyValues *kv = m_pList->GetItemData(i);
				kv = kv->FindKey(m_iPlayerIndexSymbol);

				if (kv && kv->GetInt() == iPlayerIndex)
					return i;
			}
		}

		return -1;
	}
};

//=========================================================
//=========================================================
void __MsgFunc_SquadMsg(bf_read &msg)
{
	if(g_pSquadSetup)
		g_pSquadSetup->HandleMsg(msg);
}

//=========================================================
//=========================================================
CSquadSetup *g_pSquadSetup = NULL;

CSquadSetup::CSquadSetup(IViewPort *pViewPort)
	: INSFrame(NULL, PANEL_SQUADSETUP)
{
	m_pViewPort = pViewPort;

	// setup global pointer
	g_pSquadSetup = this;

	// set the scheme
	SetScheme("ClientScheme");

	// setup headers
	m_pSquadMsg = new CSquadMsg(this);
	AddHeader(m_pSquadMsg);

	AddHeader(new CSquadSetupHeader(this, "SquadSetupHeader"));

	// setup panels
	m_pSelectionOrder = new vgui::ImagePanel(this, "SelectionOrder");
	m_pPlayerOrder = new ::CPlayerOrder(this, this, "PlayerOrder");
	m_pScoreboard = new CScoreboard(this, this, NULL);

	// setup the panel
	HOOK_MESSAGE(SquadMsg);

	// reset
	Reset();

	// load the control settings
	LoadControlSettings("Resource/UI/Frames/SquadSetup.res");

	// now set the correct parent
	m_pPlayerOrder->SetParent(GetClientArea());
	m_pSelectionOrder->SetParent(GetClientArea());
	m_pScoreboard->SetParent(GetClientArea());
}

//=========================================================
//=========================================================
void CSquadSetup::Reset( void )
{
	m_pPlayerOrder->Reset();

	m_bActive = false;
	//m_iCurrentPlayer = -1;
	m_flShowSquadTime = 0.0f;
	m_flSquadCloseTime = 0.0f;
	m_bEnding = false;
	m_flEndTime = 0.0f;
}

//=========================================================
//=========================================================
void CSquadSetup::ShowPanel( bool bShow )
{
	if(IsVisible() == bShow)
		return;

	Assert(g_pGameRules);

	if(bShow)
	{
		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}
}

//=========================================================
//=========================================================
void CSquadSetup::HandleMsg(bf_read &msg)
{
	int iType = msg.ReadByte();
	float flStartTime = 0.0f;

	if(iType == SQUADMSG_SHOW_WAITING)
		flStartTime = msg.ReadFloat();

	switch(iType)
	{
	case SQUADMSG_SHOW:
		//StartSelection();
		break;
	case SQUADMSG_SHOW_WAITING:
		m_pSquadMsg->SetState(SQUADMSG_STATE_PLAYERWAIT, flStartTime);
		break;
	case SQUADMSG_START:
		//StartSelection();
		break;
	case SQUADMSG_START_CHOOSE:
		m_flShowSquadTime = gpGlobals->curtime + SQUAD_CHOICEDELAY;
		//m_pSquadMsg->SetState(SQUADMSG_STATE_CLIENTSELECT);
		break;
	case SQUADMSG_STOP_CHOOSE:
		ShowChangeSquadPanel(false);
		break;
	case SQUADMSG_ENDING:
		m_bEnding = true;
		m_flEndTime = gpGlobals->curtime + SQUAD_ENDTIME;
		m_pPlayerOrder->UpdateActivePlayer();
		m_pSquadMsg->SetState(SQUADMSG_STATE_STARTGAME);
		break;
	case SQUADMSG_STOP_ENDING:
		m_bEnding = false;
		m_pSquadMsg->SetState(SQUADMSG_STATE_TEAM);
		break;
	}

	if(iType == SQUADMSG_SHOW || iType == SQUADMSG_SHOW_WAITING)
	{
		m_bActive = true;
		m_pViewPort->ShowPanel(this, true);
	}
}

//=========================================================
//=========================================================
void CSquadSetup::CurrentPlayerUpdate(void)
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();
	Assert(pPlayer);

	int iPlayerTeam = pPlayer->GetTeamID();
	C_SquadMode *pSquadMode = INSRules()->GetSquadMode();

	int iCurrentPlayerIndex = pSquadMode->GetPlayer(pSquadMode->GetCurrentPlayer(iPlayerTeam), iPlayerTeam);

	if(iCurrentPlayerIndex != pPlayer->entindex())
		m_pSquadMsg->SetState(SQUADMSG_STATE_PLAYERSELECT);
	else
		m_pSquadMsg->SetState(SQUADMSG_STATE_CLIENTSELECT);

	// if this is not the last player
	/*(
	if((pSquadMode->GetCurrentPlayer(iPlayerTeam) + 1) <= (pSquadMode->GetPlayerCount(iPlayerTeam) - 1))
	{
		m_iCurrentPlayer++;
		ShowPlayerSelect();
	}
	else
	{
		m_iCurrentPlayer = -1;
	}

	*/	
	m_pPlayerOrder->UpdateActivePlayer();
}

//=========================================================
//=========================================================
void CSquadSetup::PerformLayout(void)
{
	BaseClass::PerformLayout();

	m_pPlayerOrder->Update();
	m_pScoreboard->Setup();
}

//=========================================================
//=========================================================
void CSquadSetup::OnThink(void)
{
	if(!IsActive())
		return;

	if(m_flShowSquadTime != 0.0f && m_flShowSquadTime <= gpGlobals->curtime)
	{
		m_flShowSquadTime = 0.0f;
		m_flSquadCloseTime = gpGlobals->curtime + INSRules()->GetSquadMode()->GetSelectionTimeOut();

		m_pSquadMsg->SetState(SQUADMSG_STATE_NONE);
		ShowChangeSquadPanel(true);
		return;
	}

	if(m_bEnding && m_flEndTime <= gpGlobals->curtime)
	{
		m_bActive = false;
		m_pViewPort->ShowPanel(this, false);
		Reset();
	}
}

//=========================================================
//=========================================================
void CSquadSetup::StartSelection(void)
{
	//m_iCurrentPlayer = 0;
	//ShowPlayerSelect();

	//m_pPlayerOrder->UpdateActivePlayer();
}

//=========================================================
//=========================================================
void CSquadSetup::ShowPlayerSelect(void)
{
	//C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	//Assert(pPlayer);


}

//=========================================================
//=========================================================
void CSquadSetup::ShowChangeSquadPanel(bool bState)
{
	// NVGUI: since the ChangeSquad panel and Squad Choice Routine thing
	// are in the same panel, none of this shit is needed
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName(PANEL_CHANGESQUAD);
	m_pViewPort->ShowPanel(pPanel, bState);
}


/*
	int iPlayerTeam = pPlayer->GetTeamID();

	C_SquadMode *pSquadMode = INSRules()->GetSquadMode();

	if((m_iCurrentPlayer + 1) <= (pSquadMode->GetPlayerCount(iPlayerTeam) - 1))
		{
			m_iCurrentPlayer++;
			ShowPlayerSelect();
		}
		else
		{
			m_iCurrentPlayer = -1;
		}
*/