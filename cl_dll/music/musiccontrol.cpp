//deathz0rz - Music!

#define PROTECTED_THINGS_H
#include "cbase.h"
#include <windows.h>
#undef PROTECTED_THINGS_H
#include "protected_things.h"
#include "convar.h"
#include "keyvalues.h"
#include "filesystem.h"
#include "clientmode_shared.h"
#include "c_ins_player.h"

#include "musiccontrol.h"
#include "gameuipanel.h"
#include "c_team.h"
#include "c_play_team.h"

#include "delayload.h"

#include "ins_gamerules.h"
#include "play_team_shared.h"

#include "tier0/vcrmode.h"
#include <time.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//Warning doesnt always (CAutoGameSystem::Init) work for some reason
#define Warning Msg

CBaseMusic g_Music;

DEFINE_STRING_LOOKUP_CONSTANTS(int,MUSIC_MODE)

	ADD_LOOKUP(MUSIC_MODE_OFF)
	ADD_LOOKUP(MUSIC_MODE_MENU)
	ADD_LOOKUP(MUSIC_MODE_PREPARATION)
	ADD_LOOKUP(MUSIC_MODE_COMBAT)
	ADD_LOOKUP(MUSIC_MODE_DEATH)
	ADD_LOOKUP(MUSIC_MODE_VICTORY)
	ADD_LOOKUP(MUSIC_MODE_DEFEAT)

END_STRING_LOOKUP_CONSTANTS()

//deathz0rz [ HACK HACK - this should be in???
ConVar snd_musicvolume("snd_musicvolume","");;

ConVar snd_music_ingame("snd_music_ingame", "1", FCVAR_ARCHIVE|FCVAR_CLIENTDLL, "Enables the In-Game music\n\n0) No In-Game music\n1) Limited In-Game music (not during playtime)\n2) Full In-Game music");
ConVar snd_music_output("snd_music_output", "1", FCVAR_ARCHIVE|FCVAR_CLIENTDLL, "Selects the music system output\nWill change on restart\n\n0) No output\n1) DirectSound (default)\n2) Windows MultiMedia\n3) Audio stream I/O (you need ASIO drivers)");
ConVar snd_music_device("snd_music_device", "0", FCVAR_ARCHIVE|FCVAR_CLIENTDLL, "Selects the music system driver for the selected output\nWill change on restart\n\nSee snd_music_status for possible values (0 is the default)");

ConVar snd_music_debug("snd_music_debug", 
#ifdef  _DEBUG
"1"
#else
"0"
#endif
, FCVAR_CLIENTDLL);

CON_COMMAND_F(snd_music_restart, "Restarts the music system", FCVAR_CLIENTDLL)
{
	g_Music.Restart();
}

CON_COMMAND_F(snd_music_status, "Show the status of the music system", FCVAR_CLIENTDLL)
{
	g_Music.PrintStatus();
}

CON_COMMAND_F(snd_music_off, "Turn off the music system (sets output to no output and restarts)", FCVAR_CLIENTDLL)
{
	snd_music_output.SetValue(0);
	g_Music.Restart();
}

CBaseMusic::CBaseMusic()
{
	m_eMusicMode=MUSIC_MODE_MENU;
	m_iMusicList=0;
	m_pPlayer=NULL;
	m_bInitialized=false;

	m_pKV=new KeyValues("Music");
}

CBaseMusic::~CBaseMusic()	
{
	m_Warnings.PurgeAndDeleteElements();
	m_pKV->deleteThis();

	gameeventmanager->RemoveListener(this);
}

