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

namespace vgui
{
	class IScheme;
};

//=========================================================
//=========================================================
class CHudMessages : public CHudMessagesBase, public CDeadHUDHelper, public IINSChatMessages
{
	DECLARE_CLASS_SIMPLE(CHudMessages, CHudMessagesBase);

public:
	CHudMessages(const char* pszElementName);
	~CHudMessages();

	void PrintRadio(const char* pszMessage, int iEntID);

	void MsgFunc_FFMsg(bf_read& msg);

private:
	void ApplySettings(KeyValues* pResourceData);
	void FireGameEvent(IGameEvent* pEvent);

	void Init(void);
	void Update(void);
	void OnThink(void);

	const char* FontName(void) const { return "ChatFont"; }
};

//=========================================================
//=========================================================
extern CHudMessages* g_pPlayerChat;

#endif // HUD_MESSAGES_H