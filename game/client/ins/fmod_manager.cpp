//=========       Copyright © Reperio Studios 2025 @ Bernt Andreas Eide!       ============//
//
// Purpose: FMOD - Sound System
//
//========================================================================================//

#include "cbase.h"
#include "fmod/fmod.hpp"
#include "fmod_manager.h"
#include "filesystem.h"
#include "GameBase_Client.h"
#include "c_soundscape.h"
#include "musicmanager.h"

using namespace FMOD;

static System* pSystem;
static CFMODManager gFMODMng;

CFMODManager* FMODManager() { return &gFMODMng; }

CFMODManager::CFMODManager() {}
CFMODManager::~CFMODManager() {}

void CFMODManager::Init(void)
{
	if (System_Create(&pSystem) != FMOD_OK)
		Warning("FMOD ERROR: System creation failed!\n");
	else
		DevMsg("FMOD system successfully created.\n");

	if (pSystem->init(MAX_FMOD_CHANNELS, FMOD_INIT_NORMAL, 0) != FMOD_OK)
		Warning("FMOD ERROR: Failed to initialize properly!\n");
	else
		DevMsg("FMOD initialized successfully.\n");

	m_pVarMusicVolume = cvar->FindVar("snd_musicvolume");
	m_pVarGameVolume = cvar->FindVar("volume");
	m_pVarMuteSoundFocus = cvar->FindVar("snd_mute_losefocus");
}

void CFMODManager::Exit(void)
{
	if (pSystem->release() != FMOD_OK)
		Warning("FMOD ERROR: System did not terminate properly!\n");
	else
		DevMsg("FMOD system terminated successfully.\n");

	pSystem = NULL;
}

void CFMODManager::Restart()
{
	Soundscape_StopFMODSounds();
	Exit();
	Init();
	Soundscape_RestartFMODSounds();
}

const char* CFMODManager::GetSoundPath(const char* pathToFileFromModFolder)
{
	static char fullPath[512];
	static char relativePath[256];

	Q_snprintf(relativePath, sizeof(relativePath), "sound/%s", pathToFileFromModFolder);
	filesystem->RelativePathToFullPath(relativePath, "MOD", fullPath, sizeof(fullPath));

	// convert backwards slashes to forward slashes
	const int iLength = strlen(fullPath);
	for (int i = 0; i < iLength; i++)
	{
		if (fullPath[i] == '\\')
			fullPath[i] = '/';
	}

	return fullPath;
}

void CFMODManager::Think(void)
{
	if (pSystem == NULL)
		return;

	pSystem->update();
	g_pMusicManager->Update(0.0f);
}

System* CFMODManager::GetFMODSystem()
{
	return pSystem;
}