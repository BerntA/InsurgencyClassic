// Insurgency Team (C) 2007
// First revision

#define PROTECTED_THINGS_H
#include "cbase.h"
#include <Windows.h>
#undef PROTECTED_THINGS_H
#include "protected_things.h"
#include "MusicManager.h"
#include "musicplayer.h"

#include "convar.h"
#include "keyvalues.h"
#include "filesystem.h"
#include "clientmode_shared.h"
#include "c_ins_player.h"

#include "c_team.h"
#include "c_play_team.h"

#include "ins_gamerules.h"
#include "play_team_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMusicManager *IMusicManager::s_pSingleton = NULL;

//=========================================================
//=========================================================
DECLARE_STRING_LOOKUP_CONSTANTS( int, MUSIC_MODE );

DEFINE_STRING_LOOKUP_CONSTANTS( int, MUSIC_MODE )

	ADD_LOOKUP(MUSIC_MODE_OFF)
	ADD_LOOKUP(MUSIC_MODE_MENU)
	ADD_LOOKUP(MUSIC_MODE_PREPARATION)
	ADD_LOOKUP(MUSIC_MODE_COMBAT)
	ADD_LOOKUP(MUSIC_MODE_DEATH)
	ADD_LOOKUP(MUSIC_MODE_VICTORY)
	ADD_LOOKUP(MUSIC_MODE_DEFEAT)

END_STRING_LOOKUP_CONSTANTS()


bool g_bMusicPlayerInitialized;

//=========================================================
// CMusicManager
//=========================================================
DWORD WINAPI MusicPlayerThread( IMusicPlayer *pPlayer )
{
	if(!pPlayer)
		return FALSE;

	g_bMusicPlayerInitialized = pPlayer->Initialized( );

	while( pPlayer->Update( ) )
	{
		// lets sleep a bit so the cpu doesnt get owned
		Sleep( 5 );
	}
	
	return TRUE;
}

//=========================================================
// CMusicManager
//=========================================================
class CMusicManager : public IMusicManager, public IGameEventListener2, public CAutoGameSystem
{
public:
	CMusicManager( void );
	virtual ~CMusicManager( void );

	// Init/Shutdown stuff
	virtual bool Init( void );
	virtual void Restart( void );
	virtual void Shutdown( void );

	// Updates just check if it should add a music to song queue when player is in menu
	virtual void Update( float flFrameTime );

	virtual void FireGameEvent( IGameEvent *pEvent );

	virtual void LevelShutdownPreEntity( void );
	virtual void LevelInitPostEntity( void );

	// Settings
	void SetVolume( float flVolume );
	void SetOutput( int iOutput );
	void SetDevice( int iDevice );
	void SetMusicIngame( MusicIngame_t iState );

	static CMusicManager &GetSingleton( )
	{
		return *dynamic_cast< CMusicManager * >(GetSingletonPtr( ));
	}

private:
	IMusicPlayer *m_pPlayer;
	DWORD		  m_dwPlayerThread;

	MusicIngame_t m_iIngameMusicState;

	void ClearMusicLists( void );
	virtual bool LoadData( void );

	int m_iCurrentSoundGroup;

	void SendMusic( MusicModes_t iMode ); 

	struct Sound_t
	{
		int iSoundType;
		int iSoundIdx;
	};

	typedef vector< Sound_t > SoundList;

	struct SoundGroup_t
	{
		SoundList sounds[MUSIC_MODE_MAX];
		char szName[32];
		int iTeam; // 0 = usmc, 1 = iraqi, 3 = Menu

	};

	SoundGroup_t m_soundGroups[3]; // 0 = usmc, 1 = iraqi, 3 = Menu
};

//=========================================================
//=========================================================
CMusicManager *g_pMusicManagerInstance = new CMusicManager( );

//=========================================================
//=========================================================
void MusicIngameCallback(IConVar* pConVar, char const* pOldString, float flOldValue)
{
	ConVar* var = (ConVar*)pConVar;
	CMusicManager::GetSingleton( ).SetMusicIngame( (MusicIngame_t)var->GetInt( ) );
}

//=========================================================
//=========================================================
void MusicVolumeCallback(IConVar* pConVar, char const* pOldString, float flOldValue)
{
	ConVar* var = (ConVar*)pConVar;
	CMusicManager::GetSingleton( ).SetVolume( var->GetFloat( ) );
}

