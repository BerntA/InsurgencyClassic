// Insurgency Team (C) 2007
// First revision

#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#ifdef _WIN32
#pragma once
#endif

class IMusicPlayer
{
public:
	IMusicPlayer(void) {}

	// Init/Shutdown stuff
	virtual bool Init(void) = 0;
	virtual void Shutdown(void) = 0;
	virtual bool Update(void) = 0;

	// Load a song, add it to a list, can access it with the index
	virtual bool LoadSong(const char* pszSong) = 0;

	// Music queue management
	virtual void AddMusicToQueue(int iSongIdx) = 0;
	virtual void ClearMusicQueue(bool bFadeOut = false) = 0;

	// Settings
	virtual void SetVolume(float flVolume) = 0;
	virtual void SetOutput(int iOutput) = 0;
	virtual void SetDevice(int iDevice) = 0;

	virtual bool Initialized(void) const = 0;
	virtual bool IsPlaying(void) = 0;
};

IMusicPlayer* CreateMusicPlayer(void);

#endif // MUSIC_PLAYER_H