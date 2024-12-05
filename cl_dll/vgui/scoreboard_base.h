//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SCOREBOARD_BASE_H
#define SCOREBOARD_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/editablepanel.h>

#define TABNAME_SCOREBOARD "Scoreboard"

#define SECTION_HEADER 0

//-----------------------------------------------------------------------------
// Purpose: Base Scoreboard
//-----------------------------------------------------------------------------
class CScoreboardBase : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CScoreboardBase, vgui::EditablePanel);

public:
	CScoreboardBase(vgui::Panel *pParent, vgui::Panel *pViewportPanel);

	virtual void Setup(void);

	void SetClientSelectable(bool bState);

protected:
	virtual void OnSizeChanged(int iWide, int iTall);
	virtual void OnVisibilityChange(int iOldVisible);
	virtual void OnTick(void);

	MESSAGE_FUNC(OnResetData, "ResetData");
	MESSAGE_FUNC_INT(ItemLeftClick, "ItemLeftClick", iItemID);
 	
	virtual void InitScoreboardSections(void);
	virtual void AddHeader(void);

	virtual void Update(void);
	static bool StaticPlayerSortFunc(vgui::SectionedListPanel *pList, int iItemID1, int iItemID2);

	virtual void FillScoreBoard(void);
	virtual void UpdateTeamInfo(void);
	virtual void UpdatePlayerInfo(void) { }

	virtual int GetSectionIDFromTeamID(int iTeamID) { return 0; }
	int FindItemIDForPlayerIndex(int iPlayerIndex);

	bool CanShowFullTeamInfo(const C_Team *pTeam) const;

protected:
	vgui::Panel *m_pViewportPanel;

	int m_iPlayerIndexSymbol;
	vgui::SectionedListPanel *m_pPlayerList;

	int m_iSelectedItemID;
};

#endif // SCOREBOARD_BASE_H