//=========================================================
//=========================================================
void MusicOuputCallback(IConVar* pConVar, char const* pOldString, float flOldValue)
{
	ConVar* var = (ConVar*)pConVar;
	CMusicManager::GetSingleton( ).SetOutput( var->GetFloat( ) );
}

//=========================================================
//=========================================================
void MusicDeviceCallback(IConVar* pConVar, char const* pOldString, float flOldValue)
{
	ConVar* var = (ConVar*)pConVar;
	CMusicManager::GetSingleton( ).SetDevice( var->GetInt( ) );
}

//=========================================================
// Music Variables
//=========================================================
ConVar snd_music_volume( "snd_music_volume", "1", FCVAR_ARCHIVE|FCVAR_CLIENTDLL, "Volume of the ingame music", true, 0.0f, true, 1.0f, MusicVolumeCallback );
ConVar snd_music_ingame( "snd_music_ingame", "1", FCVAR_ARCHIVE|FCVAR_CLIENTDLL, "Enables the In-Game music\n\n0) No In-Game music\n1) Limited In-Game music (not during playtime)\n2) Full In-Game music", MusicIngameCallback );
ConVar snd_music_output( "snd_music_output", "6", FCVAR_ARCHIVE|FCVAR_CLIENTDLL, "Selects the music system output\nWill change on restart\n\n0) No output\n1) DirectSound (default)\n2) Windows MultiMedia\n3) Audio stream I/O (you need ASIO drivers)", MusicOuputCallback );
ConVar snd_music_device( "snd_music_device", "0", FCVAR_ARCHIVE|FCVAR_CLIENTDLL, "Selects the music system driver for the selected output\nWill change on restart\n\nSee snd_music_status for possible values (0 is the default)", MusicDeviceCallback );

//=========================================================
// Music Command
//=========================================================
CON_COMMAND_F( snd_music_restart, "Restarts the music system", FCVAR_CLIENTDLL )
{
	IMusicManager::GetSingleton( ).Restart( );
}

CON_COMMAND_F( snd_music_shutdown, "Stop the music system", FCVAR_CLIENTDLL )
{
	CMusicManager::GetSingleton( ).SetMusicIngame( MUSIC_INGAME_OFF );
}

//=========================================================
//=========================================================
CMusicManager::CMusicManager( void )
{
	m_pPlayer = CreateMusicPlayer( );

	m_iIngameMusicState  = MUSIC_INGAME_LIMITED;
	m_iCurrentSoundGroup = 2;
}

//=========================================================
//=========================================================
CMusicManager::~CMusicManager( void )
{
	if(m_pPlayer)
		delete m_pPlayer;
}

//=========================================================
//=========================================================
bool CMusicManager::Init( void )
{
	if(m_pPlayer && !m_pPlayer->Init( ))
		return false;

	if(!LoadData( ))
		return false;

	gameeventmanager->AddListener( this, "player_spawn", false );
	gameeventmanager->AddListener( this, "player_death", false );
	gameeventmanager->AddListener( this, "player_team", false );
	gameeventmanager->AddListener( this, "round_end", false );
	gameeventmanager->AddListener( this, "game_newmap", false );

	CloseHandle( g_pVCR->Hook_CreateThread( NULL, 0, MusicPlayerThread, m_pPlayer, 0, &m_dwPlayerThread ) );

	return true;
}

