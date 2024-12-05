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
#include <cl_dll/iviewport.h>

#include "ingame.h"
#include "motd.h"
#include "mapinfo.h"
#include "scoreboard.h"
// objs
#include "comms.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
CInGame::CInGame(IViewPort *pViewPort) : PropertyDialog(NULL, PANEL_IGPLAYER)
{
	m_pViewPort = pViewPort;

	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	SetCancelButtonVisible(false);
	SetCloseButtonVisible(false);

	SetTitle("In-Game Player", false);

	// motd
	m_pMOTD = new CMOTD(this);
	AddPage(m_pMOTD, "MOTD");

	// mapinfo
	//IViewPortPanel *pPanel = pViewPort->FindPanelByName( PANEL_MAPINFO );
	//CMapInfo *pMapInfo = dynamic_cast<CMapInfo*>(pPanel);
	m_pMapInfo = new CMapInfo(this);
	AddPage(m_pMapInfo, "Map Overview");

	// scoreboard
	m_pScoreBoard = new CScoreBoard(this);
	AddPage(m_pScoreBoard, TABNAME_SCOREBOARD);

	// player objectives

	// comms
	m_pComms = new CComms(pViewPort, this);
	AddPage(m_pComms, "Communications");
	g_pCommsPanel = m_pComms;

	LoadControlSettings("Resource/UI/InGame.res");

	Reset();
}

//=========================================================
//=========================================================
CInGame::~CInGame()
{
}

//=========================================================
//=========================================================
void CInGame::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBackgroundType(2);
}

//=========================================================
//=========================================================
void CInGame::Reset( void )
{
	ResetAllData();

	//m_pScoreBoard->Reset();
}

//=========================================================
//=========================================================
void CInGame::Update()
{
	//m_pMapInfo->AddMessageFile(szNewMapName);

	//m_pTeamMenu->Update();
}

//=========================================================
//=========================================================
void CInGame::OnCommand(const char *command)
{


	BaseClass::OnCommand(command);
}

//=========================================================
//=========================================================
void CInGame::ShowPanel( bool bShow )
{
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
	}
}