bool CBaseMusic::Init()
{
	char szFMODPath[MAX_PATH];
	Q_snprintf(szFMODPath,MAX_PATH,"%s/bin/",engine->GetGameDirectory());

	__DLInitDelayLoadDLL(__DLNoFail);
	__DLSetSearchPath(szFMODPath);
	__DLLoadDelayLoadDLL("fmodex.dll");
	__DLFinishDelayLoadDLL;

	gameeventmanager->AddListener(this,"player_spawn",false);
	gameeventmanager->AddListener(this,"player_death",false);
	gameeventmanager->AddListener(this,"player_team",false);
	//gameeventmanager->AddListener(this,"game_end",false);
	gameeventmanager->AddListener(this,"round_end",false);
	gameeventmanager->AddListener(this,"game_newmap",false);
	//gameeventmanager->AddListener(this,"game_start",false);

	CreateMusicPlayer(GetInitData(this));

	m_bInitialized=true;
	MusicLog("Starting music control system");
	return true;
}

void CBaseMusic::Shutdown()
{
	if (m_pPlayer)
		m_pPlayer->Terminate();

	MusicLog("Stopping music control system");
	gameeventmanager->RemoveListener(this);
}

mplayer_init_data_s* CBaseMusic::GetInitData(IMusicController* pController)
{
	static mplayer_init_data_s InitData;
	InitData.pController=pController;
	InitData.iOutput=m_bInitialized?snd_music_output.GetInt():MUSIC_OUTPUTTYPE_NOSOUND; //wait until the cvars are initialized by running config.cfg!
	InitData.iDriver=snd_music_device.GetInt();
	Q_strncpy(InitData.szSearchPath,engine->GetGameDirectory(),MAX_PATH);

	m_pKV->LoadFromFile(::filesystem,"music/music.txt");
	InitData.iSounds=m_pKV->GetInt("Count");
	InitData.pszSounds=new const char*[InitData.iSounds];
	memset(m_aiMusicLists,0,sizeof(m_aiMusicLists));

	char szTeamID[16];
	int iSound=0, iMode=0;
	KeyValues* pKV[3];
#define FOR_KV(var,base) \
	for (var[base+1]=var[base]->GetFirstSubKey();var[base+1];var[base+1]=var[base+1]->GetNextKey())

	for (int iCurTeam=0;iCurTeam<=MAX_LOOKUP_TEAMS;iCurTeam++)
	{
		if (iCurTeam==MAX_LOOKUP_TEAMS)
		{
			Q_strncpy(szTeamID,"Generic",16);
		}
		else
		{
			Q_snprintf(szTeamID,16,"%d",iCurTeam);
		}
		pKV[0]=m_pKV->FindKey(szTeamID,false);
		if (!pKV[0])
		{
			Warning("Music: no entry for %s\n",szTeamID);
			continue;
		}
		FOR_KV(pKV,0)
		{
			if (iSound>=InitData.iSounds)
			{
				Warning("Music: sounds overflow\n");
				break;
			}
			if (!STRING_LOOKUP(MUSIC_MODE,pKV[1]->GetName(),iMode)||iMode<=MUSIC_MODE_OFF||iMode>=MUSIC_MODE_MAX)
			{
				Warning("Music: invalid music mode %s\n",pKV[1]->GetName());
				continue;
			}
			m_aiMusicLists[iCurTeam][iMode][0]=iSound;
			FOR_KV(pKV,1)
			{
				if (iSound>=InitData.iSounds)
				{
					Warning("Music: sounds overflow\n");
					break;
				}
				InitData.pszSounds[iSound]=pKV[2]->GetString();
				iSound++;
			}
			m_aiMusicLists[iCurTeam][iMode][1]=iSound-m_aiMusicLists[iCurTeam][iMode][0];
		}
	}

	if (iSound<InitData.iSounds)
	{
		Warning("Music: sounds underflow\n");
		InitData.iSounds=iSound;
	}

	Update(0.0f);

	return &InitData;
}

void CBaseMusic::Restart()
{
	if (!m_pPlayer)
	{
		Warning("There is no player object\n");
		return;
	}
	m_pPlayer->Restart(GetInitData());
	g_iMusicUpdateBits|=0x1;
	while (g_iMusicUpdateBits&0x1)
	{
		Sleep(5);
	}
}

