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

#include <keyvalues.h>

#include <vgui_controls/propertydialog.h>
#include <vgui_controls/propertysheet.h>
#include <vgui_controls/imagepanel.h>

#include <vgui_controls/richtext.h>
#include <cl_dll/iviewport.h>

#include "mapname_utils.h"

#include "endgame.h"
#include "ins_headers.h"
#include "game_result.h"
#include "ins_gamerules.h"

#include "scoreboard.h"

#include "c_play_team.h"
#include "team_lookup.h"

#include "vgui_controls/vgui_helper.h"

#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Header for Team Choice
//-----------------------------------------------------------------------------
class CEndGameHeader : public INSTopHeader
{
	DECLARE_CLASS_SIMPLE(CEndGameHeader, INSTopHeader);

public:
	CEndGameHeader(INSFrame *pParent, const char *pszName)
		: INSTopHeader(pParent, pszName)
	{
		LoadControlSettings("Resource/UI/Frames/Headers/EndGame.res");
	}

private:
	virtual bool UseDefaultTall(void) const { return false; }
};

//=========================================================
//=========================================================
CEndGame::CEndGame(IViewPort *pViewPort) : PropertyDialog(NULL, PANEL_ENDGAME)
{
	SetScheme("ClientScheme");
	SetProportional(true);
	SetCancelButtonVisible(false);
	SetOKButtonVisible(false);

	INSFrameHeader *pTabHeader = new INSFrameHeader(this, NULL);

	AddHeader(pTabHeader);
	AddHeader(new CEndGameHeader(this, "EndGameHeader"));

	GetPropertySheet()->CreateTabParent(pTabHeader);

	m_pGameResult = new CGameResult(this, "gameresult");
	AddPage(m_pGameResult, TABNAME_GAMERESULT);
	
	//CScoreBoard *pScoreBoard = new CScoreBoard(this);
	//pScoreBoard->SetType(SCOREBOARD_FULL);
	//AddPage(pScoreBoard, TABNAME_SCOREBOARD);

	LoadControlSettings("Resource/UI/Frames/EndGame.res");

	Reset();
}

//=========================================================
//=========================================================
void CEndGame::SetData(KeyValues *data)
{
	Assert(INSRules() && INSRules()->IsStatusRunning() && !INSRules()->GetRunningMode()->GetStatus(GAMERUNNING_PLAYERWAIT));

	Result_t Result = { data->GetInt("team"),
						data->GetInt("type"),
						data->GetInt("length"),
						data->GetInt("balance") };

	m_pGameResult->SetResult(Result);
}

//=========================================================
//=========================================================
void CEndGame::Reset( void )
{
	ResetAllData();

	m_pGameResult->Reset();
}

//=========================================================
//=========================================================
void CEndGame::OnCommand(const char *command)
{
	 if(!stricmp(command, "OK"))
	 {
		ShowPanel(false);
		Reset();
	 }

	BaseClass::OnCommand(command);
}

//=========================================================
//=========================================================
void CEndGame::ShowPanel( bool bShow )
{
	if(!g_pGameRules)
		return;

	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );

		Reset();
	}
}
