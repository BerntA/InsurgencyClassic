//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: BrainBread 2 Server Handler - Handles kicking, voting, baning, map change, etc...
//
//========================================================================================//

#ifndef GAME_BASE_SERVER_H
#define GAME_BASE_SERVER_H

#ifdef _WIN32
#pragma once
#endif

class CGameBaseServer
{
public:

	void Init();
	void Release();
	void PostInit();
	void PostLoad(float flDelay) { m_flPostLoadTimer = engine->Time() + flDelay; }
	void SetCurrentMapAddon(const char* map);
	void OnUpdate(int iClientsInGame);

private:

	float m_flPostLoadTimer;
};

extern CGameBaseServer* GameBaseServer();

#endif // GAME_BASE_SERVER_H