//=========================================================
//=========================================================
bool CMusicManager::LoadData( void )
{
	if(!m_pPlayer || !m_pPlayer->Initialized( ))
		return false;

	KeyValues *pSounds = new KeyValues( "Music" );

	if(!pSounds->LoadFromFile( ::filesystem, "music/music.txt" ))
		return false;

	int i = 0;
	int iSoundCount = 0;
	// Sound groups like teams, and songs for menu
	for( KeyValues *pSoundGroup = pSounds->GetFirstTrueSubKey( ); pSoundGroup; pSoundGroup = pSoundGroup->GetNextKey( ) )
	{
		Q_strncpy( m_soundGroups[i].szName, pSoundGroup->GetName( ), 32 );
		m_soundGroups[i].iTeam = i;

		// Sounds types like Death, Victory songs, Menu songs
		for (KeyValues *pSoundTypes = pSoundGroup->GetFirstTrueSubKey( ); pSoundTypes; pSoundTypes = pSoundTypes->GetNextKey( ))
		{
			int iMode = 0;
			if (!STRING_LOOKUP( MUSIC_MODE, pSoundTypes->GetName( ), iMode ) 
				|| iMode <= MUSIC_MODE_OFF
				|| iMode >= MUSIC_MODE_MAX )
			{
				Warning( "Music: invalid music mode %s\n", pSoundTypes->GetName( ) );
				continue;
			}

			// Now you can have several songs per type, lets read them and load them all
			for (KeyValues *pSound = pSoundTypes->GetFirstSubKey( ); pSound; pSound = pSound->GetNextKey( ))
			{
				Sound_t sound;
				sound.iSoundType	= iMode;
				sound.iSoundIdx		= iSoundCount;

				char szFileName[MAX_PATH];
				Q_snprintf( szFileName, MAX_PATH, "%s/%s", engine->GetGameDirectory( ),  pSound->GetString( ) );

				// If we can load it, add it to the list
				if(m_pPlayer->LoadSong( szFileName ))
				{
					m_soundGroups[i].sounds[iMode].push_back( sound );
					iSoundCount++;
				}
			}
		}
		i++;
	}

	pSounds->deleteThis( );

	return true;
}

//=========================================================
//=========================================================
void CMusicManager::Restart( void )
{
	if(!m_pPlayer)
		return;

	ClearMusicLists( );

	m_pPlayer->Shutdown( );

	if(m_iIngameMusicState == MUSIC_MODE_OFF) // Ok music is off, lets not load it again
		return;

	if(!m_pPlayer->Init( ))
		return;

	LoadData( );

	// Recreate the thread if needed
	CloseHandle( g_pVCR->Hook_CreateThread( NULL, 0, MusicPlayerThread, m_pPlayer, 0, &m_dwPlayerThread ) );
}

//=========================================================
//=========================================================
void CMusicManager::Shutdown( void )
{
	gameeventmanager->RemoveListener( this );

	m_pPlayer->Shutdown( );

	ClearMusicLists( );
}

//=========================================================
//=========================================================
void CMusicManager::ClearMusicLists( void )
{
	for( int i = 0; i < 3; i++ )
	{
		// Clear all sound lists of that group
		for( int j = 0; j < MUSIC_MODE_MAX; j++ )
		{
			m_soundGroups[i].sounds[j].clear( );
		}
		
		// Reset sound group data
		m_soundGroups[i].iTeam = -1;
		Q_strncpy( m_soundGroups[i].szName, "", 32 );
	}
}

//=========================================================
//=========================================================
void CMusicManager::Update( float flFrameTime )
{
	if(!m_pPlayer || !m_pPlayer->Initialized( ))
		return;

	// Music for menu
	if ( !m_pPlayer->IsPlaying( ) && m_iCurrentSoundGroup == 2 && !engine->IsInGame( ) )
	{
		m_pPlayer->ClearMusicQueue( );

		SendMusic( MUSIC_MODE_MENU );
	}
}

//=========================================================
//=========================================================
void CMusicManager::LevelShutdownPreEntity( void )
{
	m_pPlayer->ClearMusicQueue( );

	m_iCurrentSoundGroup = 2;
	SendMusic( MUSIC_MODE_MENU );
}

//=========================================================
//=========================================================
void CMusicManager::LevelInitPostEntity( void )
{
	m_pPlayer->ClearMusicQueue( );

	m_iCurrentSoundGroup = 2;
	SendMusic( MUSIC_MODE_PREPARATION );
}

//=========================================================
//=========================================================
void CMusicManager::SetVolume( float flVolume )
{
	if(!m_pPlayer || !m_pPlayer->Initialized( ))
		return;

	// Volume for FMOD is between [0, 255]
	m_pPlayer->SetVolume( flVolume * 255.0f );
}

//=========================================================
//=========================================================
void CMusicManager::SetMusicIngame( MusicIngame_t iState )
{
	m_iIngameMusicState = iState;

	if(m_iIngameMusicState != MUSIC_INGAME_OFF)
		Restart( );
	else
		m_pPlayer->Shutdown( );
}

