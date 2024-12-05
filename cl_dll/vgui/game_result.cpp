//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "filesystem.h"
#include <vgui/ischeme.h>
#include <vgui/isurface.h>

#include "ins_player_shared.h"

#include <keyvalues.h>

#include <vgui_controls/imagepanel.h>

#include <vgui_controls/richtext.h>
#include <cl_dll/iviewport.h>

#include "mapname_utils.h"

#include "game_result.h"
#include "ins_headers.h"

#include "play_team_shared.h"
#include "team_lookup.h"
#include "ins_gamerules.h"

#include "vgui_controls/vgui_helper.h"
#include <vgui_controls/embeddedimage.h>

#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define MAPIMAGE_ENDGAME "endgame"
#define MAX_VICTORY_IMAGES 5
#define INVALID_IMAGE -1

class ValidImages_t
{
public:
	void Reset(void)
	{
		m_bValid = false;
		m_ImageList.Purge();
	}

public:
	bool m_bValid;
	CUtlVector<int> m_ImageList;
};

class CVictoryImage : public EmbeddedImage
{
	DECLARE_CLASS_SIMPLE(CVictoryImage, EmbeddedImage);

private:
	ValidImages_t m_ValidImages1, m_ValidImages2;

public:
	CVictoryImage(Panel *pParent, const char *pszPanelName)
		: EmbeddedImage(pParent, pszPanelName) { }

	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		SetBorder(pScheme->GetBorder("BaseBorder"));
	}

	void Reset(void)
	{
		GetValidImages(TEAM_ONE).Reset();
		GetValidImages(TEAM_TWO).Reset();
	}

	void LoadImages(void)
	{
		Reset();

		LoadImages(TEAM_ONE);
		LoadImages(TEAM_TWO);
	}

	void LoadImages(int iTeam)
	{
		char szImageBuffer[_MAX_PATH];
		ValidImages_t &ValidImages = GetValidImages(iTeam);

		const char *pszTeamPrefix = C_PlayTeam::GetNameFromID(iTeam);
		ValidImages.m_bValid = GetMapImagePath(MAPIMAGE_ENDGAME, pszTeamPrefix, true, szImageBuffer, sizeof(szImageBuffer), &ValidImages.m_ImageList);
	}

	void SetWinner(int iTeam)
	{
		char szImageBuffer[_MAX_PATH];
		ValidImages_t &ValidImages = GetValidImages(iTeam);

		Assert(ValidImages.m_ImageList.Count() != 0);
		int iImage = ValidImages.m_ImageList[random->RandomInt(0, ValidImages.m_ImageList.Count() - 1)];

		const char *pszTeamPrefix = C_PlayTeam::GetNameFromID(iTeam);
		FormMapImagePath(MAPIMAGE_ENDGAME, pszTeamPrefix, ValidImages.m_bValid, szImageBuffer, sizeof(szImageBuffer));
		FormCyclePath(iImage, szImageBuffer, sizeof(szImageBuffer));

		SetImage(szImageBuffer);
	}

private:
	ValidImages_t &GetValidImages(int iTeam)
	{
		return (iTeam == TEAM_ONE) ? m_ValidImages1 : m_ValidImages2;
	}
};

//=========================================================
//=========================================================
CGameResult::CGameResult(Panel *pParent, const char *pszPanelName)
	: EditablePanel(pParent, pszPanelName)
{
	SetScheme("ClientScheme");
	SetProportional(true);

	m_bNeedsUpdate = true;

	m_pVictory = new CVictoryImage(this, "Victory");
	m_pWinner = new vgui::ImagePanel(this, "Winner");
	m_pResult = new CResultGenerator(this, "Result");

	m_bResultSet = false;

	m_pResult->SetVerticalScrollbar(false);

	LoadControlSettings("Resource/UI/Frames/Panels/GameResult.res");
}
//=========================================================
//=========================================================
void CGameResult::PerformLayout(void)
{
	BaseClass::PerformLayout();

	if(!m_bResultSet)
		return;

	UpdateMap();

	m_pVictory->SetWinner(m_Result.m_iTeam);
	UpdateWinner();
	m_pResult->Generate(m_Result);
}

//=========================================================
//=========================================================
void CGameResult::Reset(void)
{
	m_pResult->Reset();
	m_bResultSet = false;
}

//=========================================================
//=========================================================
void CGameResult::OnLevelInit(void)
{
	m_bNeedsUpdate = true;
}

//=========================================================
//=========================================================
void CGameResult::SetResult(const Result_t &Result)
{
	m_Result = Result;
	m_bResultSet = true;

	InvalidateLayout();
}

//=========================================================
//=========================================================
void CGameResult::UpdateWinner(void)
{
	const char *pszFileName = GetGlobalPlayTeam(m_Result.m_iTeam)->GetTeamLookup()->GetFileName();

	char szTeamWinPath[MAX_PATH];
	Q_snprintf(szTeamWinPath, sizeof(szTeamWinPath), "teams/%s/endgame/win", pszFileName);

	m_pWinner->SetImage(szTeamWinPath);
}

//=========================================================
//=========================================================
void CGameResult::UpdateMap(void)
{
	if(m_bNeedsUpdate)
	{
		m_pVictory->LoadImages();
		m_bNeedsUpdate = false;
	}
}