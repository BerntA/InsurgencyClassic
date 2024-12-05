//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SCOREBOARD_H
#define SCOREBOARD_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/editablepanel.h>

class CScoreboard;
class CPlayerInfo;
//class CPlayerDisplay; -- will impliment once deathz0rz has got the code from Valve
class CPlayerDisplay;

#include "scoreboard_base.h"

//=========================================================
//=========================================================
class CScoreboardManager : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CScoreboardManager, vgui::EditablePanel);

public:
	CScoreboardManager(vgui::Panel *pParent, vgui::Panel *pViewportPanel);

	void HandleMsg(bf_read &Msg);

	void PlayerSelected(int iPlayerID);
	void DeselectPlayer(void);

protected:
	virtual void PerformLayout(void);

private:
	CScoreboard *m_pScoreboard;
	//CPlayerInfo *m_pPlayerInfo;
	CPlayerDisplay *m_pPlayerDisplay;
};

//=========================================================
//=========================================================
class CScoreboard : public CScoreboardBase
{
	DECLARE_CLASS_SIMPLE(CScoreboard, CScoreboardBase);

public:
	CScoreboard(vgui::Panel *pParent, vgui::Panel *pViewportPanel, CScoreboardManager *pScoreboardManager);

	virtual void PlayerSelected(int iPlayerID);
	void DeselectPlayer(void);

protected:
	virtual void ApplySettings(KeyValues *inResourceData);
	virtual void ItemLeftClick(int iItemID);

	virtual void InitScoreboardSections(void);
	virtual void AddHeader(void);
	virtual void FillScoreBoard(void);

	virtual void UpdatePlayerInfo(void);

	virtual int GetSectionIDFromTeamID(int iTeamID);

private:
	void AddSection(int iSectionID);

	void UpdateSquadInfo(void);
	void UpdateSquadInfo(int iTeamID);

	int GetSectionIDFromSquadID(int iTeamID, int iSquadID);

private:
	CScoreboardManager *m_pScoreboardManager;

	enum { NAME_WIDTH = 160, DEAD_WIDTH = 40, CLASS_WIDTH = 65, SCORE_WIDTH = 50, PING_WIDTH = 30 };
	enum { TEAM_HEADER_OFFSET = 6, TEAM_ITEM_OFFSET = 9,
		SQUAD_HEADER_OFFSET = 13, SQUAD_ITEM_OFFSET = 16 };

	int m_iWide, m_iTall;
	int m_iTallSmall;
};


#endif // SCOREBOARD_H
