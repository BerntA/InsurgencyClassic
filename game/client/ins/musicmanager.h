// Insurgency Team (C) 2007
// First revision

#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

#ifdef _WIN32
#pragma once
#endif

#include <vector>
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

enum MusicOuputs_t
{
	MUSIC_OUTPUTTYPE_NOSOUND = 0,
	MUSIC_OUTPUTTYPE_DSOUND,
	MUSIC_OUTPUTTYPE_WINMM,
	MUSIC_OUTPUTTYPE_ASIO,
	MUSIC_OUTPUTTYPE_MAX,
};

// returns on error
#define FMODErrorCheck( eResult ) \
	if (_FMODErrorCheck( eResult, __LINE__ )) \
	return

// returns value on error
#define FMODErrorCheckV( eResult, vReturn ) \
	if (_FMODErrorCheck( eResult, __LINE__ )) \
	return vReturn

// Leaves critical section and returns on error
#define FMODErrorCheckLCS( eResult ) \
	if (_FMODErrorCheck( eResult, __LINE__ )) \
{	LeaveCS( ); return; }

// Leaves critical section and returns value on error
#define FMODErrorCheckVLCS( eResult, vReturn ) \
	if (_FMODErrorCheck( eResult, __LINE__)) \
{	LeaveCS( ); return vReturn; }

// only reports errors
#define FMODErrorCheckN( eResult ) \
	_FMODErrorCheck( eResult,__LINE__ )

class IMusicManager
{
	static IMusicManager* s_pSingleton;

public:
	IMusicManager(void)
	{
		if (s_pSingleton)
			return;

		s_pSingleton = this;
	}

	static IMusicManager& GetSingleton(void)
	{
		return *s_pSingleton;
	}

	static IMusicManager* GetSingletonPtr(void)
	{
		return s_pSingleton;
	}

	// Init/Shutdown stuff
	virtual bool Init(void) = 0;
	virtual void Restart(void) = 0;
	virtual void Shutdown(void) = 0;

	// Updates just check if it should add a music to song queue when player is in menu
	virtual void Update(float flFrameTime) = 0;

};

#endif // MUSIC_MANAGER_H