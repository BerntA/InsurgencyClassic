// Insurgency Team (C) 2007
// First revision
// remastered by BerntA

#include "cbase.h"
#include "fmod_ambience.h"
#include "musicmanager.h"
#include "convar.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "clientmode_shared.h"
#include "c_ins_player.h"
#include "c_team.h"
#include "c_play_team.h"
#include "ins_gamerules.h"
#include "play_team_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CUtlVector<int> g_iMusicRandomizerHelper;
static CUtlVector<const MusicEntry_t*> g_pMusicQueue;

DECLARE_STRING_LOOKUP_CONSTANTS(int, MUSIC_MODE);

DEFINE_STRING_LOOKUP_CONSTANTS(int, MUSIC_MODE)

ADD_LOOKUP(MUSIC_MODE_OFF)
ADD_LOOKUP(MUSIC_MODE_MENU)
ADD_LOOKUP(MUSIC_MODE_PREPARATION)
ADD_LOOKUP(MUSIC_MODE_COMBAT)
ADD_LOOKUP(MUSIC_MODE_DEATH)
ADD_LOOKUP(MUSIC_MODE_VICTORY)
ADD_LOOKUP(MUSIC_MODE_DEFEAT)
ADD_LOOKUP(MUSIC_MODE_MAX)

END_STRING_LOOKUP_CONSTANTS()

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

static CMusicManager s_MusicManager;

CMusicManager::CMusicManager() : CAutoGameSystem("MusicManager")
{
	m_iIngameMusicState = MUSIC_INGAME_LIMITED;
	m_iCurrentSoundGroup = 2;
}

CMusicManager::~CMusicManager()
{
}

bool CMusicManager::Init(void)
{
	if (!LoadData())
		return false;

	ListenForGameEvent("player_spawn");
	ListenForGameEvent("player_death");
	ListenForGameEvent("player_team");
	ListenForGameEvent("round_end");
	ListenForGameEvent("game_newmap");

	return true;
}

bool CMusicManager::LoadData(void)
{
	bool success = false;
	KeyValues* pSounds = new KeyValues("Music");

	if (pSounds->LoadFromFile(filesystem, "scripts/music.txt", "MOD"))
	{
		int i = 0;
		int iMode = 0;

		// Sound groups like teams, and songs for menu
		for (KeyValues* pSoundGroup = pSounds->GetFirstTrueSubKey(); pSoundGroup; pSoundGroup = pSoundGroup->GetNextKey())
		{
			Q_strncpy(m_soundGroups[i].szName, pSoundGroup->GetName(), sizeof(m_soundGroups[i].szName));
			m_soundGroups[i].iTeam = i;

			// Sounds types like Death, Victory songs, Menu songs
			for (KeyValues* pSoundTypes = pSoundGroup->GetFirstTrueSubKey(); pSoundTypes; pSoundTypes = pSoundTypes->GetNextKey())
			{
				if (!STRING_LOOKUP(MUSIC_MODE, pSoundTypes->GetName(), iMode)
					|| iMode <= MUSIC_MODE_OFF
					|| iMode >= MUSIC_MODE_MAX)
				{
					Warning("Music: invalid music mode %s\n", pSoundTypes->GetName());
					continue;
				}

				// Now you can have several songs per type, lets read them and load them all
				for (KeyValues* pSound = pSoundTypes->GetFirstSubKey(); pSound; pSound = pSound->GetNextKey())
				{
					MusicEntry_t sound;
					sound.type = iMode;
					Q_strncpy(sound.pchSoundFile, pSound->GetString(), MAX_PATH);
					m_soundGroups[i].sounds.AddToTail(sound);
				}
			}
			i++;
		}

		success = true;
	}

	pSounds->deleteThis();
	return success;
}

void CMusicManager::Restart(void)
{
	if (m_iIngameMusicState == MUSIC_MODE_OFF) // Ok music is off, lets not load it again
		return;

	m_fmodSound.Restart();
}

void CMusicManager::Shutdown(void)
{
	g_iMusicRandomizerHelper.Purge();
	m_fmodSound.Destroy();
	ClearMusicQueue();
}

void CMusicManager::Update(float flFrameTime)
{
	// Music for menu
	if (!m_fmodSound.IsPlaying() && m_iCurrentSoundGroup == 2 && !engine->IsInGame())
	{
		ClearMusicQueue();
		SendMusic(MUSIC_MODE_MENU);
	}

	m_fmodSound.Think();

	if ((g_pMusicQueue.Count() == 0) || m_fmodSound.IsPlaying())
		return;

	const MusicEntry_t* pNextSong = g_pMusicQueue.Head();
	m_fmodSound.PlaySound(pNextSong->pchSoundFile, false);
	m_fmodSound.Think();
	g_pMusicQueue.Remove(0);
}

void CMusicManager::LevelShutdownPreEntity(void)
{
	ClearMusicQueue();
	m_iCurrentSoundGroup = 2;
	SendMusic(MUSIC_MODE_MENU);
}

void CMusicManager::LevelInitPostEntity(void)
{
	ClearMusicQueue();
	m_iCurrentSoundGroup = 2;
	SendMusic(MUSIC_MODE_PREPARATION);
}

