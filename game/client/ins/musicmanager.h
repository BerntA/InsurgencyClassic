// Insurgency Team (C) 2007
// First revision

#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

#ifdef _WIN32
#pragma once
#endif

#include "stringlookup.h"

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

#endif // MUSIC_MANAGER_H