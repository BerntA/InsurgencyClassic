//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <keyvalues.h>
#include <filesystem.h>
#include <vgui_controls/richtext.h>
#include <vgui_controls/listbox.h>
#include <vgui_controls/imagecycler.h>
#include <cl_dll/iviewport.h>
#include "mapinfo.h"
#include "c_ins_imcsync.h"
#include "imc_config.h"

#include "ins_gamerules.h"
#include "c_ins_obj.h"
#include "obj_manager.h"
#include "c_play_team.h"
#include "ins_squad_shared.h"
#include "mapname_utils.h"

#include <vgui/isurface.h>

#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
CMapInfo::CMapInfo(Panel *pParent) : EditablePanel(pParent, PANEL_MAPINFO)
{
	SetScheme("ClientScheme");

	m_pOverview = new ListBox(this, "ServerOverview");
	m_pDescription = new RichText(this, "MapDescription");
	m_pPreviewImages = new ImageCycler(this, "MapPreview");

	m_pOverview->SetAllowMouse(false);
	m_pDescription->SetAllowMouse(false);
	m_pDescription->SetMaximumCharCount(MAX_MAPOVERVIEW_LENGTH);

	m_bSetupPanel = false;

	LoadControlSettings("Resource/UI/Frames/Panels/MapInfo.res");
}

//=========================================================
//=========================================================
void CMapInfo::Reset(void)
{
	m_bSetupPanel = true;
}

//=========================================================
//=========================================================
void CMapInfo::OnVisibilityChange(int iOldVisible)
{
	BaseClass::OnVisibilityChange(iOldVisible);

	if(!IsVisible() || !m_bSetupPanel)
		return;

	// do nothing when theres no IMC
	if(!g_pIMCSync || !IMCConfig())
		return;

	// setup overview
	float flRoundLength = g_pIMCSync->GetRoundLength() / 60;
	char szRoundLength[64];
	sprintf(szRoundLength, "%.2f (Minutes)", flRoundLength);

	char szObjectives[2048];

	const CUtlVector< C_INSObjective* > Objectives;

	if(Objectives.Count() > 0)
	{
		sprintf(szObjectives, "%i (", iObjectiveCount);

		for(int i = 0; i < Objectives.Count(); i++)
		{
			C_INSObjective *pObjective = C_INSObjective::GetObjective(i);

			if(!pObjective)
				continue;

			Q_strncat(szObjectives, pObjective->GetName(), sizeof(szObjectives), COPY_ALL_CHARACTERS);
			Q_strncat(szObjectives, ", ", sizeof(szObjectives), COPY_ALL_CHARACTERS);
		}

		int iObjectivesLength = strlen(szObjectives) + 1;
		szObjectives[iObjectivesLength - 3] = '\0';

		Q_strncat(szObjectives, ")", sizeof(szObjectives), COPY_ALL_CHARACTERS);
	}
	else
	{
		strcpy(szObjectives, "None");
	}

	char szVersion[2];
	Q_snprintf(szVersion, sizeof(szVersion), "%i",  g_pIMCSync->GetVersion());

	m_pOverview->ClearText();
	m_pOverview->InsertLine("Map Name", IMCConfig()->GetMapName());
	m_pOverview->InsertLine("Version", szVersion);
	m_pOverview->InsertLine("Offical", g_pIMCSync->IsOffical() ? "Yes" : "No");
	m_pOverview->InsertLine("Operation Date", "March 30th 2004");
	m_pOverview->InsertLine("Profile", IMCConfig()->GetProfileName());
	m_pOverview->InsertLine("Objectives", szObjectives);
	m_pOverview->InsertLine("Game-Type", INSRules()->GetRuleName());
	m_pOverview->InsertLine("Round Length", szRoundLength);
	m_pOverview->InsertLine();

	for(int i = TEAM_ONE; i <= TEAM_TWO; i++)
	{
		C_PlayTeam *pPlayTeam = (C_PlayTeam*)g_Teams[i];

		char szTeamString[64];
		const char *pszName = (i == TEAM_ONE) ? "One" : "Two";

		sprintf(szTeamString, "Team %s", pszName);
		m_pOverview->InsertLine(szTeamString, pPlayTeam->Get_Name());

		char szMaxWaves[64];

		if(pPlayTeam->m_iMaxWaves != UNLIMITED_SUPPLIES)
			sprintf(szMaxWaves, "%i", pPlayTeam->m_iMaxWaves);
		else
			strcpy(szMaxWaves, "Unlimited");

		m_pOverview->InsertLine("Reinforcement Waves", szMaxWaves, true);

		char szTimeWave[64];
		sprintf(szTimeWave, "%i (Seconds)", pPlayTeam->GetTimeWave());

		m_pOverview->InsertLine("Time Between Waves", szTimeWave, true);

		char szSquadString[256];
		szSquadString[0] = 0;

		//strcpy(szSquadString, "% i

		for(int j = 0; j < pPlayTeam->GetSquadCount(); j++)
		{
			C_INSSquad *pSquad = pPlayTeam->GetSquad(j);

			Q_strncat(szSquadString, pSquad->GetName(), sizeof(szSquadString), COPY_ALL_CHARACTERS);
			Q_strncat(szSquadString, ", ", sizeof(szSquadString), COPY_ALL_CHARACTERS);
		}

		int iSquadsLength = (strlen(szSquadString) + 1);
		szSquadString[iSquadsLength - 3] = 0;

		m_pOverview->InsertLine("Squads", szSquadString, true);

		m_pOverview->InsertLine();
	}

	// setup description
	m_pDescription->ClearText();
	m_pDescription->SetText(IMCConfig()->GetMapOverview());

	// setup image group
	char szOverviewPath[1024];
	GetMapImagePath("overview", "", true, szOverviewPath, sizeof(szOverviewPath));

	m_pPreviewImages->Reset();
	m_pPreviewImages->AddImages(szOverviewPath);

	// no more setup
	m_bSetupPanel = false;
}
