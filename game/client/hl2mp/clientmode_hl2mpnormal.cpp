//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Draws the normal TF2 or HL2 HUD.
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "clientmode_hl2mpnormal.h"
#include "vgui_int.h"
#include "hud.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include "iinput.h"
#include "hl2mptextwindow.h"
#include "ienginevgui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Instance the singleton and expose the interface to it.
IClientMode* GetClientModeNormal()
{
	static ClientModeHL2MPNormal g_ClientModeNormal;
	return &g_ClientModeNormal;
}

ClientModeHL2MPNormal* GetClientModeHL2MPNormal()
{
	Assert(dynamic_cast<ClientModeHL2MPNormal*>(GetClientModeNormal()));
	return static_cast<ClientModeHL2MPNormal*>(GetClientModeNormal());
}

//-----------------------------------------------------------------------------
// Purpose: this is the viewport that contains all the hud elements
//-----------------------------------------------------------------------------
class CHudViewport : public CBaseViewport, public CAutoGameSystem
{
private:
	DECLARE_CLASS_SIMPLE(CHudViewport, CBaseViewport);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);
		gHUD.InitColors(pScheme);
		SetPaintBackgroundEnabled(false);
	}

	virtual IViewPortPanel* CreatePanelByName(const char* szPanelName);

	virtual void LevelInitPostEntity(void)
	{
		ResetAllPanels();
	}
};

int ClientModeHL2MPNormal::GetDeathMessageStartHeight(void)
{
	return m_pViewport->GetDeathMessageStartHeight();
}

IViewPortPanel* CHudViewport::CreatePanelByName(const char* szPanelName)
{
	IViewPortPanel* newpanel = NULL;

	if (Q_strcmp(PANEL_INFO, szPanelName) == 0)
	{
		newpanel = new CHL2MPTextWindow(this);
		return newpanel;
	}

	return BaseClass::CreatePanelByName(szPanelName);
}

//-----------------------------------------------------------------------------
// ClientModeHLNormal implementation
//-----------------------------------------------------------------------------
ClientModeHL2MPNormal::ClientModeHL2MPNormal()
{
	m_pViewport = new CHudViewport();
	m_pViewport->Start(gameuifuncs, gameeventmanager);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeHL2MPNormal::~ClientModeHL2MPNormal()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeHL2MPNormal::Init()
{
	BaseClass::Init();
}