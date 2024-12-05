//deathz0rz - Music!

#include "igamesystem.h"
#include "igameevents.h"
#include "musicglobal.h"
#include "imc_format.h"

#ifndef MUSICCONTROL_H
#define MUSICCONTROL_H

typedef struct warning_s {
	char* m_pszWarning;
	warning_s(const char* pszWarning);
	~warning_s();
} warning_t;

class CBaseMusic : public CAutoGameSystem, public IGameEventListener2, public IMusicController
{
public:
	CBaseMusic();
	~CBaseMusic();

	virtual bool Init();
	virtual void Shutdown();
	virtual void Update(float frametime);
	virtual void LevelShutdownPreEntity();
	virtual void LevelInitPostEntity();

	virtual void FireGameEvent(IGameEvent *event);

	mplayer_init_data_s* GetInitData(IMusicController* pController=NULL);

	void Restart();
	void PrintStatus();
	
	const char* GetStatusText();
	mplayer_driver_t* GetDrivers(int &iCount); //do not forget to delete [] the return value!!!

	bool MusicPlaying();

	void SetMusicMode(music_mode_t eMode, const char* pszReason);
	void SetMusicList(int iList, const char* pszReason);

	virtual void SetPlayer(IMusicPlayer* pPlayer);
	virtual void PlayerWarning(const char* pszWarning);

protected:
	music_mode_t GetCurrentMusicMode();				//take in to account menu state
	int GetCurrentMusicList(music_mode_t eMode);	//fallback system

	int RandomSoundFromModeList(int iList,music_mode_t eMode, int iNotThisSound=-1);

	void MusicLog(const char* pszFormat, ...);
	const char* GetModeString(music_mode_t eMode, const char* pszNoMusicString=NULL);
	
	//lookup in m_aiMusicLists
	bool GetIndexModeListFromSound(int iSound, int *iSoundIndex=NULL, music_mode_t *eMode=NULL, int *iList=NULL);

private:
	music_mode_t			m_eMusicMode;
	int						m_iMusicList;
	bool					m_bInitialized;

	//m_aiMusicLists[m_iMusicList][GetCurrentMusicMode()][0]=index into sounds array
	//m_aiMusicLists[m_iMusicList][GetCurrentMusicMode()][1]=number of songs at index
	//Generic music list = MAX_LOOKUP_TEAMS
	int						m_aiMusicLists[MAX_LOOKUP_TEAMS+1][MUSIC_MODE_MAX][2];

	IMusicPlayer*			m_pPlayer;
	//we need this here because stuff needs to be kept in memory
	KeyValues*				m_pKV;
	CUtlVector<warning_t*>	m_Warnings;
	mplayer_status_t		m_Status;
};

extern CBaseMusic g_Music;

#endif //MUSICCONTROL_H