//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: BrainBread: 2 Main Game Interface for BB2 Client Side
//
//========================================================================================//

#include "cbase.h"
#include "GameBase_Client.h"
#include "vgui_controls/Frame.h"
#include "weapon_parse.h"
#include "ienginevgui.h"
#include <GameUI/IGameUI.h>
#include "c_ins_player.h"

// ADD INCLUDES FOR OTHER MENUS: (NON-BASEVIEWPORT/INTERFACE)
#include "AddonInstallerPanel.h"
#include "fmod_manager.h"
#include "ivoicetweak.h"

// Other helpers
#include "clientmode_shared.h"
#include "tier0/icommandline.h"
#include "c_leaderboard_handler.h"
#include "GameBase_Shared.h"
#include "iviewrender.h"
#include "view.h"
#include "c_playerresource.h"
#include "voice_status.h"
#include "GlobalRenderEffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void OnUpdateMirrorRenderingState(IConVar* pConVar, char const* pOldString, float flOldValue)
{
	C_BasePlayer* pLocal = C_BasePlayer::GetLocalPlayer();
	if (!pLocal)
		return;

	pLocal->UpdateVisibility();

	C_BaseCombatWeapon* pLocalWeapon = pLocal->GetActiveWeapon();
	if (!pLocalWeapon)
		return;

	pLocalWeapon->UpdateVisibility();
}

void OnUpdateMulticoreState(IConVar* pConVar, char const* pOldString, float flOldValue)
{
	ConVar* var = (ConVar*)pConVar;
	if (var && var->GetBool())
		engine->ClientCmd_Unrestricted("exec multicore.cfg\n");
	else
		engine->ClientCmd_Unrestricted("mat_queue_mode 0\n");
}

