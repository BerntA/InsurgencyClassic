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

#include "initialsetup.h"
#include "motd.h"
#include "mapinfo.h"
#include "ins_headers.h"
#include "ins_gamerules.h"
#include "ins_player_shared.h"
#include "ins_stats_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TABNAME_CREDITS "Credits"
#define CREDITS_PATH "materials/VGUI/resources/credits/credits.htm"

using namespace vgui;

//=========================================================
//=========================================================
CInitialSetup::CInitialSetup(IViewPort *pViewPort)
	: PropertyDialog(NULL, PANEL_INITIALSETUP)
{
	SetScheme("ClientScheme");
	SetCancelButtonVisible(false);

	m_pViewPort = pViewPort;

	// setup the frame headers
	INSFrameHeader *pTabHeader = new INSFrameHeader(this, NULL);

	AddHeader(pTabHeader);
	AddHeader(new CServerHeader(this, "ServerHeader"));
	AddHeader(new CINSHeader(this, "INSHeader"));

	GetPropertySheet()->CreateTabParent(pTabHeader);

	// add the customise button
	AddButton("Customize", "Custom", false);

	// add the pages
	m_pMOTD = new CMOTD(this, PANEL_MOTD);
	AddPage(m_pMOTD, "MOTD");

	m_pCredits = new CHTMLWindow(this);
	m_pCredits->LoadFile(CREDITS_PATH);
	AddPage(m_pCredits, TABNAME_CREDITS);

	m_pMapInfo = new CMapInfo(this);
	AddPage(m_pMapInfo, TABNAME_MAPINFO);

	// load the settings
	LoadControlSettings("Resource/UI/Frames/InitialSetup.res");
}

//=========================================================
//=========================================================
void CInitialSetup::Reset(void)
{
	ResetAllData();

	m_pMapInfo->Reset();
}

//=========================================================
//=========================================================
void CInitialSetup::OnCommand(const char *pszCommand)
{
	if(!stricmp(pszCommand, "OK"))
	{
		if(INSRules()->GetStatsType() != STATSTYPE_BLOCKED)
		{
			engine->ClientCmd("changeteam");

			if(!CINSStats::IsBlocked(C_INSPlayer::GetLocalPlayer()))
			{
				engine->ClientCmd("changeteam");
			}
			else
			{
				//IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName(PANEL_PLAYERLOGIN);
				//m_pViewPort->ShowPanel(pPanel, true);
			}
		}

		m_pViewPort->ShowPanel(this, false);
		Reset();
		return;
	}
	else if(!stricmp(pszCommand, "Custom"))
	{
		gViewPortInterface->ShowPanel(PANEL_CUSTOMIZEGUI, true);
		return;
	}

	BaseClass::OnCommand(pszCommand);
}

//=========================================================
//=========================================================
void CInitialSetup::ShowPanel(bool bShow)
{
	if(IsVisible() == bShow)
		return;

	Assert(g_pGameRules);

	if(bShow)
	{
		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}
}