//=========================================================
//=========================================================
void CMusicManager::SetDevice( int iDevice )
{
	if(m_pPlayer)
		m_pPlayer->SetDevice( iDevice );
}

//=========================================================
//=========================================================
void CMusicManager::SetOutput( int iOutput )
{
	if(m_pPlayer)
		m_pPlayer->SetOutput( iOutput );
}

//=========================================================
//=========================================================
void CMusicManager::SendMusic( MusicModes_t iMode )
{
	Assert( m_iCurrentSoundGroup >= 0 && m_iCurrentSoundGroup <= 2 );

	if(!m_pPlayer || !m_pPlayer->Initialized( ))
		return;

	SoundList sList = m_soundGroups[m_iCurrentSoundGroup].sounds[iMode];
	if(!sList.size( ))
		return;

	int id = RandomInt( 0, sList.size( ) ); // Lets choose a random id

	m_pPlayer->AddMusicToQueue( sList[id == 0 ? 0 : id-1].iSoundIdx );
}

//=========================================================
//=========================================================
void CMusicManager::FireGameEvent( IGameEvent *pEvent )
{
	const char *pszEventName = pEvent->GetName( );

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	if(!m_pPlayer || !m_pPlayer->Initialized( ))
		return;

	// Constants....
	const int iTeam		= pEvent->GetInt( "team" );
	const int iUserID	= pEvent->GetInt( "userid" );
	const int iNoDeath	= pEvent->GetInt( "nodeath" );
	const bool bDeath	= pEvent->GetBool( "death" );
	const int iWinner	= pEvent->GetInt( "winner" );
	const int iPlayerTeam= pPlayer->GetTeamID( );

	// Check if it's a player event
	if( !Q_strncmp( pszEventName, "player_", 7 ) )
	{
		// Check this event is for me
		if( ToINSPlayer( USERID2PLAYER( iUserID ) ) != pPlayer )
			return;

		if ( !Q_strcmp( "player_spawn", pszEventName ) && m_iIngameMusicState == MUSIC_INGAME_FULL )
		{
			if (IsPlayTeam( iTeam ))
				if (!bDeath)
				{
					m_pPlayer->ClearMusicQueue( );
					SendMusic( MUSIC_MODE_COMBAT );
				}
			else
			{
				m_pPlayer->ClearMusicQueue( );
				SendMusic( MUSIC_MODE_PREPARATION );
			}
		}
		else if ( !Q_strcmp( "player_death", pszEventName ) && m_iIngameMusicState == MUSIC_INGAME_FULL )
		{
			if (!iNoDeath)
			{
				m_pPlayer->ClearMusicQueue( );
				SendMusic( MUSIC_MODE_DEATH );
			}
			else
			{
				m_pPlayer->ClearMusicQueue( );
				SendMusic( MUSIC_MODE_PREPARATION );
			}
		}
		else if ( !Q_strcmp( "player_team", pszEventName ) )
		{
			m_pPlayer->ClearMusicQueue( );

			if (!IsPlayTeam( iTeam ) && m_iIngameMusicState == MUSIC_INGAME_FULL)
				SendMusic( MUSIC_MODE_PREPARATION );

			if (IsPlayTeam( iTeam ))
			{
				if(GetGlobalPlayTeam( iTeam )->GetTeamLookupID( ) == TEAM_IRAQI)
					m_iCurrentSoundGroup = 1;
				else if(GetGlobalPlayTeam( iTeam )->GetTeamLookupID( ) == TEAM_USMC)
					m_iCurrentSoundGroup = 0;
			}
			else
				m_iCurrentSoundGroup = 2;
		}
	}
	else if( !Q_strcmp( pszEventName, "round_end" ) && m_iIngameMusicState == MUSIC_INGAME_FULL )
	{
		m_pPlayer->ClearMusicQueue( );

		if (iPlayerTeam == iWinner )
			SendMusic( MUSIC_MODE_VICTORY );
		else
			SendMusic( MUSIC_MODE_DEFEAT );
	}
	else if( !Q_strcmp( pszEventName, "game_newmap" ) && m_iIngameMusicState > MUSIC_INGAME_OFF )
	{
		m_pPlayer->ClearMusicQueue( );
		SendMusic( MUSIC_MODE_PREPARATION );
	}
}