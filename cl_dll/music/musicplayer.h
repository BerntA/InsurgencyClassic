// Insurgency Team (C) 2007
// First revision

#pragma once

/*
#include <stdarg.h>
#include "musicglobal.h"
#include "tier0/vcrmode.h"
#include "fmod/fmod.h"
#include "fmod/fmod_errors.h"

//=========================================================
//=========================================================
class CMusicPlayer : public IMusicPlayer
{
public:
	CMusicPlayer( );
	~CMusicPlayer( );

	void Init( mplayer_init_data_t* pInitData );
	void Shutdown( void );
	bool Update( void );

	void Restart( mplayer_init_data_t* pInitData );
	void Terminate( void );
	bool Initialized( void ) { return m_bInitialized; }

	bool GetStatus( mplayer_status_t* pStatus );
	void SetVolume( float flVolume );

	int PlaylistEnqueued( void );
	void PlaylistClear( bool bFadeOut = false );
	void PlaylistAdd( int iSound );

	int GetDrivers( mplayer_driver_t* pDrivers, int iCount );

private:
	void EnterCS( void );
	void LeaveCS( void );

	void FMODWarning(const char* pszFormat, int iLine, ...);
	void _Warning(const char* pszWarning);
	bool _FMODErrorCheck(FMOD_RESULT eResult,int iLine);

private:
	bool					m_bInitialized;
	bool					m_bRestart;
	bool					m_bShutdown;
	bool					m_bUpdateStatus;

	FMOD::System*			m_pSystem;
	int						m_iSounds;
	FMOD::Sound**			m_pSounds;

	IMusicController*		m_pController;
	CRITICAL_SECTION		m_pCrititicalSection;
	mplayer_status_t		m_Status;
	mplayer_init_data_t		m_InitData;

	int						m_aiPlaylist[16];
	float					m_flVolume,m_flFadeVolume;
	bool					m_bFadeOut;
};
*/

class IMusicPlayer
{
public:
	IMusicPlayer( void ) { }

	// Init/Shutdown stuff
	virtual bool Init( void ) = 0;
	virtual void Shutdown( void ) = 0;
	virtual bool Update( void ) = 0;

	// Load a song, add it to a list, can access it with the index
	virtual bool LoadSong( const char *pszSong ) = 0;

	// Music queue management
	virtual void AddMusicToQueue( int iSongIdx ) = 0;
	virtual void ClearMusicQueue( bool bFadeOut = false ) = 0;

	// Settings
	virtual void SetVolume( float flVolume ) = 0;
	virtual void SetOutput( int iOutput ) = 0;
	virtual void SetDevice( int iDevice ) = 0;

	virtual bool Initialized( void ) const = 0;
	virtual bool IsPlaying( void ) = 0;
};

IMusicPlayer *CreateMusicPlayer( void );