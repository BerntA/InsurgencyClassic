//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "squadboard.h"

#include "scoreboard.h"
#include "scoreboard_base.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
class CSquadBoardHeader : public INSTopHeader
{
	DECLARE_CLASS_SIMPLE(CSquadBoardHeader, INSTopHeader);

public:
	CSquadBoardHeader(INSFrame *pParent, const char *pszName);

private:
	virtual bool UseDefaultTall(void) const { return false; }
};

//=========================================================
//=========================================================
CSquadBoardHeader::CSquadBoardHeader(INSFrame *pParent, const char *pszName)
	: INSTopHeader(pParent, pszName)
{
	LoadControlSettings("Resource/UI/Frames/Headers/SquadBoard.res");
}

//=========================================================
//=========================================================
CSquadBoard::CSquadBoard(IViewPort *pViewPort)
	: INSFrame(NULL, PANEL_QSCOREBOARD)
{
	SetScheme("ClientScheme");

	m_pScoreboard = new CScoreboard(this, this, NULL);

	LoadControlSettings("Resource/UI/Frames/SquadBoard.res");
}

//=========================================================
//=========================================================
void CSquadBoard::ShowPanel( bool bShow )
{
	if(IsVisible() == bShow)
		return;

	Assert(g_pGameRules);

	if(bShow)
	{
		Activate();
	}
	else
	{
		SetVisible(false);
	}
}

//=========================================================
//=========================================================
void CSquadBoard::PerformLayout(void)
{
	m_pScoreboard->Setup();
}
