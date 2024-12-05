//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "scoreboard.h"
#include "scoreboard_base.h"

#include "ins_player_shared.h"

#include <vgui/ilocalize.h>
#include <vgui/ivgui.h>
#include <keyvalues.h>

#include <vgui_controls/label.h>
#include <vgui_controls/sectionedlistpanel.h>
#include <vgui_controls/richtext.h>
#include <cl_dll/iviewport.h>

#include "igameresources.h"
#include "ins_gamerules.h"
#include "play_team_shared.h"
#include "ins_squad_shared.h"
//#include "playerstats.h"
#include "c_playerresource.h"
#include "team_lookup.h"

#include "basic_colors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
bool StaticSquadSortFunc(SectionedListPanel *pList, int iSectionID1, int iSectionID2)
{
	KeyValues *pSectionData1 = pList->GetSectionData(iSectionID1);
	KeyValues *pSectionData2 = pList->GetSectionData(iSectionID2);

	if(!pSectionData1 || !pSectionData2)
		return false;

	int iSectionID1Score = pSectionData1->GetInt("score");
	int iSectionID2Score = pSectionData2->GetInt("score");
	
	if (iSectionID1Score > iSectionID2Score)
		return true;
	else if (iSectionID1Score < iSectionID2Score)
		return false;

	// the same, so compare itemID's (as a sentinel value to get deterministic sorts)
	return (iSectionID1 < iSectionID2);
}

//=========================================================
//=========================================================
Color g_DeveloperColor(230, 230, 0, 128);

//=========================================================
//=========================================================
enum ScoreBoardSections_t
{
	SECTION_TEAMONE = SECTION_HEADER + 1,

		SECTION_TEAMONE_SQUAD1,
		SECTION_TEAMONE_SQUAD2,
		//SECTION_TEAMONE_SQUAD3,
		//SECTION_TEAMONE_SQUAD4,

		SECTION_TEAMONE_UNASSIGNED,

	SECTION_TEAMTWO,

		SECTION_TEAMTWO_SQUAD1,
		SECTION_TEAMTWO_SQUAD2,
		//SECTION_TEAMTWO_SQUAD3,
		//SECTION_TEAMTWO_SQUAD4,

		SECTION_TEAMTWO_UNASSIGNED,

	SECTION_SPECTATORS,
	SECTION_UNASSIGNED,
	MAX_SECTIONS
};

CScoreboard::CScoreboard(Panel *pParent, Panel *pViewportPanel, CScoreboardManager *pScoreboardManager)
	: CScoreboardBase(pParent, pViewportPanel)
{
	m_pScoreboardManager = pScoreboardManager;

	if(!m_pScoreboardManager)
		SetClientSelectable(false);
}

void CScoreboard::PlayerSelected(int iPlayerID)
{
	if(iPlayerID == 0)
		SetSize(m_iWide, m_iTall);
	else if(iPlayerID != 0)
		SetSize(m_iWide, m_iTallSmall);
}

void CScoreboard::DeselectPlayer(void)
{
	m_pPlayerList->DeselectCurrentItem();
	PlayerSelected(0);
	m_iSelectedItemID = -1;
}

void CScoreboard::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	GetSize(m_iWide, m_iTall);

	m_iTallSmall = scheme()->GetProportionalScaledValue(inResourceData->GetInt("tall_small", 0));
}

void CScoreboard::ItemLeftClick(int iItemID)
{
	BaseClass::ItemLeftClick(iItemID);

	if(m_pScoreboardManager)
	{
		int iSelectedID = m_pPlayerList->GetSelectedItem();
		int iPlayerIndex = 0;

		if(iSelectedID != -1)
		{
			KeyValues *pPlayerData = m_pPlayerList->GetItemData(iSelectedID);
			iPlayerIndex = pPlayerData->GetInt("playerIndex");
		}
		else
		{
			iPlayerIndex = 0;
		}

		m_pScoreboardManager->PlayerSelected(iPlayerIndex);
	}
}