mplayer_driver_t* CBaseMusic::GetDrivers(int &iCount)
{
	int iDrivers=m_pPlayer->GetDrivers(NULL,0);
	mplayer_driver_t* pDrivers=new mplayer_driver_t[iDrivers];
	m_pPlayer->GetDrivers(pDrivers,iDrivers);
	iCount=iDrivers;
	return pDrivers;
}

void CBaseMusic::PrintStatus()
{
	if (!m_pPlayer)
	{
		Warning("There is no player object\n");
		return;
	}
	
	int iDrivers=m_pPlayer->GetDrivers(NULL,0);
	mplayer_driver_t* pDrivers=new mplayer_driver_t[iDrivers];
	m_pPlayer->GetDrivers(pDrivers,iDrivers);

	size_t iStatusLen=(iDrivers+2)*300; //arbitrary size, this should always be enough;
	char* pszStatus=new char[iStatusLen];
	static char szTemp[1024];
	pszStatus[0]='\0';
	szTemp[0]='\0';

	if (!m_pPlayer->GetStatus(&m_Status))
	{
		Warning("The player object is unavailable\n");
		return;
	}
	music_mode_t eMode=GetCurrentMusicMode();

	bool bAddPlaybackstatus=true;
	if (m_Status.iSound!=-1)
		Q_strncpy(szTemp,"Now playing music for ",1024);
	else
	{
		Q_strncpy(szTemp,"Not playing any music for ",1024);
		bAddPlaybackstatus=false;
	}

	const char *pszTemp=GetModeString(eMode);

	if (!pszTemp)
	{
		Q_strncpy(szTemp,"Not playing any music\n",1024);
		bAddPlaybackstatus=false;
	}
	else
	{
		Q_strncat(szTemp,pszTemp,sizeof(szTemp),COPY_ALL_CHARACTERS);
		Q_strncat(szTemp,"\n",sizeof(szTemp),COPY_ALL_CHARACTERS);
	}

	Q_strncat(pszStatus,szTemp,iStatusLen,iStatusLen);
	if (bAddPlaybackstatus)
	{
		m_Status.iLength*=.001;
		m_Status.iPosition*=.001;
		int iLenM=m_Status.iLength/60;
		m_Status.iLength%=60;
		int iPosM=m_Status.iPosition/60;
		m_Status.iPosition%=60;
		Q_snprintf(szTemp,1024,"%s - %d:%02d/%d:%02d\n",m_Status.szSoundName,iPosM,m_Status.iPosition,iLenM,m_Status.iLength);
		Q_strncat(pszStatus,szTemp,iStatusLen,iStatusLen);
	}

	Q_snprintf(szTemp,1024,"\nAvailable drivers for current output:\n");
	Q_strncat(pszStatus,szTemp,iStatusLen,iStatusLen);
	for (int i=0;i<iDrivers;i++)
	{
		Q_snprintf(szTemp,1024,"%2d) %s\n",pDrivers[i].iID,pDrivers[i].szName);
		Q_strncat(pszStatus,szTemp,iStatusLen,iStatusLen);
	}

	Msg(pszStatus);

	delete [] pszStatus;
	delete [] pDrivers;
}

int CBaseMusic::RandomSoundFromModeList(int iList,music_mode_t eMode, int iNotThisSound)
{
	if (iList>=0&&iList<=MAX_LOOKUP_TEAMS&&eMode!=MUSIC_MODE_OFF)
		return m_aiMusicLists[iList][eMode][0]+RandomInt(0,m_aiMusicLists[iList][eMode][1]-1);
	return -1;
}

