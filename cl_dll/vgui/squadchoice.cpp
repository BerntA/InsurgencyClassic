//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <cdll_client_int.h>

#include <vgui/ivgui.h>
#include "squadchoice.h"

#include <vgui/ischeme.h>
#include <vgui/isurface.h>
#include <keyvalues.h>
#include <filesystem.h>

#include <vstdlib/ikeyvaluessystem.h>

#include <igameresources.h>

#include <vgui_controls/sectionedlistpanel.h>
#include <cl_dll/iviewport.h>

#include "ins_gamerules.h"
#include "c_team.h"
#include "c_ins_squad.h"

#include "squadsetup.h"

#include "tier0/memdbgon.h"

using namespace vgui;



//=========================================================
//=========================================================
CSquadChoice::CSquadChoice(Panel *pParent) : EditablePanel(pParent, PANEL_SQUAD)
{
	SetScheme("ClientScheme");

	m_pPlayerOrder = new vgui::CPlayerOrder(this, "PlayerOrder");
	m_pSquadSetup = (CSquadSetup*)pParent;

	LoadControlSettings("Resource/UI/SquadChoice.res");
}

//=========================================================
//=========================================================
void CSquadChoice::OnResetData(void)
{
	m_pPlayerOrder->Reset();
}

//=========================================================
//=========================================================
void CSquadChoice::RequestFocus(int direction)
{
	m_pPlayerOrder->Update();
}

//=========================================================
//=========================================================
void CSquadChoice::SetProcessState(bool bState)
{
	m_pPlayerOrder->SetProcessState(bState);
}

//=========================================================
//=========================================================
bool CSquadChoice::ShouldUpdate(void)
{
	return m_pSquadSetup->IsVisible();
}