//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "voice_commands_data.h"
#include "hud_actionmenu.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define MAIN_SECTION 0

//=========================================================
//=========================================================
void SendVoiceChat(int iVoiceGrp, int iID)
{
	char szBuffer[128];
	Q_snprintf(szBuffer, sizeof(szBuffer), "voicechat %i %i", iVoiceGrp, iID);

	engine->ServerCmd(szBuffer);
}

//=========================================================
//=========================================================
class CVoiceMenu : public IActionMenu
{
public:
	void Setup(void)
	{
		CHudActionMenu *pActionMenu = CHudActionMenu::GetActionMenu();

		if(!pActionMenu)
			return;
		
		// add section
		pActionMenu->AddSection(MAIN_SECTION, NULL);

		// add items
		for(int i = 0; i < GetVoiceCount(); i++)
			pActionMenu->AddItem(MAIN_SECTION, GetVoiceText(i));
	}

	bool Action(int iItemID)
	{
		SendVoiceChat(GetVoiceType(), iItemID);
		return true;
	}

protected:
	virtual int GetVoiceType(void) const = 0;
	virtual int GetVoiceCount(void) const = 0;
	virtual const char *GetVoiceText(int iID) const = 0;
};

//=========================================================
//=========================================================
class CUnitVoiceMenu : public CVoiceMenu
{
public:
	int GetActionID(void) const
	{
		return ACTIONMENU_UNIT;
	}

	const char *GetTitle(void) const
	{
		return "Unit Commands";
	}

private:
	int GetVoiceType(void) const
	{
		return VOICEGRP_UNITCMDS;
	}

	int GetVoiceCount(void) const
	{
		return GRPUNIT_COUNT;
	}

	const char *GetVoiceText(int iID) const
	{
		return g_pszUnitGroupText[iID];
	}
};

DECLARE_ACTIONMENU(ACTIONMENU_UNIT, CUnitVoiceMenu);

//=========================================================
//=========================================================
class CChatVoiceMenu : public CVoiceMenu
{
public:
	int GetActionID(void) const
	{
		return ACTIONMENU_CHAT;
	}

	const char *GetTitle(void) const
	{
		return "Voice Chat";
	}

private:
	int GetVoiceType(void) const
	{
		return VOICEGRP_CHAT;
	}

	int GetVoiceCount(void) const
	{
		return GRPCHAT_COUNT;
	}

	const char *GetVoiceText(int iID) const
	{
		return g_pszChatGroupText[iID];
	}
};

DECLARE_ACTIONMENU(ACTIONMENU_CHAT, CChatVoiceMenu);

//=========================================================
//=========================================================
class CMiscVoiceMenu : public CVoiceMenu
{
public:
	int GetActionID(void) const
	{
		return ACTIONMENU_MISC;
	}

	const char *GetTitle(void) const
	{
		return "Voice Responses";
	}

private:
	int GetVoiceType(void) const
	{
		return VOICEGRP_MISC;
	}

	int GetVoiceCount(void) const
	{
		return GRPMISC_COUNT;
	}

	const char *GetVoiceText(int iID) const
	{
		return g_pszMiscGroupText[iID];
	}
};

DECLARE_ACTIONMENU(ACTIONMENU_MISC, CMiscVoiceMenu);

//=========================================================
//=========================================================
CON_COMMAND(unitvoice, "Bring up Unit Voice")
{
	CHudActionMenu *pActionMenu = CHudActionMenu::GetActionMenu();

	if(pActionMenu)
		pActionMenu->ShowAction(ACTIONMENU_UNIT);
}

//=========================================================
//=========================================================
CON_COMMAND(chatvoice, "Bring up Chat Voice")
{
	CHudActionMenu *pActionMenu = CHudActionMenu::GetActionMenu();

	if(pActionMenu)
		pActionMenu->ShowAction(VOICEGRP_CHAT);
}

//=========================================================
//=========================================================
CON_COMMAND(miscvoice, "Bring up Misc Voice")
{
	CHudActionMenu *pActionMenu = CHudActionMenu::GetActionMenu();

	if(pActionMenu)
		pActionMenu->ShowAction(VOICEGRP_MISC);
}
