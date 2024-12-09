#ifndef INS_CLIENTMODE_H
#define INS_CLIENTMODE_H

#ifdef _WIN32
#pragma once
#endif

#include "clientmode_shared.h"
#include "ivmodemanager.h"

class CHudViewport;

class CINSModeManager : public IVModeManager
{
public:
	CINSModeManager(void);
	virtual ~CINSModeManager(void);

	virtual void	Init(void);
	virtual void	SwitchMode(bool commander, bool force);
	virtual void	OverrideView(CViewSetup* pSetup);
	virtual void	CreateMove(float flInputSampleTime, CUserCmd* cmd);
	virtual void	LevelInit(const char* newmap);
	virtual void	LevelShutdown(void);
};

class ClientModeINSNormal : public ClientModeShared
{
public:
	DECLARE_CLASS(ClientModeINSNormal, ClientModeShared);

	ClientModeINSNormal();
	virtual ~ClientModeINSNormal();

	virtual void	Init();
};

extern IClientMode* GetClientModeNormal();
extern ClientModeINSNormal* GetClientModeINSNormal();

#endif // INS_CLIENTMODE_H