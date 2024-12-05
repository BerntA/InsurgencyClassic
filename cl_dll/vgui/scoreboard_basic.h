//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SCOREBOARD_BASIC_H
#define SCOREBOARD_BASIC_H
#ifdef _WIN32
#pragma once
#endif

#include "scoreboard_base.h"

//-----------------------------------------------------------------------------
// Purpose: Basic ScoreBoard
//-----------------------------------------------------------------------------
class CScoreboardBasic : public CScoreboardBase
{
private:
	DECLARE_CLASS_SIMPLE(CScoreboardBasic, CScoreboardBase);

public:
	CScoreboardBasic(vgui::Panel *pParent, vgui::Panel *pViewpanelParent);

protected:
	virtual void InitScoreboardSections(void);
	virtual void AddHeader(void);
	
	virtual void UpdatePlayerInfo(void);

	virtual int GetSectionIDFromTeamID(int iTeamID);

private:
	void AddSection(int iSectionID);

private:
	enum { NAME_WIDTH = 160, SCORE_WIDTH = 60, PING_WIDTH = 80 };
};

#endif // SCOREBOARD_BASIC_H
