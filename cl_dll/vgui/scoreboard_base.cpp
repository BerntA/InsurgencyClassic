//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "scoreboard_base.h"

#include "ins_player_shared.h"

#include <vgui/ilocalize.h>
#include <vgui/ivgui.h>

#include <keyvalues.h>
#include <vgui_controls/label.h>
#include <vgui_controls/sectionedlistpanel.h>
#include <cl_dll/iviewport.h>

#include "basic_colors.h"

#include "play_team_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define INVALID_SELECT -1

//=========================================================
//=========================================================
CScoreboardBase::CScoreboardBase(vgui::Panel *pParent, vgui::Panel *pViewportPanel)
	: EditablePanel(pParent, PANEL_SCOREBOARD)
{
	SetScheme("ClientScheme");

	// set another parent
	m_pViewportPanel = pViewportPanel;

	// add a heart beat
	ivgui()->AddTickSignal(GetVPanel(), 1000);

	// setup the player list
	m_iPlayerIndexSymbol = KeyValuesSystem()->GetSymbolForString("playerIndex");

	m_pPlayerList = new SectionedListPanel(this, "PlayerList");
	m_pPlayerList->SetVerticalScrollbar(true);

	// load resources
	LoadControlSettings("Resource/UI/Frames/Panels/Controls/ScoreboardBase.res");
}

//=========================================================
//=========================================================
void CScoreboardBase::OnSizeChanged(int iWide, int iTall)
{
	BaseClass::OnSizeChanged(iWide, iTall);

	// set the size and position of the player list
	m_pPlayerList->SetPos(0, 0);
	m_pPlayerList->SetSize(iWide, iTall);

	int iSelectedItem = m_pPlayerList->GetSelectedItem();

	if(iSelectedItem != -1)
		m_pPlayerList->ScrollToItem(iSelectedItem);
}

//=========================================================
//=========================================================
void CScoreboardBase::OnVisibilityChange(int iOldVisible)
{
	BaseClass::OnVisibilityChange(iOldVisible);

	if(iOldVisible)
		return;

	Update();
}

//=========================================================
//=========================================================
void CScoreboardBase::OnTick(void)
{
	if(IsVisible() && m_pViewportPanel->IsVisible())
		Update();
}

//=========================================================
//=========================================================
void CScoreboardBase::OnResetData(void)
{
	Setup();
}

//=========================================================
//=========================================================
void CScoreboardBase::ItemLeftClick(int iItemID)
{
	m_iSelectedItemID = m_pPlayerList->GetSelectedItem();
}

//=========================================================
//=========================================================
void CScoreboardBase::Setup(void)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	// clear
	m_iSelectedItemID = INVALID_SELECT;

	m_pPlayerList->DeleteAllItems();
	m_pPlayerList->RemoveAllSections();

	// add all the sections
	InitScoreboardSections();

	// setup should only be called when data needs to be displayed so ...
	FillScoreBoard();
}

//=========================================================
//=========================================================
void CScoreboardBase::SetClientSelectable(bool bState)
{
	m_pPlayerList->SetClientSelectable(bState);
}

//=========================================================
//=========================================================
void CScoreboardBase::InitScoreboardSections(void)
{
	AddHeader();
}

//=========================================================
//=========================================================
void CScoreboardBase::AddHeader(void)
{
	m_pPlayerList->AddSection(SECTION_HEADER, "");
	m_pPlayerList->SetSectionAlwaysVisible(SECTION_HEADER, true);

	m_pPlayerList->SetSectionFgColor(SECTION_HEADER, COLOR_GREY, false);
}

//=========================================================
//=========================================================
void CScoreboardBase::Update(void)
{
	// NOTE: HACKHACK
	if(!GetParent() || !GetParent()->IsVisible() || g_Teams.Count() == 0)
		return;

	m_pPlayerList->DeleteAllItems();

	FillScoreBoard();
}

//=========================================================
//=========================================================
void CScoreboardBase::FillScoreBoard()
{
	// update totals information
	UpdateTeamInfo();

	// update player info
	UpdatePlayerInfo();

	// repaint
	Repaint();
}

//=========================================================
//=========================================================
void CScoreboardBase::UpdateTeamInfo(void)
{
	wchar_t szString[256];

	for(int i = 0; i < MAX_TEAMS; i++)
	{
		C_Team *pTeam = GetGlobalTeam(i);
		int iSectionID = GetSectionIDFromTeamID(i);

		if(i == TEAM_ONE || i == TEAM_TWO)
		{
			C_PlayTeam *pPlayTeam = (C_PlayTeam*)pTeam;

			swprintf(szString, L"%S (%i)", pTeam->Get_Name(), pPlayTeam->GetScore());
			m_pPlayerList->ModifyColumn(iSectionID, "name", szString);

			swprintf(szString, L"%i", pPlayTeam->Get_Ping(true));
			m_pPlayerList->ModifyColumn(iSectionID, "ping", szString);
		}
		else
		{
			localize()->ConvertANSIToUnicode(pTeam->Get_Name(), szString, sizeof(szString));
			m_pPlayerList->ModifyColumn(iSectionID, "name", szString);
		}

		m_pPlayerList->SetSectionFgColor(GetSectionIDFromTeamID(i), GetColorFromTeam(i));
	}
}


//=========================================================
//=========================================================
bool CScoreboardBase::StaticPlayerSortFunc(SectionedListPanel *pList, int iItemID1, int iItemID2)
{
	KeyValues *it1 = pList->GetItemData(iItemID1);
	KeyValues *it2 = pList->GetItemData(iItemID2);
	Assert(it1 && it2);

	// first compare frags
	int v1 = it1->GetInt("level");
	int v2 = it2->GetInt("level");
	if (v1 > v2)
		return true;
	else if (v1 < v2)
		return false;

	// next compare deaths
	v1 = it1->GetInt("deaths");
	v2 = it2->GetInt("deaths");
	if (v1 > v2)
		return false;
	else if (v1 < v2)
		return true;

	// the same, so compare itemID's (as a sentinel value to get deterministic sorts)
	return (iItemID1 < iItemID2);
}

//=========================================================
//=========================================================
int CScoreboardBase::FindItemIDForPlayerIndex(int playerIndex)
{
	for(int i = 0; i <= m_pPlayerList->GetHighestItemID(); i++)
	{
		if (m_pPlayerList->IsItemIDValid(i))
		{
			KeyValues *kv = m_pPlayerList->GetItemData(i);
			kv = kv->FindKey(m_iPlayerIndexSymbol);

			if (kv && kv->GetInt() == playerIndex)
				return i;
		}
	}

	return -1;
}

//=========================================================
//=========================================================
bool CScoreboardBase::CanShowFullTeamInfo(const C_Team *pTeam) const
{
	C_Team *pLocalTeam = GetLocalTeam();

	if(!pTeam || !pLocalTeam || !pTeam->IsPlayTeam())
		return false;

	return (pLocalTeam == pTeam || pLocalTeam->GetTeamID() == TEAM_SPECTATOR);
}