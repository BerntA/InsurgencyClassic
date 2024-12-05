//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: BrainBread: 2 Main Game Interface for BB2 Client Side
//
//========================================================================================//

#ifndef GAME_BASE_CLIENT_H
#define GAME_BASE_CLIENT_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include "c_hl2mp_player.h"
#include <steam/steam_api.h>
#include "steam/isteamapps.h"

extern ConVar bb2_render_client_in_mirrors;
extern ConVar bb2_enable_multicore;

typedef struct IVoiceTweak_s IVoiceTweak;

class IGameBaseClient
{
public:

	virtual void        Initialize(void) = 0;
	virtual void        CreateInGamePanels(vgui::VPANEL parent) = 0;
	virtual void		Destroy(void) = 0;
	virtual void		PostInit(void) = 0;

	// Shared
	virtual void LoadGameLocalization(void) = 0;
	virtual void OnUpdate(void) = 0;
	virtual void CloseGamePanels(bool bInGamePanelsOnly = false) = 0;
	virtual bool IsInGame(void) = 0;
	virtual void Changelevel(const char* szMap) = 0;

	// In-Game Panel Accessors
	virtual void ShowVotePanel(bool bForceOff = false) = 0;
	virtual bool IsViewPortPanelVisible(const char* panel) = 0;

	// In-Game 
	virtual void OnLocalPlayerExternalRendering(void) = 0;
	virtual bool IsMainMenuVisibleWhileInGame(void) = 0;
	virtual bool IsExtremeGore(void) = 0;

	// Voice
	virtual bool IsPlayerGameVoiceMuted(int playerIndex) = 0;
	virtual void MutePlayerGameVoice(int playerIndex, bool value) = 0;
	virtual IVoiceTweak* GetVoiceAPI(void) = 0;
};

extern IGameBaseClient* GameBaseClient;

#endif // GAME_BASE_CLIENT_H