void CBaseMusic::Update(float frametime)
{
	for (int i=0;i<m_Warnings.Count();i++)
	{
		MusicLog("Warning: %s",m_Warnings[i]->m_pszWarning);
		Warning(m_Warnings[i]->m_pszWarning);
	}
	m_Warnings.PurgeAndDeleteElements();

	if (!m_pPlayer)
		return;

	int iSoundPrev=m_Status.iSound;
	if (!m_pPlayer->GetStatus(&m_Status))
		return;

	music_mode_t eMode=GetCurrentMusicMode();

	if (eMode!=MUSIC_MODE_OFF)
	{
		int iSound=-1;
		int iList=-1;
		eMode=MUSIC_MODE_OFF;
		if (m_Status.iSound!=-1)
		{
			GetIndexModeListFromSound(m_Status.iSound,&iSound,&eMode,&iList);
		}
		if (eMode!=GetCurrentMusicMode()||iList!=GetCurrentMusicList(eMode))
		{
			eMode=GetCurrentMusicMode();
			iList=GetCurrentMusicList(eMode);
			if (iList>=0&&eMode!=MUSIC_MODE_OFF)
			{
				switch (eMode)
				{
					case MUSIC_MODE_DEATH:
					case MUSIC_MODE_VICTORY:
					case MUSIC_MODE_DEFEAT:
						break;
					case MUSIC_MODE_PREPARATION:
					case MUSIC_MODE_COMBAT:
						m_pPlayer->PlaylistClear();
						m_pPlayer->PlaylistAdd(RandomSoundFromModeList(iList,eMode,m_Status.iSound));
						break;
					case MUSIC_MODE_MENU:
						m_pPlayer->PlaylistClear();
						m_pPlayer->PlaylistAdd(RandomSoundFromModeList(MAX_LOOKUP_TEAMS,MUSIC_MODE_MENU,m_Status.iSound));
				}
			}
			else
			{
				m_pPlayer->PlaylistClear(true);
			}
		}
	}
	else
	{
		m_pPlayer->PlaylistClear(true);
	}

	m_pPlayer->SetVolume(snd_musicvolume.GetFloat());

	if (iSoundPrev!=m_Status.iSound)
	{
		MusicLog("Changed music: was playing %d, now playing %d",iSoundPrev,m_Status.iSound);
	}
}

bool CBaseMusic::MusicPlaying()
{
	if (!m_pPlayer)
		return false;
	if (!m_pPlayer->GetStatus(&m_Status))
		return false;
	return m_Status.iSound!=-1;
}

bool CBaseMusic::GetIndexModeListFromSound(int iSound, int *iSoundIndex, music_mode_t *eMode, int *iList)
{
	int ePMode;
	int iPList;
	int iTSound;

	for (iPList=0;iPList<=MAX_LOOKUP_TEAMS;iPList++)
		for (ePMode=MUSIC_MODE_OFF;ePMode<MUSIC_MODE_MAX;ePMode++)
		{
			iTSound=iSound-m_aiMusicLists[iPList][ePMode][0];
			if (iTSound>=0&&iTSound<m_aiMusicLists[iPList][ePMode][1])
			{
				if (iSoundIndex)
					*iSoundIndex=iTSound;
				if (eMode)
					*eMode=(music_mode_t)ePMode;
				if (iList)
					*iList=iPList;
				return true;
			}
		}
	return false;
}

const char* CBaseMusic::GetModeString(music_mode_t eMode, const char* pszNoMusicString)
{
	switch (eMode)
	{
		case MUSIC_MODE_MENU:
			return "menu";
		case MUSIC_MODE_PREPARATION:
			return "preparation";
		case MUSIC_MODE_DEATH:
			return "death";
		case MUSIC_MODE_VICTORY:
			return "victory";
		case MUSIC_MODE_DEFEAT:
			return "defeat";
		case MUSIC_MODE_COMBAT:
			return "combat";
	}
	return pszNoMusicString;
}

void CBaseMusic::SetMusicMode(music_mode_t eMode, const char* pszReason)
{
	// Pongles ]
	if(!m_pPlayer)
		return;
	// Pongles ]

	if (eMode!=m_eMusicMode)
		m_pPlayer->PlaylistClear(true);
	m_eMusicMode=eMode;
	Update(0.0f);
	MusicLog("Changing music mode to %s because: %s",GetModeString(eMode,"no music"),pszReason);
}

