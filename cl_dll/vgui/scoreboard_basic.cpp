//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "scoreboard_basic.h"

#include <vgui/ilocalize.h>
#include <vgui/ivgui.h>
#include <keyvalues.h>

#include <vgui_controls/label.h>
#include <vgui_controls/sectionedlistpanel.h>
#include <cl_dll/iviewport.h>

#include "igameresources.h"
#include "ins_gamerules.h"
#include "play_team_shared.h"
#include "ins_player_shared.h"

#include "c_playerresource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
enum ScoreBoardSections_t
{
	SECTION_TEAMONE = SECTION_HEADER + 1,
	SECTION_TEAMTWO,
	SECTION_SPECTATORS,
	SECTION_UNASSIGNED,
	MAX_SECTIONS
};

//=========================================================
//=========================================================
CScoreboardBasic::CScoreboardBasic(Panel *pParent, Panel *pViewpanelParent)
	: CScoreboardBase(pParent, pViewpanelParent)
{
	SetClientSelectable(false);
}

//=========================================================
//=========================================================
void CScoreboardBasic::InitScoreboardSections(void)
{
	BaseClass::InitScoreboardSections();

	AddSection(SECTION_TEAMONE);
	AddSection(SECTION_TEAMTWO);
	AddSection(SECTION_SPECTATORS);
	AddSection(SECTION_UNASSIGNED);
}

//=========================================================
//=========================================================
void CScoreboardBasic::AddHeader(void)
{
	BaseClass::AddHeader();

	m_pPlayerList->AddColumnToSection(SECTION_HEADER, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH + SCORE_WIDTH));
	m_pPlayerList->AddColumnToSection(SECTION_HEADER, "ping", "#PlayerPing", 0, scheme()->GetProportionalScaledValue(PING_WIDTH));
}

//=========================================================
//=========================================================
void CScoreboardBasic::AddSection(int iSectionID)
{
	switch(iSectionID)
	{
		case SECTION_TEAMONE:
		case SECTION_TEAMTWO:
		{
			m_pPlayerList->AddSection(iSectionID, "", NULL, StaticPlayerSortFunc);
			m_pPlayerList->SetSectionAlwaysVisible(iSectionID, true);

			m_pPlayerList->AddColumnToSection(iSectionID, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH));
			m_pPlayerList->AddColumnToSection(iSectionID, "level", "", 0, scheme()->GetProportionalScaledValue(SCORE_WIDTH));
			m_pPlayerList->AddColumnToSection(iSectionID, "ping", "", 0, scheme()->GetProportionalScaledValue(PING_WIDTH));

			break;
		}
		case SECTION_SPECTATORS:
		case SECTION_UNASSIGNED:
		{
			m_pPlayerList->AddSection(iSectionID, "");
			m_pPlayerList->AddColumnToSection(iSectionID, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH));
			break;
		}
	}
}

//=========================================================
//=========================================================
void CScoreboardBasic::UpdatePlayerInfo()
{
	C_Team *pTeam = NULL;
	
	if(!g_PR)
		return;

	for(int i = 0; i < MAX_TEAMS; i++)
	{
		pTeam = GetGlobalTeam(i);

		for(int j = 0; j < pTeam->Get_Number_Players(); j++)
		{
			int iPlayerIndex = pTeam->GetPlayerID(j);

			if(g_PR->IsConnected(iPlayerIndex))
			{
				// make the players name "safe"
				char szName[MAX_PLAYER_NAME_LENGTH*2];

				if(!C_INSPlayer::GetSafePlayerName(iPlayerIndex, false, szName, sizeof(szName)))
					continue;

				KeyValues *pPlayerData = new KeyValues("data");
				pPlayerData->SetInt("playerIndex", iPlayerIndex);
				pPlayerData->SetString("name", szName);
				pPlayerData->SetInt("ping", g_PR->GetPing(iPlayerIndex));

				if(CanShowFullTeamInfo(pTeam))
					pPlayerData->SetInt("level", g_PR->GetLevel(iPlayerIndex));

				int iSectionID = GetSectionIDFromTeamID(g_PR->GetTeamID(iPlayerIndex));
				int iItemID = FindItemIDForPlayerIndex(iPlayerIndex);

				if (iItemID == -1)
				{
					// add a new row
					iItemID = m_pPlayerList->AddItem(iSectionID, pPlayerData);
				}
				else
				{
					// modify the current row
					m_pPlayerList->ModifyItem(iItemID, iSectionID, pPlayerData);
				}

				m_pPlayerList->SetItemFgColor(iItemID, GetColorFromTeam(pTeam));

				pPlayerData->deleteThis();
			}
			else
			{
				int iItemID = FindItemIDForPlayerIndex(i);

				if (iItemID != -1)
				{
					m_pPlayerList->RemoveItem(iItemID);
				}
			}
		}
	}
}

//=========================================================
//=========================================================
int CScoreboardBasic::GetSectionIDFromTeamID(int iTeamID)
{
	switch(iTeamID)
	{
	case TEAM_UNASSIGNED:
		return SECTION_UNASSIGNED;
	case TEAM_ONE:
		return SECTION_TEAMONE;
	case TEAM_TWO:
		return SECTION_TEAMTWO;
	case TEAM_SPECTATOR:
		return SECTION_SPECTATORS;
	}

	Assert(false);
	return 0;
}
