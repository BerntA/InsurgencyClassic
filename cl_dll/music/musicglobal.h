#ifndef MUSICGLOBAL_H
#define MUSICGLOBAL_H

#include "stringlookup.h"

namespace FMOD
{
	class System;
	class Sound;
	class Channel;
	class ChannelGroup;
	class DSP;
	class Geometry;
};

class IMusicController;
class IMusicPlayer;

enum music_mode_t {
	MUSIC_MODE_OFF=0,
	MUSIC_MODE_MENU,
	MUSIC_MODE_PREPARATION,
	MUSIC_MODE_COMBAT,
	MUSIC_MODE_DEATH,
	MUSIC_MODE_VICTORY,
	MUSIC_MODE_DEFEAT,
	MUSIC_MODE_MAX,
};

enum music_output_t {
	MUSIC_OUTPUTTYPE_NOSOUND=0,
	MUSIC_OUTPUTTYPE_DSOUND,
	MUSIC_OUTPUTTYPE_WINMM,
	MUSIC_OUTPUTTYPE_ASIO,
	MUSIC_OUTPUTTYPE_MAX,
};

typedef struct mplayer_driver_s {
	int iID;
	char szName[256];
} mplayer_driver_t;

typedef struct mplayer_init_data_s {
	IMusicController* pController;
	int iOutput;
	int iDriver;
	int iSounds;
	char szSearchPath[MAX_PATH];
	const char** pszSounds;
} mplayer_init_data_t;

typedef struct mplayer_status_s {
	int iSound;
	char szSoundName[256];
	//both in ms
	unsigned int iLength;
	unsigned int iPosition;
} mplayer_status_t;

class IMusicPlayer
{
public:
	virtual void Init(mplayer_init_data_t* pInitData) = 0;
	virtual void Shutdown() = 0;

	virtual bool GetStatus(mplayer_status_t* pStatus) = 0;

	virtual void SetVolume(float flVolume) = 0;

	virtual int PlaylistEnqueued() = 0;
	virtual void PlaylistClear(bool bFadeOut=false) = 0;
	virtual void PlaylistAdd(int iSound) = 0;

	virtual void Restart(mplayer_init_data_t* pInitData) = 0;
	virtual void Terminate() = 0;
	virtual bool Initialized() = 0;
	virtual bool Update() = 0;

	//fills pDrivers with max iCount driver information structs
	virtual int GetDrivers(mplayer_driver_t* pDrivers, int iCount) = 0;
};

class IMusicController
{
public:
	virtual void SetPlayer(IMusicPlayer* pPlayer) = 0;

	virtual void PlayerWarning(const char* pszWarning) = 0;
};

//Send terminate to the player to delete the instance
//and terminate the thread
void CreateMusicPlayer(mplayer_init_data_t* pInitData);

DECLARE_STRING_LOOKUP_CONSTANTS(int,MUSIC_MODE);

//idea: you set it a bit, when its unset, a full call of IMusicPlayer::Update
//has been run
//register here any used bits
//bit 0 in use!
extern unsigned int g_iMusicUpdateBits;

extern bool g_bMusicPlayerInitialized;

#endif //MUSICGLOBAL_H