void CMusicManager::ClearMusicQueue(void)
{
	g_pMusicQueue.RemoveAll();
}

void CMusicManager::SetMusicIngame(MusicIngame_t iState)
{
	m_iIngameMusicState = iState;

	if (m_iIngameMusicState != MUSIC_INGAME_OFF)
		Restart(); // TODO
	else
		m_fmodSound.Destroy();
}

void CMusicManager::SendMusic(int iMode)
{
	const auto* pMusicItem = m_soundGroups[m_iCurrentSoundGroup].GetRandomSongForMode(iMode);

	if (pMusicItem == NULL)
	{
		Warning("Found no valid music for mode %i!\n", iMode);
		return;
	}

	g_pMusicQueue.AddToTail(pMusicItem);
}

void CMusicManager::FireGameEvent(IGameEvent* pEvent)
{
	const char* pszEventName = pEvent->GetName();

	C_INSPlayer* pPlayer = C_INSPlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	// Constants....
	const int iTeam = pEvent->GetInt("team");
	const int iUserID = pEvent->GetInt("userid");
	const int iNoDeath = pEvent->GetInt("nodeath");
	const bool bDeath = pEvent->GetBool("death");
	const int iWinner = pEvent->GetInt("winner");
	const int iPlayerTeam = pPlayer->GetTeamID();

	// Check if it's a player event
	if (!Q_strncmp(pszEventName, "player_", 7))
	{
		// Check this event is for me
		if (ToINSPlayer(USERID2PLAYER(iUserID)) != pPlayer)
			return;

		if (!Q_strcmp("player_spawn", pszEventName) && m_iIngameMusicState == MUSIC_INGAME_FULL)
		{
			if (IsPlayTeam(iTeam))
				if (!bDeath)
				{
					ClearMusicQueue();
					SendMusic(MUSIC_MODE_COMBAT);
				}
				else
				{
					ClearMusicQueue();
					SendMusic(MUSIC_MODE_PREPARATION);
				}
		}
		else if (!Q_strcmp("player_death", pszEventName) && m_iIngameMusicState == MUSIC_INGAME_FULL)
		{
			if (!iNoDeath)
			{
				ClearMusicQueue();
				SendMusic(MUSIC_MODE_DEATH);
			}
			else
			{
				ClearMusicQueue();
				SendMusic(MUSIC_MODE_PREPARATION);
			}
		}
		else if (!Q_strcmp("player_team", pszEventName))
		{
			ClearMusicQueue();

			if (!IsPlayTeam(iTeam) && m_iIngameMusicState == MUSIC_INGAME_FULL)
				SendMusic(MUSIC_MODE_PREPARATION);

			if (IsPlayTeam(iTeam))
			{
				if (GetGlobalPlayTeam(iTeam)->GetTeamLookupID() == TEAM_IRAQI)
					m_iCurrentSoundGroup = 1;
				else if (GetGlobalPlayTeam(iTeam)->GetTeamLookupID() == TEAM_USMC)
					m_iCurrentSoundGroup = 0;
			}
			else
				m_iCurrentSoundGroup = 2;
		}
	}
	else if (!Q_strcmp(pszEventName, "round_end") && m_iIngameMusicState == MUSIC_INGAME_FULL)
	{
		ClearMusicQueue();

		if (iPlayerTeam == iWinner)
			SendMusic(MUSIC_MODE_VICTORY);
		else
			SendMusic(MUSIC_MODE_DEFEAT);
	}
	else if (!Q_strcmp(pszEventName, "game_newmap") && m_iIngameMusicState > MUSIC_INGAME_OFF)
	{
		ClearMusicQueue();
		SendMusic(MUSIC_MODE_PREPARATION);
	}
}

const MusicEntry_t* CSoundGroup::GetRandomSongForMode(int mode) const
{
	g_iMusicRandomizerHelper.Purge();

	for (int i = 0; i < sounds.Count(); i++)
	{
		if (sounds[i].type == mode)
			g_iMusicRandomizerHelper.AddToTail(i);
	}

	if (g_iMusicRandomizerHelper.Count() == 0)
		return NULL;

	int index = g_iMusicRandomizerHelper[random->RandomInt(0, g_iMusicRandomizerHelper.Count() - 1)];

	return &sounds[index];
}

void MusicIngameCallback(IConVar* pConVar, char const* pOldString, float flOldValue)
{
	ConVar* var = (ConVar*)pConVar;
	s_MusicManager.SetMusicIngame((MusicIngame_t)var->GetInt());
}

ConVar snd_music_ingame("snd_music_ingame", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL, "Enables the In-Game music\n\n0) No In-Game music\n1) Limited In-Game music (not during playtime)\n2) Full In-Game music", MusicIngameCallback);

CON_COMMAND_F(snd_music_restart, "Restarts the music system", FCVAR_CLIENTDLL)
{
	s_MusicManager.Restart();
}

CON_COMMAND_F(snd_music_shutdown, "Stop the music system", FCVAR_CLIENTDLL)
{
	s_MusicManager.SetMusicIngame(MUSIC_INGAME_OFF);
}