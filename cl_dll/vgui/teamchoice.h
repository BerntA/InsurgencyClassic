//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_TEAMCHOICE_H
#define VGUI_TEAMCHOICE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/editablepanel.h>

namespace vgui
{
	class RichText;
	class ImageCyclerGroup;
};

class CChangeTeam;
class CChoiceButton;
class CTeamChoiceButton;
class CPlayTeamChoiceButton;

enum TeamImageGroups_t
{
	GROUP_UNKNOWN = 0,
	GROUP_TEAMONE,
	GROUP_TEAMTWO
};

#define TABNAME_TEAMCHOICE "Team Selection"

// ------------------------------------------------------------------------------------ //
// Team Choices
// ------------------------------------------------------------------------------------ //
class CTeamChoice : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CTeamChoice, vgui::EditablePanel);

public:
	CTeamChoice(CChangeTeam *pParent);

	void Reset(void);
	void SetTeam(int iTeam);

protected:
	virtual void PerformLayout(void);
	virtual void OnCommand(const char *pszCommand);

private:
	void ShowTeam(int iTeam, bool bForceOut);

	void CreateTeamSelectPath(char *pszBuffer, const char *pszName);

private:
	CPlayTeamChoiceButton *m_pJoinOne;
	CPlayTeamChoiceButton *m_pJoinTwo;
	CChoiceButton *m_pAutoAssign;
	CTeamChoiceButton *m_pSpectator;

	vgui::ImageCyclerGroup *m_pPreview;
	vgui::RichText *m_pDescription;

	bool m_bUsed;
};


#endif // VGUI_TEAMCHOICE_H