ConVar bb2_render_client_in_mirrors("bb2_render_client_in_mirrors", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Render the player in mirrors.", true, 0, true, 1, OnUpdateMirrorRenderingState);
ConVar bb2_enable_multicore("bb2_enable_multicore", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Enable or Disable multicore rendering, this feature is unstable but could increase performance!", true, 0, true, 1, OnUpdateMulticoreState);

static void VoiceThresholdChange(IConVar* pConVar, char const* pOldString, float flOldValue)
{
	ConVar* var = (ConVar*)pConVar;
	if (GameBaseClient->GetVoiceAPI() && var)
		GameBaseClient->GetVoiceAPI()->SetControlFloat(MicrophoneVolume, var->GetFloat());
}

static void VoiceBoostChange(IConVar* pConVar, char const* pOldString, float flOldValue)
{
	ConVar* var = (ConVar*)pConVar;
	if (GameBaseClient->GetVoiceAPI() && var)
		GameBaseClient->GetVoiceAPI()->SetControlFloat(MicBoost, (var->GetBool() ? 1.0f : 0.0f));
}

static ConVar voice_threshold("voice_threshold", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Voice Send Volume", true, 0.0f, true, 1.0f, VoiceThresholdChange);
static ConVar voice_boost("voice_boost", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Voice Boost Gain", true, 0.0f, true, 1.0f, VoiceBoostChange);

float m_flLastBloodParticleDispatchTime = 0.0f;
static bool g_bHasLoadedSteamStats = false;

// GameUI
static CDllDemandLoader g_GameUIDLL("GameUI");

class CGameBaseClient : public IGameBaseClient
{
private:

	CAddonInstallerPanel* ClientWorkshopInstallerPanel;
	IGameUI* GameUI;
	IVoiceTweak* m_pVoiceTweak;		// Engine voice tweak API.
	bool m_bIsMenuVisibleAndInGame;

	STEAM_CALLBACK(CGameBaseClient, Steam_OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived);

public:

	CGameBaseClient(void);

	// Initialize, called when the game launches. (only once)
	void Initialize(void);
	// Init/Create in-game panels.
	void CreateInGamePanels(vgui::VPANEL parent);
	// Cleanup
	void Destroy(void);

	// Are we in-game?
	bool IsInGame(void);

	// Post Init - Late Init - Starts up the main menu.
	void PostInit(void);

	// Handle per-frame thinking...
	void OnUpdate(void);

	// In-Game Stuff
	void OnLocalPlayerExternalRendering(void);
	bool IsMainMenuVisibleWhileInGame(void);
	bool IsExtremeGore(void) { return false; } // TODO

	// Voice
	bool IsPlayerGameVoiceMuted(int playerIndex);
	void MutePlayerGameVoice(int playerIndex, bool value);
	IVoiceTweak* GetVoiceAPI(void) { return m_pVoiceTweak; }
};

static CGameBaseClient g_GameBaseClient;
IGameBaseClient* GameBaseClient = (IGameBaseClient*)&g_GameBaseClient;

CGameBaseClient::CGameBaseClient(void) : m_CallbackUserStatsReceived(this, &CGameBaseClient::Steam_OnUserStatsReceived)
{
	ClientWorkshopInstallerPanel = NULL;
	GameUI = NULL;
	m_bIsMenuVisibleAndInGame = false;
}

// Early Initialization
void CGameBaseClient::Initialize(void)
{
	// Init Game UI
	CreateInterfaceFn gameUIFactory = g_GameUIDLL.GetFactory();
	if (gameUIFactory)
	{
		GameUI = (IGameUI*)gameUIFactory(GAMEUI_INTERFACE_VERSION, NULL);
		if (!GameUI)
			Error("Couldn't load GameUI!\n");
	}
	else
		Error("Couldn't load GameUI!\n");

	// Create our MainMenu
	//MainMenu = new CMainMenu(NULL);
	//GameUI->SetMainMenuOverride(MainMenu->GetVPanel());

	// Create our Loading Panel
	//LoadingPanel = new CLoadingPanel(NULL);
	//LoadingPanel->SetVisible(false);
	//GameUI->SetLoadingBackgroundDialog(LoadingPanel->GetVPanel());

	VPANEL GameUiDll = enginevgui->GetPanel(PANEL_GAMEUIDLL);
	ClientWorkshopInstallerPanel = new CAddonInstallerPanel(GameUiDll);
	ClientWorkshopInstallerPanel->SetVisible(false);

	if (steamapicontext && steamapicontext->SteamRemoteStorage())
		steamapicontext->SteamRemoteStorage()->SetCloudEnabledForApp(false); // enable?

	PostInit();
}

// Generate the in-game panels.
void CGameBaseClient::CreateInGamePanels(vgui::VPANEL parent)
{
}

// Cleanup - called in vgui_int.cpp
void CGameBaseClient::Destroy(void)
{
	g_GameUIDLL.Unload();
	GameUI = NULL;

	//if (LoadingPanel)
	//{
	//	LoadingPanel->SetParent((vgui::Panel *)NULL);
	//	delete LoadingPanel;
	//}

	if (ClientWorkshopInstallerPanel)
	{
		ClientWorkshopInstallerPanel->SetParent((vgui::Panel*)NULL);
		delete ClientWorkshopInstallerPanel;
	}

	GlobalRenderEffects->Shutdown();
}

// Are we in-game?
bool CGameBaseClient::IsInGame(void)
{
	C_BasePlayer* pClient = C_BasePlayer::GetLocalPlayer();
	return ((pClient != NULL) && !engine->IsLevelMainMenuBackground());
}

// Post Init - Late Init : Loads the main menu
void CGameBaseClient::PostInit(void)
{
	m_bIsMenuVisibleAndInGame = false;

	//	if (MainMenu)
	//	MainMenu->ActivateMainMenu();

	if (bb2_enable_multicore.GetBool())
		engine->ClientCmd_Unrestricted("exec multicore.cfg\n");
	else
		engine->ClientCmd_Unrestricted("mat_queue_mode 0\n");

	m_pVoiceTweak = engine->GetVoiceTweakAPI();
	if (m_pVoiceTweak)
	{
		m_pVoiceTweak->SetControlFloat(MicBoost, (voice_boost.GetFloat() >= 1.0f) ? 1.0f : 0.0f);
		m_pVoiceTweak->SetControlFloat(MicrophoneVolume, voice_threshold.GetFloat());
		DevMsg("Initialized voice api\n");
	}

	GlobalRenderEffects->Initialize();

	if (steamapicontext && steamapicontext->SteamUserStats())
		steamapicontext->SteamUserStats()->RequestCurrentStats();
}

void CGameBaseClient::OnUpdate(void)
{
	CLeaderboardHandler::Update();

	C_BasePlayer* pClient = C_BasePlayer::GetLocalPlayer();
	if (!pClient)
	{
		m_bIsMenuVisibleAndInGame = false;
		return;
	}

	if (!engine->IsLevelMainMenuBackground() && engine->IsInGame())
	{
		if (!m_bIsMenuVisibleAndInGame)
		{
			if (enginevgui->IsGameUIVisible())
				m_bIsMenuVisibleAndInGame = true;
		}
		else if (m_bIsMenuVisibleAndInGame)
		{
			if (!enginevgui->IsGameUIVisible())
				m_bIsMenuVisibleAndInGame = false;
		}
	}
}

void CGameBaseClient::OnLocalPlayerExternalRendering(void)
{
	C_BasePlayer* pClient = C_BasePlayer::GetLocalPlayer();
	if (!pClient)
		return;

	if (bb2_render_client_in_mirrors.GetBool())
	{
		pClient->ThirdPersonSwitch(g_bShouldRenderLocalPlayerExternally);
		C_BaseCombatWeapon* pWeapon = pClient->GetActiveWeapon();
		if (pWeapon)
			pWeapon->UpdateVisibility();
	}
}

// Is the main menu up while in-game?
bool CGameBaseClient::IsMainMenuVisibleWhileInGame(void)
{
	return m_bIsMenuVisibleAndInGame;
}

bool CGameBaseClient::IsPlayerGameVoiceMuted(int playerIndex)
{
	if (!engine->IsInGame())
		return false;

	return GetClientVoiceMgr()->IsPlayerBlocked(playerIndex);
}

void CGameBaseClient::MutePlayerGameVoice(int playerIndex, bool value)
{
	if (!engine->IsInGame())
		return;

	GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, value);
}

void CGameBaseClient::Steam_OnUserStatsReceived(UserStatsReceived_t* pUserStatsReceived)
{
	if (g_bHasLoadedSteamStats || !steamapicontext || !steamapicontext->SteamUserStats())
		return;

	DevMsg("Load SteamStats: EResult %d\n", pUserStatsReceived->m_eResult);

	if (pUserStatsReceived->m_eResult != k_EResultOK)
		return;

	IGameEvent* event = gameeventmanager->CreateEvent("user_data_downloaded");
	if (event)
		gameeventmanager->FireEventClientSide(event);

	// Update achievement panel!
	// TODO / INITIAL RENDER

	CLeaderboardHandler::InitHandle();
	g_bHasLoadedSteamStats = true;
}