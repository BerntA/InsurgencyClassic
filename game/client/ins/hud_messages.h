//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_MESSAGES_H
#define HUD_MESSAGES_H
#ifdef _WIN32
#pragma once
#endif

#include "hud_messages_base.h"

#include "vgui_basepanel.h"
#include <vgui_controls/textentry.h>

#include "deadhelper.h"
#include "inshud.h"

//=========================================================
//=========================================================
namespace vgui
{
	class IScheme;
};

class CHudChatInputLine;
class CHudChatEntry;

//=========================================================
//=========================================================
class CHudMessages : public CHudMessagesBase, public CDeadHUDHelper, public IINSChatMessages, public IINSChatListener
{
	DECLARE_CLASS_SIMPLE( CHudMessages, CHudMessagesBase );

public:
	CHudMessages( const char *pszElementName );
	~CHudMessages( );

	void StartMessageMode( int iType );
	vgui::Panel *GetInputPanel( void );

	void PrintChat( CColoredString &String, int iType );
	void PrintChat( const char *pszMessage );
	void PrintRadio( const char *pszMessage, int iEntID );

	static bool IsValidSayType( int iType );

	void MsgFunc_FFMsg( bf_read &msg );

private:
	void ApplySettings( KeyValues *pResourceData );
	void FireGameEvent( IGameEvent *pEvent );

	void Init( void );
	void Update( void );
	void RecaculateInput( void );
	void OnThink( void );

	const char *FontName( void ) const { return "ChatFont"; }

private:
	CHudChatInputLine *m_pInputLine;
};

//=========================================================
//=========================================================
extern CHudMessages *g_pPlayerChat;

#endif // HUD_MESSAGES_H