void CScoreboard::InitScoreboardSections(void)
{
	BaseClass::InitScoreboardSections();

	for(int i = SECTION_TEAMONE; i < MAX_SECTIONS; i++)
		AddSection(i);
}

void CScoreboard::AddHeader(void)
{
	BaseClass::AddHeader();

	m_pPlayerList->AddColumnToSection(SECTION_HEADER, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH));

	if(m_pScoreboardManager)
	{
		m_pPlayerList->AddColumnToSection(SECTION_HEADER, "dead", "", 0, scheme()->GetProportionalScaledValue(DEAD_WIDTH));
		m_pPlayerList->AddColumnToSection(SECTION_HEADER, "class", "", 0, scheme()->GetProportionalScaledValue(CLASS_WIDTH));
	}

	m_pPlayerList->AddColumnToSection(SECTION_HEADER, "level", "Level", 0, scheme()->GetProportionalScaledValue(SCORE_WIDTH));
	m_pPlayerList->AddColumnToSection(SECTION_HEADER, "ping", "#PlayerPing", 0, scheme()->GetProportionalScaledValue(PING_WIDTH));
}

void CScoreboard::FillScoreBoard(void)
{
	UpdateSquadInfo();

	BaseClass::FillScoreBoard();
}

void CScoreboard::UpdatePlayerInfo(void)
{
	C_Team *pTeam = NULL;

	if(!g_PR)
		return;

	bool bIsLocalPlayerDeveloper = (C_INSPlayer::GetLocalPlayer()->IsDeveloper() != DEVMODE_OFF);

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

				if(!C_INSPlayer::GetSafePlayerName(iPlayerIndex, CanShowFullTeamInfo(pTeam), szName, sizeof(szName)))
					continue;

				if(bIsLocalPlayerDeveloper)
				{
					char szUserID[MAX_PLAYER_NAME_LENGTH];
					Q_snprintf(szUserID, MAX_PLAYER_NAME_LENGTH, " (%i)", iPlayerIndex);
					Q_strcat(szName, szUserID);
				}

				int iSectionID = DISABLED_SQUAD;

				KeyValues *pPlayerData = new KeyValues("data");
				pPlayerData->SetInt("playerIndex", iPlayerIndex);
				pPlayerData->SetString("name", szName);
				pPlayerData->SetString("class", szName);
				pPlayerData->SetInt("ping", g_PR->GetPing(iPlayerIndex));

				bool bWrittenClass = false;

				if(IsPlayTeam(i))
				{
					C_PlayTeam *pPlayTeam = (C_PlayTeam*)pTeam;

					SquadInfo_t SquadInfo;
					pPlayTeam->GetSquadInfo(iPlayerIndex, SquadInfo);

					iSectionID = GetSectionIDFromSquadID(g_PR->GetTeamID(iPlayerIndex), SquadInfo.GetSquadID());

					pPlayerData->SetInt("level", g_PR->GetLevel(iPlayerIndex));

					if(CanShowFullTeamInfo(pTeam) && m_pScoreboardManager)
					{
						pPlayerData->SetString("dead", g_PR->IsAlive(iPlayerIndex) ? "" : "Dead");
						
						C_INSSquad *pSquad = pPlayTeam->GetSquad(i);

						// NOTE: this shouldn't be nesscary
						if(pSquad)
						{
							CPlayerClass *pClass = pSquad->GetClass(SquadInfo.GetSlotID());

							if(pClass)
							{
								pPlayerData->SetString("class", pClass->GetName());
								bWrittenClass = true;
							}
						}
					}
				}
				else
				{
					iSectionID = GetSectionIDFromSquadID(g_PR->GetTeamID(iPlayerIndex), DISABLED_SQUAD);
				}

				if(!bWrittenClass)
					pPlayerData->SetString("class", "");

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

				//Color ItemColor = ((iSquadID == DISABLED_SQUAD) ? COLOR_GREY : GetColorFromTeam(pTeam));
				Color ItemColor = GetColorFromTeam(pTeam);
				m_pPlayerList->SetItemFgColor(iItemID, ItemColor);

				if(g_PR->GetDeveloper(iPlayerIndex) == DEVMODE_ON)
					m_pPlayerList->SetItemBgColor(iItemID, g_DeveloperColor);
				else
					m_pPlayerList->SetItemBgColor(iItemID);


				if(m_iSelectedItemID == iItemID)
					m_pPlayerList->SetSelectedItem(iItemID);

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

int CScoreboard::GetSectionIDFromTeamID(int iTeamID)
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

void CScoreboard::AddSection(int iSectionID)
{
	switch(iSectionID)
	{
	case SECTION_TEAMONE:
	case SECTION_TEAMTWO:
		{
			m_pPlayerList->AddSection(iSectionID, "", "DefaultExtraBig", StaticPlayerSortFunc);
			m_pPlayerList->SetSectionAlwaysVisible(iSectionID, true);
			m_pPlayerList->SetSectionHeaderOffset(iSectionID, scheme()->GetProportionalScaledValue(TEAM_HEADER_OFFSET));
			m_pPlayerList->SetSectionItemOffset(iSectionID, scheme()->GetProportionalScaledValue(TEAM_ITEM_OFFSET));

			int iNameLength = scheme()->GetProportionalScaledValue(NAME_WIDTH);

			if(m_pScoreboardManager)
				iNameLength -= scheme()->GetProportionalScaledValue(TEAM_ITEM_OFFSET);

			m_pPlayerList->AddColumnToSection(iSectionID, "name", "", 0, iNameLength);

			if(m_pScoreboardManager)
			{
				m_pPlayerList->AddColumnToSection(iSectionID, "dead", "", 0, scheme()->GetProportionalScaledValue(DEAD_WIDTH));
				m_pPlayerList->AddColumnToSection(iSectionID, "class", "", 0, scheme()->GetProportionalScaledValue(CLASS_WIDTH));
			}

			m_pPlayerList->AddColumnToSection(iSectionID, "score", "", 0, scheme()->GetProportionalScaledValue(SCORE_WIDTH));
			m_pPlayerList->AddColumnToSection(iSectionID, "ping", "", 0, scheme()->GetProportionalScaledValue(PING_WIDTH));

			break;
		}
	case SECTION_TEAMONE_SQUAD1:
	case SECTION_TEAMONE_SQUAD2:
	//case SECTION_TEAMONE_SQUAD3:
	//case SECTION_TEAMONE_SQUAD4:
	case SECTION_TEAMTWO_SQUAD1:
	case SECTION_TEAMTWO_SQUAD2:
	//case SECTION_TEAMTWO_SQUAD3:
	//case SECTION_TEAMTWO_SQUAD4:
		{
			m_pPlayerList->AddSection(iSectionID, "", NULL, StaticPlayerSortFunc, StaticSquadSortFunc);
			m_pPlayerList->SetSectionHeaderOffset(iSectionID, scheme()->GetProportionalScaledValue(SQUAD_HEADER_OFFSET));
			m_pPlayerList->SetSectionItemOffset(iSectionID, scheme()->GetProportionalScaledValue(SQUAD_ITEM_OFFSET));

			int iNameLength = scheme()->GetProportionalScaledValue(NAME_WIDTH);

			if(m_pScoreboardManager)
				iNameLength -= scheme()->GetProportionalScaledValue(SQUAD_ITEM_OFFSET);

			m_pPlayerList->AddColumnToSection(iSectionID, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH));

			if(m_pScoreboardManager)
			{
				m_pPlayerList->AddColumnToSection(iSectionID, "dead", "", 0, scheme()->GetProportionalScaledValue(DEAD_WIDTH));
				m_pPlayerList->AddColumnToSection(iSectionID, "class", "", 0, scheme()->GetProportionalScaledValue(CLASS_WIDTH));
			}

			m_pPlayerList->AddColumnToSection(iSectionID, "level", "", 0, scheme()->GetProportionalScaledValue(SCORE_WIDTH));
			m_pPlayerList->AddColumnToSection(iSectionID, "ping", "", 0, scheme()->GetProportionalScaledValue(PING_WIDTH));

			break;
		}

	case SECTION_TEAMONE_UNASSIGNED:
	case SECTION_TEAMTWO_UNASSIGNED:
		{
			m_pPlayerList->AddSection(iSectionID, "");
			m_pPlayerList->SetSectionHeaderOffset(iSectionID, 20);
			m_pPlayerList->SetSectionItemOffset(iSectionID, 25);

			int iNameWidth = scheme()->GetProportionalScaledValue(NAME_WIDTH);

			m_pPlayerList->SetSectionLineLength(iSectionID, iNameWidth + 2);

			m_pPlayerList->AddColumnToSection(iSectionID, "name", "Unassigned Players", 0, iNameWidth);
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

void CScoreboard::UpdateSquadInfo(void)
{
	UpdateSquadInfo(TEAM_ONE);
	UpdateSquadInfo(TEAM_TWO);
}

void CScoreboard::UpdateSquadInfo(int iTeamID)
{
	if(!CanShowFullTeamInfo(GetGlobalTeam(iTeamID)))
		return;

	char szSquadInfo[MAX_SQUADNAME_LENGTH*2];
	wchar_t szString[256];

	C_PlayTeam *pTeam = GetGlobalPlayTeam(iTeamID);
	const Color &TeamColor = GetColorFromTeam(pTeam);

	for(int i = 0; i < pTeam->GetSquadCount(); i++)
	{
		C_INSSquad *pSquad = pTeam->GetSquad(i);
		int iSectionID = GetSectionIDFromSquadID(iTeamID, i);

		// name
		Q_snprintf(szSquadInfo, sizeof(szSquadInfo), "> %s", pSquad->GetName());
		localize()->ConvertANSIToUnicode(szSquadInfo, szString, sizeof(szString));

		m_pPlayerList->ModifyColumn(iSectionID, "name", szString);

		// score
		KeyValues *pSectionData = m_pPlayerList->GetSectionData(iSectionID);

		if(pSectionData)
			pSectionData->SetInt("score", pSquad->GetScore());
	
		// ping
		//Q_snprintf(szSquadInfo, sizeof(szSquadInfo), "%i", pSquad->GetPing());
		Q_snprintf(szSquadInfo, sizeof(szSquadInfo), " ");
		localize()->ConvertANSIToUnicode(szSquadInfo, szString, sizeof(szString));
		
		m_pPlayerList->ModifyColumn(iSectionID, "ping", szString);

		// set the color
		m_pPlayerList->SetSectionFgColor(iSectionID, TeamColor, true);
	}

	m_pPlayerList->SetSectionFgColor(GetSectionIDFromSquadID(iTeamID, DISABLED_SQUAD), TeamColor, true);
}

int CScoreboard::GetSectionIDFromSquadID(int iTeamID, int iSquadID)
{
	if(!CanShowFullTeamInfo(GetGlobalTeam(iTeamID)))
		return GetSectionIDFromTeamID(iTeamID);

	int iSectionID;

	switch(iSquadID)
	{
	case SQUAD_ONE:
		iSectionID = SECTION_TEAMONE_SQUAD1;
		break;
	case SQUAD_TWO:
		iSectionID = SECTION_TEAMONE_SQUAD2;
		break;
	/*case SQUAD_THREE:
		iSectionID = SECTION_TEAMONE_SQUAD3;
		break;
	case SQUAD_FOUR:
		iSectionID = SECTION_TEAMONE_SQUAD4;
		break;*/
	default:
		iSectionID = SECTION_TEAMONE_UNASSIGNED;
		break;
	}

	if(iTeamID == TEAM_TWO)
		iSectionID += SECTION_TEAMONE_UNASSIGNED;

	return iSectionID;
}

//=========================================================
//=========================================================
/*const char *g_PlayerInfoTypeNames[PLAYERSTATS_COUNT] = {
	{"Frags:"},							// PLAYERSTATS_FRAGS
	{"Headshots:"},						// PLAYERSTATS_HEADSHOTS
	{"Friendly Kills:"},				// PLAYERSTATS_FRIENDLYKILLS
	{"Deaths:"},						// PLAYERSTATS_DEATHS

	{"Accuracy with Pistols:"},			// PLAYERSTATS_ACCURACYPISTOLS
	{"Accuracy with Automatics:"},		// PLAYERSTATS_ACCURACYMACHINEGUNS
	{"Favourite Weapon:"},				// PLAYERSTATS_FAVOURITEWEAPON

	{"Damage Given:"},					 // PLAYERSTATS_DAMAGEGIVEN
	{"Damage Taken:"},					// PLAYERSTATS_DAMAGETAKEN
	{"HitZone Given:"},					// PLAYERSTATS_HITZONEGIVEN
	{"HitZone Taken:"},					// PLAYERSTATS_HITZONETAKEN

	{"Favourite Objective:"}			// PLAYERSTATS_FAVOURITEOBJECTIVE
};*/

#define SECTION_INFO_COLUMNS 3

enum SectionInfoColumnTypes_t
{
	SECTIONINFOCOLUMNTYPE_NAME = 0,
	SECTIONINFOCOLUMNTYPE_VALUE
};

#define SECTION_INFO_NAME

/*class CPlayerInfo : public EditablePanel
{
	DECLARE_CLASS_SIMPLE(CPlayerInfo, EditablePanel);

private:
	CScoreboardManager *m_pScoreboardManager;

	#define SECTION_INFO 0
	enum { NAME_WIDTH = 80, VALUE_WIDTH = 50 };
	SectionedListPanel *m_pInfoList;
	//PlayerStats_t m_PlayerStats[MAX_PLAYERS+1];

	int m_iPlayerWaitingID;

	Panel *m_pUpdating;

public:
	CPlayerInfo(CScoreboardManager *pScoreboardManager)
		: EditablePanel(pScoreboardManager, "PlayerInfo")
	{
		m_pScoreboardManager = pScoreboardManager;

		m_pInfoList = new SectionedListPanel(this, "PlayerInfo");
		m_pInfoList->SetClientSelectable(false);
		m_pInfoList->SetSectionHeaderVisible(SECTION_INFO, false);

		m_pUpdating = new Panel(this);
		m_pUpdating->SetVisible(false);

		m_iPlayerWaitingID = -1;

		SetVisible(false);

		LoadControlSettings("Resource/UI/Frames/Panels/Controls/PlayerInfo.res");
	}

	virtual void Setup(void)
	{
		char szBuffer[32];

		// delete everything
		m_pInfoList->DeleteAllItems();
		m_pInfoList->RemoveAllSections();

		// setup the columns
		m_pInfoList->AddSection(SECTION_INFO, "");

		for(int i = 0; i < SECTION_INFO_COLUMNS; i++)
		{
			GetColumnName(i, SECTIONINFOCOLUMNTYPE_NAME, szBuffer, sizeof(szBuffer));
			m_pInfoList->AddColumnToSection(SECTION_INFO, szBuffer, szBuffer, SectionedListPanel::COLUMN_RIGHT, scheme()->GetProportionalScaledValue(NAME_WIDTH));

			GetColumnName(i, SECTIONINFOCOLUMNTYPE_VALUE, szBuffer, sizeof(szBuffer));
			m_pInfoList->AddColumnToSection(SECTION_INFO, szBuffer, "", SectionedListPanel::COLUMN_CENTER, scheme()->GetProportionalScaledValue(VALUE_WIDTH));
		}

		// setup the rows
		KeyValues *pKV = new KeyValues("data");
		int iLastDivisible = 0;

		for(int i = 0; i < PLAYERSTATS_COUNT; i++)
		{
			int iRemainder = i % 3;

			if(i != 0 && iRemainder == 0)
			{
				m_pInfoList->AddItem(SECTION_INFO, pKV);

				iLastDivisible = i;
				pKV->Clear();
			}

			int iColumnID = i - iLastDivisible;

			GetColumnName(iColumnID, SECTIONINFOCOLUMNTYPE_NAME, szBuffer, sizeof(szBuffer));
			pKV->SetString(szBuffer, g_PlayerInfoTypeNames[i]);
		}
	}

	virtual void PlayerSelected(int iPlayerID)
	{
		if(iPlayerID == 0)
		{
			SetVisible(false);
			return;
		}
		else
		{
			SetVisible(true);
		}

		char szPlayerInfo[64];

		Q_snprintf(szPlayerInfo, sizeof(szPlayerInfo), "playerinfo %i %i",
			C_BasePlayer::GetLocalPlayer()->entindex(),
			iPlayerID);

		engine->ClientCmd(szPlayerInfo);

		WaitingForUpdate();
	}

	void HandleMsg(bf_read &Msg)
	{
		/*
		int iPlayerID = Msg.ReadByte();
		PlayerStats_t *pPlayerStats = &m_PlayerStats[iPlayerID];

		bool bUpdate = Msg.ReadByte();

		if(bUpdate)
		{
			for(int i = 0; i < PLAYERSTATS_COUNT; i++)
			{
				PlayerStats_t *pShiftedPlayerStats = (pPlayerStats += (i * sizeof(short)));
				short *pValue = reinterpret_cast<short*>(pShiftedPlayerStats);
				*pValue = Msg.ReadByte();
			}

			pPlayerStats->m_iPlayerID = iPlayerID;

			if(pPlayerStats->m_iDeaths != 0)
				pPlayerStats->m_flKillDeathRatio = (pPlayerStats->m_iFrags / pPlayerStats->m_iDeaths);
			else
				pPlayerStats->m_flKillDeathRatio = (float)pPlayerStats->m_iFrags;
		}

		UpdateReceived();
	}

protected:
	virtual void OnSizeChanged(int iWide, int iTall)
	{
		BaseClass::OnSizeChanged(iWide, iTall);

		m_pUpdating->SetSize(iWide, iTall);

		m_pInfoList->SetPos(0, 0);
		m_pInfoList->SetSize(iWide, iTall);
	}

private:
	void WaitingForUpdate(void)
	{
		m_pUpdating->SetVisible(true);
	}

	void UpdateReceived(void)
	{
		//char szBuffer[32];
		//char szSecBuffer[32];

		// show values
		m_pUpdating->SetVisible(false);

		// now update the rows
		/*KeyValues *pKV = new KeyValues("data");
		int iLastDivisible = 0;

		for(int i = 0; i < PLAYERSTATS_COUNT; i++)
		{
			int iRemainder = i % 3;

			if(i != 0 && iRemainder == 0)
			{
				m_pInfoList->AddItem(SECTION_INFO, iLastDivisible, pKV);

				iLastDivisible = i;
				pKV->Clear();
			}

			int iColumnID = i - iLastDivisible;

			// name
			GetColumnName(iColumnID, SECTIONINFOCOLUMNTYPE_NAME, szBuffer, sizeof(szBuffer));
			pKV->SetString(szBuffer, g_PlayerInfoTypeNames[i]);

			// info
			GetColumnName(iColumnID, SECTIONINFOCOLUMNTYPE_NAME, szBuffer, sizeof(szBuffer));
			GetValue(i, szSecBuffer, sizeof(szSecBuffer));
			pKV->SetString(szBuffer, szSecBuffer);

			GetColumnName(iColumnID, SECTIONINFOCOLUMNTYPE_VALUE, szBuffer, sizeof(szBuffer));
			pKV->SetString(szBuffer, szValue);
		}
	}

	void GetColumnName(int iColumn, int iColumnType, char *pszBuffer, int iBufferLength)
	{
		Assert(iColumn >= 0 && iColumn <= SECTION_INFO_COLUMNS);
		Assert(iColumnType == SECTIONINFOCOLUMNTYPE_NAME || iColumnType == SECTIONINFOCOLUMNTYPE_VALUE);

		const char *pszColumnType = ((iColumnType == SECTIONINFOCOLUMNTYPE_NAME) ? "name" : "value");
		Q_snprintf(pszBuffer, iBufferLength, "%s%i", pszColumnType, iColumn);
	}

	void GetValue(int iType, char *pszBuffer, int iLength)
	{
		switch(iType)
		{
			case PLAYERSTATS_FRAGS:
			case PLAYERSTATS_HEADSHOTS:
			case PLAYERSTATS_HEADSHOTS:
			case PLAYERSTATS_FRIENDLYKILLS:
			case PLAYERSTATS_DEATHS:
			case PLAYERSTATS_DAMAGEGIVEN:
			case PLAYERSTATS_DAMAGETAKEN:
			{
				if(iType == 
				

			}
		}
	}

	void CopyValueInt(int iValue, char *pszBuffer, int iLength)
	{
		//Q_strncpy(pszBuffer, iLength, "%i", iValue);
	}

	short &GetValueFromID(int iValue)
	{
		switch(iType)
		{
			case PLAYERSTATS_FRAGS:
			case PLAYERSTATS_HEADSHOTS:
			case PLAYERSTATS_HEADSHOTS:
			case PLAYERSTATS_FRIENDLYKILLS:
			case PLAYERSTATS_DEATHS:
			case PLAYERSTATS_DAMAGEGIVEN:
			case PLAYERSTATS_DAMAGETAKEN:
			{
				if(iType == 
				

			}
		}
	}
};*/

//=========================================================
//=========================================================
class CPlayerDisplay : public RichText
{
	DECLARE_CLASS_SIMPLE(CPlayerDisplay, RichText);

public:
	CPlayerDisplay(Panel *pParent, CScoreboardManager *pScoreboardManager)
		: RichText(pParent, "PlayerDisplay")
	{
		m_pScoreboardManager = pScoreboardManager;
	}

protected:
	void OnMousePressed(MouseCode code)
	{
		// will not need this when we're not a RichText anymore
		BaseClass::OnMousePressed(code);

		m_pScoreboardManager->DeselectPlayer();
	}

private:
	CScoreboardManager *m_pScoreboardManager;
};

//=========================================================
//=========================================================
CScoreboardManager::CScoreboardManager(Panel *pParent, Panel *pViewportPanel)
	: EditablePanel(pParent, NULL)
{
	m_pScoreboard = new CScoreboard(this, pViewportPanel, this);
	//m_pPlayerInfo = new CPlayerInfo(this);
	m_pPlayerDisplay = new CPlayerDisplay(this, this);

	LoadControlSettings("Resource/UI/Frames/Panels/Scoreboard.res");
}

//=========================================================
//=========================================================
void CScoreboardManager::HandleMsg(bf_read &Msg)
{
//	m_pPlayerInfo->HandleMsg(Msg);
}

//=========================================================
//=========================================================
void CScoreboardManager::PlayerSelected(int iPlayerID)
{
	m_pScoreboard->PlayerSelected(iPlayerID);
	//m_pPlayerInfo->PlayerSelected(iPlayerID);
}

//=========================================================
//=========================================================
void CScoreboardManager::DeselectPlayer(void)
{
	m_pScoreboard->DeselectPlayer();
	//m_pPlayerInfo->DeselectPlayer();
}

//=========================================================
//=========================================================
void CScoreboardManager::PerformLayout(void)
{
	m_pScoreboard->Setup();
}
