//=========       Copyright © Reperio Studios 2025 @ Bernt Andreas Eide!       ============//
//
// Purpose: FMOD - Sound System
//
//========================================================================================//

#ifndef FMOD_MANAGER_H
#define FMOD_MANAGER_H

#ifdef _WIN32
#pragma once
#endif

#define MAX_FMOD_CHANNELS 32

namespace FMOD
{
	class System;
}

class CFMODManager
{
public:
	CFMODManager();
	~CFMODManager();

	void Init();
	void Exit();
	void Restart();
	void Think();

	FMOD::System* GetFMODSystem();

	const char* GetSoundPath(const char* pathToFileFromModFolder);

	const float GetMusicVolume() const { return (m_pVarMusicVolume ? m_pVarMusicVolume->GetFloat() : 1.0f); }
	const float GetGameVolume() const { return (m_pVarGameVolume ? m_pVarGameVolume->GetFloat() : 1.0f); }

private:
	ConVar* m_pVarMusicVolume;
	ConVar* m_pVarGameVolume;
	ConVar* m_pVarMuteSoundFocus;
};

extern CFMODManager* FMODManager();

#endif //FMOD_MANAGER_H