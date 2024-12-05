//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_CLIENTMODE_H
#define INS_CLIENTMODE_H
#ifdef _WIN32
#pragma once
#endif

#include "clientmode_shared.h"

//=========================================================
//=========================================================
class INSViewport;
class CViewSetup;

//=========================================================
//=========================================================
class ClientModeINSNormal : public ClientModeShared 
{
public:
	DECLARE_CLASS( ClientModeINSNormal, ClientModeShared );

public:
	ClientModeINSNormal( );

	void Init( void );
	void InitViewport( void );

	void LevelShutdown( void );

	bool ShouldDrawViewModel( void );
	void PreRender( CViewSetup *pSetup );
	void PostRender( void );
	void PostRenderVGui( void );
	void ProcessInput( bool bActive );

	int KeyInput( int iDown, int iKeyNum, const char *pszCurrentBinding );
	void StartMessageMode( int iMessageModeType );
	vgui::Panel *GetMessagePanel( );

	void FireGameEvent( IGameEvent *pEvent );

	INSViewport *GetINSViewport( void );
};

//=========================================================
//=========================================================
extern IClientMode *GetClientModeNormal( void );
extern ClientModeINSNormal *GetClientModeINSNormal( void );

#endif // INS_CLIENTMODE_H