void CBaseMusic::SetMusicList(int iList, const char* pszReason)
{
	// Pongles ]
	if(!m_pPlayer)
		return;
	// Pongles ]

	if (m_iMusicList!=iList)
		m_pPlayer->PlaylistClear(true);
	m_iMusicList=iList;
	Update(0.0f);
	MusicLog("Changing music list to %d because: %s",iList,pszReason);
}

void CBaseMusic::MusicLog(const char* pszFormat, ...)
{
	if (snd_music_debug.GetBool()||developer.GetBool())
	{
		struct tm now;
		g_pVCR->Hook_LocalTime(&now);
		char szMessage[2048];
		va_list argptr;
		va_start( argptr, pszFormat );
		Q_vsnprintf( szMessage, 2048, pszFormat, argptr );
		va_end( argptr );
		char szTime[24];
		Q_snprintf(szTime,24,"(%02d-%02d-%02d %02d:%02d:%02d) \n",now.tm_mday,now.tm_mon+1,now.tm_year+1900,now.tm_hour,now.tm_min,now.tm_sec);
		FileHandle_t hFile=::filesystem->Open("music.log","a+");
		::filesystem->Write(szTime,22,hFile);
		::filesystem->Write(szMessage,strlen(szMessage),hFile);
		::filesystem->Write(szTime+22,1,hFile);
		::filesystem->Close(hFile);
	}
}

void CBaseMusic::SetPlayer(IMusicPlayer* pPlayer)
{
	m_pPlayer=pPlayer;
}

void CBaseMusic::PlayerWarning(const char* pszWarning)
{
	m_Warnings[m_Warnings.AddToTail()]=new warning_t(pszWarning);
}

music_mode_t CBaseMusic::GetCurrentMusicMode()
{
	switch (m_eMusicMode)
	{
		case MUSIC_MODE_OFF:
		case MUSIC_MODE_MENU:
			return m_eMusicMode;
		case MUSIC_MODE_PREPARATION:
		case MUSIC_MODE_DEATH:
		case MUSIC_MODE_VICTORY:
		case MUSIC_MODE_DEFEAT:
			return snd_music_ingame.GetInt()>0?m_eMusicMode:MUSIC_MODE_OFF;
		case MUSIC_MODE_COMBAT:
			return snd_music_ingame.GetInt()>1?m_eMusicMode:MUSIC_MODE_OFF;
		default:
			return MUSIC_MODE_OFF;
	}
}

int CBaseMusic::GetCurrentMusicList(music_mode_t eMode)
{
	if (m_aiMusicLists[m_iMusicList][eMode][1])
		return m_iMusicList;

	if (m_aiMusicLists[MAX_LOOKUP_TEAMS][eMode][1])
		return MAX_LOOKUP_TEAMS;

	return -1;
}

void CBaseMusic::LevelShutdownPreEntity()
{
	SetMusicMode(MUSIC_MODE_MENU,"Level Shutdown");
}

void CBaseMusic::LevelInitPostEntity()
{
	SetMusicMode(MUSIC_MODE_PREPARATION,"Level Initialization");
}

