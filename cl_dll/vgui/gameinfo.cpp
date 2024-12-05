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
#include <vgui_controls/insframe.h>
#include <vgui_controls/imagecycler.h>
#include <vgui_controls/imagepanel.h>
#include <cl_dll/iviewport.h>

#include "gameinfo.h"
#include "motd.h"
#include "mapinfo.h"
#include "mapover.h"
#include "scoreboard.h"
#include "scoreboard_base.h"
#include "hud_macros.h"
#include "comms.h"

#include "clientmode_shared.h"

#include "hud_stringfont.h"

#include "imc_config.h"
#include "c_ins_imcsync.h"
#include "mapname_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
class CMapName : public Panel
{
	DECLARE_CLASS_SIMPLE(CMapName, Panel);

public:
	CMapName(Panel *pParent)
		: Panel(pParent, "MapName")
	{
		SetPaintBackgroundEnabled(false);

		SetSize(512, 256);
		SetPos(0, 0);
	}

protected:
	virtual void Paint(void)
	{
		if(!IMCConfig())
			return;

		g_pHudStringDisplay->DrawStringWithOverlay(0, 10,
			IMCConfig()->GetMapName(),
			IMCConfig()->GetMapNColorF(), IMCConfig()->GetMapNColorB(),
			1.0f, STRINGFONT_MAPNAME);
	}
};

class CMapHeader : public INSTextHeader
{
private:
	DECLARE_CLASS_SIMPLE(CMapHeader, INSTextHeader);

	ImageCycler *m_pMapPreview;
	CMapName *m_pMapName;
	//CMapName *m_pMapName2;

public:
	CMapHeader(INSFrame *pParent, const char *pszName)
		: INSTextHeader(pParent, pszName)
	{
		SetPaintBackgroundEnabled(false);

		m_pMapPreview = new ImageCycler(this, "MapPreview");
		m_pMapPreview->SetHoldTime(5.5f); // NOTE: some defines please
		m_pMapPreview->SetFadeImages(true);
		m_pMapPreview->SetFadeTime(1.3f);

		//m_pMapName = new ImagePanel(this, "MapName");

		m_pMapName = new CMapName(this);

		LoadControlSettings("Resource/UI/Frames/Headers/MapHeader.res");
	}

protected:
	void ApplySettings(KeyValues *inResourceData)
	{
		BaseClass::ApplySettings(inResourceData);
	}

	virtual void PerformLayout(void)
	{
		BaseClass::PerformLayout();

		//if(g_pIMCSync == NULL)
		//	return;

		//SetText(g_pIMCSync->GetMapName());


		// setup image group
		char szFolderPath[1024];

		GetMapImagePath("gibanner", "background", true, szFolderPath, sizeof(szFolderPath));
		m_pMapPreview->AddImages(szFolderPath);
	}

private:
	virtual bool UseDefaultTall(void) const { return false; }
};

//=========================================================
//=========================================================
CGameInfo *g_pGameInfo = NULL;

void __MsgFunc_PlayerInfo(bf_read &msg)
{
	if(g_pGameInfo)
		g_pGameInfo->HandleMsg(msg);
}

CGameInfo::CGameInfo(IViewPort *pViewPort)
	: PropertyDialog(NULL, PANEL_GAMEINFO)
{
	Assert(!g_pGameInfo);
	g_pGameInfo = this;

	m_pViewPort = pViewPort;

	HOOK_MESSAGE(PlayerInfo);

	SetScheme("ClientScheme");
	SetProportional(true);

	SetCancelButtonVisible(false);
	//SetCloseButtonVisible(false);

	//SetTitle("Game Information", false);

	EnableQuickClose(true);


	INSFrameHeader *pTabHeader = new INSFrameHeader(this, NULL);

	AddHeader(pTabHeader);
	AddHeader(new CMapHeader(this, "MapHeader"));

	GetPropertySheet()->CreateTabParent(pTabHeader);


	//AddHeader(new CINSHeader(this, "INSHeader"));


	// motd
	m_pMOTD = new CMOTD(this, "motd");
	AddPage(m_pMOTD, "MOTD");

	// mapinfo
	//IViewPortPanel *pPanel = pViewPort->FindPanelByName( PANEL_MAPINFO );
	//CMapInfo *pMapInfo = dynamic_cast<CMapInfo*>(pPanel);
	m_pMapInfo = new CMapInfo(this);
	AddPage(m_pMapInfo, TABNAME_MAPINFO);

	// mapover
	m_pMapOver = new CMapOver(this);
	AddPage(m_pMapOver, TABNAME_MAPOVER);

	// scoreboard
	m_pScoreboard = new CScoreboardManager(this, this);
	AddPage(m_pScoreboard, TABNAME_SCOREBOARD);

	// player objectives

	// comms
	m_pComms = new CComms(this);
	AddPage(m_pComms, "Communications");

	LoadControlSettings("Resource/UI/Frames/GameInfo.res");

	Reset();
}

//=========================================================
//=========================================================
CGameInfo::~CGameInfo()
{
	g_pGameInfo = NULL;
}

//=========================================================
//=========================================================
void CGameInfo::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBorder(pScheme->GetBorder("FrameBorder"));
}

//=========================================================
//=========================================================
void CGameInfo::Reset( void )
{
	ResetAllData();

	m_pMapInfo->Reset();

	//m_pScoreBoard->Reset();
}

//=========================================================
//=========================================================
void CGameInfo::Update()
{
	//m_pMapInfo->AddMessageFile(szNewMapName);

	//m_pTeamMenu->Update();
}

//=========================================================
//=========================================================
void CGameInfo::OnCommand(const char *command)
{
	 if(!stricmp(command, "OK"))
	 {
		m_pViewPort->ShowPanel(this, false);
		Reset();

		return;
	 }

	BaseClass::OnCommand(command);
}

//=========================================================
//=========================================================
void CGameInfo::ShowPanel( bool bShow )
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

//=========================================================
//=========================================================
void CGameInfo::HandleMsg(bf_read &Msg)
{
	m_pScoreboard->HandleMsg(Msg);
}
