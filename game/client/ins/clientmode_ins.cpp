#include "cbase.h"
#include "clientmode_ins.h"
#include "vgui_int.h"
#include "hud.h"
#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include "iinput.h"
#include "ienginevgui.h"
#include "panelmetaclassmgr.h"
#include "inshud.h"
#include "insvgui.h"
#include "hint_helper.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar default_fov("default_fov", "75", FCVAR_CHEAT);
ConVar fov_desired("fov_desired", "75", FCVAR_ARCHIVE | FCVAR_USERINFO, "Sets the base field-of-view.", true, 75.0, true, 120.0);

IClientMode* g_pClientMode = NULL;

#define SCREEN_FILE		"scripts/vgui_screens.txt"

IClientMode* GetClientModeNormal()
{
	static ClientModeINSNormal g_ClientModeNormal;
	return &g_ClientModeNormal;
}

ClientModeINSNormal* GetClientModeINSNormal()
{
	Assert(dynamic_cast<ClientModeINSNormal*>(GetClientModeNormal()));
	return static_cast<ClientModeINSNormal*>(GetClientModeNormal());
}

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

	virtual IViewPortPanel* CreatePanelByName(const char* szPanelName)
	{
		return BaseClass::CreatePanelByName(szPanelName);
	}

	virtual void LevelInitPostEntity(void)
	{
		ResetAllPanels();
	}
};

CINSModeManager::CINSModeManager(void)
{
}

CINSModeManager::~CINSModeManager(void)
{
}

void CINSModeManager::Init(void)
{
	g_pClientMode = GetClientModeNormal();
	PanelMetaClassMgr()->LoadMetaClassDefinitionFile(SCREEN_FILE);

	// init VGUI
	GetINSVGUIHelper()->Init();

	// init HUD
	GetINSHUDHelper()->Init();

	// hint hints
	g_HintHelper.Init();
}

void CINSModeManager::SwitchMode(bool commander, bool force)
{
}

void CINSModeManager::OverrideView(CViewSetup* pSetup)
{
}

void CINSModeManager::CreateMove(float flInputSampleTime, CUserCmd* cmd)
{
}

void CINSModeManager::LevelInit(const char* newmap)
{
	g_pClientMode->LevelInit(newmap);
}

void CINSModeManager::LevelShutdown(void)
{
	g_pClientMode->LevelShutdown();
	GetINSHUDHelper()->LevelShutdown();
}

static CINSModeManager g_HLModeManager;
IVModeManager* modemanager = &g_HLModeManager;

ClientModeINSNormal::ClientModeINSNormal()
{
	m_pViewport = new CHudViewport();
	m_pViewport->Start(gameuifuncs, gameeventmanager);
}

ClientModeINSNormal::~ClientModeINSNormal()
{
}

void ClientModeINSNormal::Init()
{
	BaseClass::Init();
}