void CBaseMusic::FireGameEvent(IGameEvent *event)
{
// xENO [
//  Why would someone put a "return;" here!?
//	return;
// ] xENO

	const char *eventname = event->GetName();

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();

	if( !pPlayer )
		return;

	if ( Q_strncmp("player_", eventname, 7) == 0 )
	{
		int iUID=event->GetInt("userid");
		C_INSPlayer *pPlayerEvent = ToINSPlayer(USERID2PLAYER( iUID ));
		if ( pPlayerEvent!=pPlayer )
			return;

		if ( Q_strcmp( "player_spawn", eventname ) == 0 )
		{
			if (IsPlayTeam(event->GetInt("team")))
			{
				if (!event->GetBool("dead"))
					SetMusicMode(MUSIC_MODE_COMBAT,"Player spawn, on team, no dead spawn");
			}
			else
				SetMusicMode(MUSIC_MODE_PREPARATION,"Player spawn, no team");
		}
		else if ( Q_strcmp( "player_death", eventname ) == 0 )
		{
			if (!event->GetInt("nodeath"))
			{
				SetMusicMode(MUSIC_MODE_DEATH,"Player death, player died");
				m_pPlayer->PlaylistAdd(RandomSoundFromModeList(GetCurrentMusicList(GetCurrentMusicMode()),GetCurrentMusicMode()));
			}
			else
				SetMusicMode(MUSIC_MODE_PREPARATION,"Player death, player didn't die");
		}
		else if ( Q_strcmp( "player_team", eventname ) == 0 )
		{
			int iTeam=event->GetInt("team");
			if (IsPlayTeam(iTeam))
			{
				C_PlayTeam* pTeam=GetGlobalPlayTeam(iTeam);
				if (pTeam)
					SetMusicList(pTeam->GetTeamLookupID(),"Team change to player team");
			}
			else
			{
				SetMusicList(MAX_LOOKUP_TEAMS,"Team change to non-player team");
				SetMusicMode(MUSIC_MODE_PREPARATION,"Team change to non-player team");
			}
		}
	}
	else if ( /*Q_strcmp( "game_end", eventname ) == 0 ||*/ Q_strcmp( "round_end", eventname ) == 0 )
	{
		if (pPlayer->GetTeamID()==event->GetInt("winner"))
		{
			SetMusicMode(MUSIC_MODE_VICTORY,"Round end, winning team");
		}
		else
		{
			SetMusicMode(MUSIC_MODE_DEFEAT,"Round end, not winning team");
		}
		m_pPlayer->PlaylistAdd(RandomSoundFromModeList(GetCurrentMusicList(GetCurrentMusicMode()),GetCurrentMusicMode()));
	}
	else if ( Q_strcmp( "game_newmap", eventname ) == 0/* || Q_strcmp( "game_start", eventname ) == 0*/ )
	{
		SetMusicMode(MUSIC_MODE_PREPARATION,"Map load");
	}
}

const char* CBaseMusic::GetStatusText()
{
	if (!m_pPlayer)
	{
		return "There is no player object\n";
	}

	static char szStatus[1024];
	static char szTemp[1024];
	szStatus[0]='\0';
	szTemp[0]='\0';

	if (!m_pPlayer->GetStatus(&m_Status))
	{
		return "The player object is unavailable\n";
	}
	music_mode_t eMode=GetCurrentMusicMode();

	bool bAddPlaybackstatus=true;
	if (m_Status.iSound!=-1)
		Q_strncpy(szTemp,"Now playing music for ",1024);
	else
	{
		Q_strncpy(szTemp,"Not playing any music for ",1024);
		bAddPlaybackstatus=false;
	}

	const char *pszTemp=GetModeString(eMode);

	if (!pszTemp)
	{
		Q_strncpy(szTemp,"Not playing any music\n",1024);
		bAddPlaybackstatus=false;
	}
	else
	{
		Q_strncat(szTemp,pszTemp,sizeof(szTemp),COPY_ALL_CHARACTERS);
		Q_strncat(szTemp,"\n",sizeof(szTemp),COPY_ALL_CHARACTERS);
	}

	Q_strncat(szStatus,szTemp,1024,1024);
	if (bAddPlaybackstatus)
	{
		m_Status.iLength*=.001;
		m_Status.iPosition*=.001;
		int iLenM=m_Status.iLength/60;
		m_Status.iLength%=60;
		int iPosM=m_Status.iPosition/60;
		m_Status.iPosition%=60;
		Q_snprintf(szTemp,1024,"%s - %d:%02d/%d:%02d\n",m_Status.szSoundName,iPosM,m_Status.iPosition,iLenM,m_Status.iLength);
		Q_strncat(szStatus,szTemp,1024,1024);
	}

	return szStatus;
}

warning_s::warning_s(const char* pszWarning)
{
	m_pszWarning=new char[Q_strlen(pszWarning)+1];
	Q_strcpy(m_pszWarning,pszWarning);
}

warning_s::~warning_s()
{
	delete [] m_pszWarning;
}