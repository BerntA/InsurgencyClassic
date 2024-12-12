// Insurgency Team (C) 2007
// First revision

#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

#ifdef _WIN32
#pragma once
#endif

#include "stringlookup.h"
#include "fmod_ambience.h"

enum MusicIngame_t
{
	MUSIC_INGAME_OFF = 0,
	MUSIC_INGAME_LIMITED,
	MUSIC_INGAME_FULL,
};

enum MusicModes_t
{
	MUSIC_MODE_OFF = 0,
	MUSIC_MODE_MENU,
	MUSIC_MODE_PREPARATION,
	MUSIC_MODE_COMBAT,
	MUSIC_MODE_DEATH,
	MUSIC_MODE_VICTORY,
	MUSIC_MODE_DEFEAT,

	MUSIC_MODE_MAX,
};

enum
{
	MUSIC_GROUP_USMC = 0,
	MUSIC_GROUP_IRAQI,
	MUSIC_GROUP_MENU,

	MUSIC_GROUP_MAX
};

struct MusicEntry_t
{
	int type;
	char pchSoundFile[MAX_PATH];
};

class CSoundGroup
{
public:
	CSoundGroup()
	{
		szName[0] = 0;
		iTeam = 0;
		Clear();
	}

	virtual ~CSoundGroup()
	{
		Clear();
	}

	void Clear()
	{
		sounds.Purge();
	}

	const MusicEntry_t* GetRandomSongForMode(int mode) const;

	char szName[32];
	int iTeam; // 0 = usmc, 1 = iraqi, 2 = Menu
	CUtlVector<MusicEntry_t> sounds;
};

class CMusicManager : public CGameEventListener, public CAutoGameSystem
{
public:
	CMusicManager();
	virtual ~CMusicManager();

	virtual bool Init(void) OVERRIDE;
	virtual void Restart(void);
	virtual void Shutdown(void) OVERRIDE;

	virtual void Update(float flFrameTime) OVERRIDE;

	virtual void FireGameEvent(IGameEvent* pEvent) OVERRIDE;

	virtual void LevelInitPostEntity(void) OVERRIDE;
	virtual void LevelShutdownPreEntity(void) OVERRIDE;

	void SetMusicIngame(MusicIngame_t iState);
	void SendMusic(int iMode);
	void ClearMusicQueue(void);

protected:
	bool LoadData(void);

private:
	MusicIngame_t m_iIngameMusicState;
	CFMODAmbience m_fmodSound;
	CSoundGroup m_soundGroups[MUSIC_GROUP_MAX]; // 0 = usmc, 1 = iraqi, 2 = Menu
	int m_iCurrentSoundGroup;
};

extern CMusicManager* g_pMusicManager;

#endif // MUSIC_MANAGER_H