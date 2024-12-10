// Insurgency Team (C) 2007
// First revision

#define PROTECTED_THINGS_H
#include "cbase.h"
#include <Windows.h>
#undef PROTECTED_THINGS_H
#include "MusicManager.h"
#include "musicplayer.h"

#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"

using namespace std;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
// CMusicPlayer
//=========================================================
class CMusicPlayer : public IMusicPlayer
{
public:
	CMusicPlayer( void );
	~CMusicPlayer( void );

	// Init/Shutdown stuff
	virtual bool Init( void );
	virtual void Shutdown( void );
	virtual bool Update( void );

	// Load a song, add it to a list, can access it with the index
	virtual bool LoadSong( const char *pszSong );
	
	// Music queue management
	virtual void AddMusicToQueue( int iSongIdx );
	virtual void ClearMusicQueue( bool bFadeOut = false );

	// Settings
	virtual void SetVolume( float flVolume );
	virtual void SetOutput( int iOutput );
	virtual void SetDevice( int iDevice );

	virtual bool Initialized( void ) const { return m_bInitialized; }

	virtual bool IsPlaying( void );

private:
	void EnterCS( void );
	void LeaveCS( void );

	bool _FMODErrorCheck( FMOD_RESULT eResult, int iLine );

private:
	bool					m_bInitialized;

	FMOD::System			*m_pSystem;
	FMOD::Channel			*m_pChannel;
	CRITICAL_SECTION		m_pCrititicalSection;

	int m_iOutput;
	int m_iDevice;

	typedef vector< FMOD::Sound * > SongList;
	SongList m_songList;

	typedef vector< int > SongQueue;
	SongQueue m_songsToBePlayed;
};

//=========================================================
// CreateMusicPlayer
//=========================================================
IMusicPlayer *CreateMusicPlayer( void )
{
	return new CMusicPlayer( );
}

//=========================================================
//=========================================================
CMusicPlayer::CMusicPlayer( void )
{
	m_pChannel	= NULL;
	m_pSystem	= NULL;

	m_bInitialized = false;

	m_iOutput = (int)FMOD_OUTPUTTYPE_DSOUND;
	m_iDevice = 0;
}

//=========================================================
//=========================================================
CMusicPlayer::~CMusicPlayer( void )
{
	
}

//=========================================================
//=========================================================
bool CMusicPlayer::Init( void )
{
	if(m_bInitialized)
		return false; // Should never go there

	static bool bBinaryLoaded = false;

	// Just load binaries one time !!!! HACK UGLY
	if(!bBinaryLoaded) {
		char szFMODPath[MAX_PATH];
		Q_snprintf( szFMODPath, MAX_PATH, "%s/bin/", engine->GetGameDirectory( ) );

		__DLInitDelayLoadDLL( __DLNoFail );
		__DLSetSearchPath( szFMODPath );
		__DLLoadDelayLoadDLL( "fmodex.dll" );
		__DLFinishDelayLoadDLL;

		bBinaryLoaded = true;
	}

	FMODErrorCheckV( System_Create( &m_pSystem ), false );

	unsigned int iVersion = 0;

	FMODErrorCheckV( m_pSystem->getVersion( &iVersion ), false );

	if (iVersion < FMOD_VERSION)
	{
		Error( "You are using an old version of FMOD %08x. This program requires %08x\n", iVersion, FMOD_VERSION );
		return false;
	}

	FMODErrorCheckV( m_pSystem->setOutput( (FMOD_OUTPUTTYPE)m_iOutput ), false );

	FMODErrorCheckV( m_pSystem->setDriver( m_iDevice ), false );

	FMODErrorCheckV( m_pSystem->init( 1, FMOD_INIT_NORMAL, 0 ), false );

	FMODErrorCheckV( m_pSystem->getChannel( 0, &m_pChannel ), false );

	m_bInitialized = true;

	return true;
}

//=========================================================
//=========================================================
bool CMusicPlayer::LoadSong( const char *pszSong )
{
	if(!m_bInitialized)
		return false;

	FMOD::Sound *pSong = NULL;

	// It'll return false if it can't load it.
	FMODErrorCheckV(
		m_pSystem->createStream( pszSong, FMOD_HARDWARE | FMOD_LOOP_OFF | FMOD_2D, NULL, &pSong ),
		false );

	m_songList.push_back( pSong );

	return true;
}

//=========================================================
//=========================================================
void CMusicPlayer::EnterCS( void )
{
	g_pVCR->Hook_EnterCriticalSection( &m_pCrititicalSection );
}

//=========================================================
//=========================================================
void CMusicPlayer::LeaveCS( void )
{
	LeaveCriticalSection( &m_pCrititicalSection );
}

//=========================================================
//=========================================================
void CMusicPlayer::Shutdown( void )
{
	if(!m_bInitialized)
		return;

	m_bInitialized = false;

	// Clear all lists, and free all songs
	SongList::iterator it = m_songList.begin( );
	while( it != m_songList.end( ) )
	{
		(*it)->release( );
		++it;
	}
	
	m_songList.clear( );

	ClearMusicQueue( );

	m_pSystem->close( );

	FMODErrorCheckN( m_pSystem->release( ) );

	m_pChannel	= NULL;
	m_pSystem	= NULL;

	printf( "Shutting down\n" );
}

//=========================================================
//=========================================================
bool CMusicPlayer::Update( void )
{
	if(!m_bInitialized)
		return false;

	FMODErrorCheckV( m_pSystem->update( ), true );

	// Do we have any other songs to play?
	if(!IsPlaying( ) && m_songsToBePlayed.size( ))
	{
		FMODErrorCheckN( m_pSystem->playSound( (FMOD_CHANNELINDEX)0, m_songList[m_songsToBePlayed[0]] ,false,&m_pChannel));

		m_songsToBePlayed.erase( m_songsToBePlayed.begin( ) );
	}

	return true;
}

//=========================================================
//=========================================================
bool CMusicPlayer::_FMODErrorCheck( FMOD_RESULT eResult, int iLine )
{
	if (eResult != FMOD_OK )
	{
		Warning( "FMOD: On line #%d, error #%d: %s\n", iLine, eResult, FMOD_ErrorString( eResult ) );
		return true;
	}

	return false;
}

//=========================================================
//=========================================================
void CMusicPlayer::AddMusicToQueue( int iSongIdx )
{
	Assert( iSongIdx >= 0 && iSongIdx < (int)m_songList.size( ) );

	m_songsToBePlayed.push_back( iSongIdx );
}

//=========================================================
//=========================================================
void CMusicPlayer::ClearMusicQueue( bool bFadeOut /* = false */ )
{
	m_songsToBePlayed.clear( );
	
	// Stop the current playing song if we are playing one
	if(IsPlaying( ))
	{
		m_pChannel->stop( );
	}
}

//=========================================================
//=========================================================
bool CMusicPlayer::IsPlaying( void )
{
	if(!m_bInitialized)
		return false;

	bool bIsPlaying = false;
	m_pChannel->isPlaying( &bIsPlaying );

	return bIsPlaying;
}
//=========================================================
//=========================================================
void CMusicPlayer::SetVolume( float flVolume )
{
	if (m_bInitialized && m_pChannel)
	{
		m_pChannel->setVolume( flVolume );
	}
}

//=========================================================
//=========================================================
void CMusicPlayer::SetOutput( int iOutput )
{
	m_iOutput = iOutput;

	IMusicManager::GetSingleton( ).Restart( );
}

//=========================================================
//=========================================================
void CMusicPlayer::SetDevice( int iDevice )
{
	m_iDevice = iDevice;

	IMusicManager::GetSingleton( ).Restart( );
}
