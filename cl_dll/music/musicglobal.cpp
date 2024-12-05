//deathz0rz - Music!

#define PROTECTED_THINGS_H
#include "cbase.h"
#include <windows.h>
#undef PROTECTED_THINGS_H
#include "protected_things.h"

#include "musicplayer.h"
#include "musicglobal.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

unsigned int g_iMusicUpdateBits;
bool g_bMusicPlayerInitialized;

DWORD WINAPI MusicPlayerThread(mplayer_init_data_t* pInitData)
{
	IMusicPlayer* pPlayer=new CMusicPlayer();
	pPlayer->Init(pInitData);
	g_bMusicPlayerInitialized=pPlayer->Initialized();
	unsigned int iAckUpdateBits=0;
	g_iMusicUpdateBits=0;
	while (pPlayer->Update())
	{
		g_bMusicPlayerInitialized=pPlayer->Initialized();
		g_iMusicUpdateBits&=~iAckUpdateBits;
		Sleep(5);
		iAckUpdateBits=g_iMusicUpdateBits;
	}
	g_iMusicUpdateBits=0;
	g_bMusicPlayerInitialized=false;
	if (pInitData->pController)
		pInitData->pController->SetPlayer(NULL);
	delete pPlayer;
	return TRUE;
}

void CreateMusicPlayer(mplayer_init_data_t* pInitData)
{
	DWORD dwThread;
	CloseHandle(g_pVCR->Hook_CreateThread(NULL,0,MusicPlayerThread,pInitData,0,&dwThread));
}