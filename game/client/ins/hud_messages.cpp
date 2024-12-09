//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include <vgui/vgui.h>
#include <vgui/ivgui.h>
#include "iclientmode.h"
#include "hud_messages.h"
#include "hud_macros.h"

#include <vgui/ilocalize.h>

#include "c_ins_obj.h"
#include "ins_obj_shared.h"
#include "ins_player_shared.h"
#include "play_team_shared.h"
#include "c_playerresource.h"

#include "ins_gamerules.h"

#include "clientmode_shared.h"
#include "basic_colors.h"
#include "vguicenterprint.h"
#include "text_message.h"
#include "ins_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar shownamechange("cl_shownamechange", "1");
ConVar showdisconnects("cl_showdisconnects", "1");

CHudMessages* g_pPlayerChat = NULL;

CHudMessages::CHudMessages(const char* pszElementName) :
	CHudMessagesBase(pszElementName, "HudMessages"),
	CDeadHUDHelper(this)
{
	// set global var
	g_pPlayerChat = this;

	// register
	GetINSHUDHelper()->RegisterChatMessages(this);

	// listen to certain game events
	gameeventmanager->AddListener(this, "player_connect", false);
	gameeventmanager->AddListener(this, "player_changename", false);
	gameeventmanager->AddListener(this, "player_death", false);
	gameeventmanager->AddListener(this, "player_disconnect", false);
	gameeventmanager->AddListener(this, "player_team", false);
}

//=========================================================
//=========================================================
DECLARE_HUDELEMENT(CHudMessages);
DECLARE_HUD_MESSAGE(CHudMessages, FFMsg);

//=========================================================
//=========================================================
CHudMessages::~CHudMessages()
{
	g_pPlayerChat = NULL;
}

//=========================================================
//=========================================================
void CHudMessages::Init(void)
{
	BaseClass::Init();
	HOOK_HUD_MESSAGE(CHudMessages, FFMsg);
}

//=========================================================
//=========================================================
void CHudMessages::ApplySettings(KeyValues* pResourceData)
{
	BaseClass::ApplySettings(pResourceData);
	DeadInit(pResourceData);
}

//=========================================================
//=========================================================
void CHudMessages::PrintRadio(const char* pszMessage, int iEntID)
{
	CColoredString RadioString;

	// setup vars
	const char* pszName = NULL;
	int iTeamID, iTeamColor;

	pszName = g_PR->GetPlayerName(iEntID);
	iTeamID = g_PR->GetTeamID(iEntID);

	if (!pszName || !IsPlayTeam(iTeamID))
	{
		Assert(false);
		return;
	}

	iTeamColor = (iTeamID == TEAM_ONE) ? CLOOKUP_TEAM_ONE : CLOOKUP_TEAM_TWO;

	// write
	RadioString.Add("(RADIO) ", COLOR_DGREY);
	RadioString.Add(pszName, iTeamColor);
	RadioString.Add(": ", iTeamColor);
	RadioString.Add(pszMessage, COLOR_ORANGE);

	// print
	Print(RadioString);
}

//=========================================================
//=========================================================
void CHudMessages::Update(void)
{
	BaseClass::Update();
}

//=========================================================
//=========================================================
void CHudMessages::FireGameEvent(IGameEvent* pEvent)
{
	C_PlayerResource* pPR = g_PR;
	if (!pPR)
		return;

	// find generic data
	int iUserID, iPlayerID, iTeamID, iTeamColorID;
	iPlayerID = 0;
	iTeamID = INVALID_TEAM;
	iTeamColorID = CLOOKUP_INVALID;

	iUserID = pEvent->GetInt("userid");

	if (iUserID != 0)
	{
		iPlayerID = engine->GetPlayerForUserID(iUserID);
		iTeamID = pPR->GetTeamID(iPlayerID);
		iTeamColorID = UTIL_TeamColorLookup(iTeamID);
	}

	// create the string
	CColoredString EventString;
	EventString.Add("* ");

	// handle the event
	const char* pszEvent = pEvent->GetName();

	if (FStrEq(pszEvent, "player_connect"))
	{
		EventString.Add(pEvent->GetString("name"));
		EventString.Add(" has Connected");
	}
	else if (FStrEq(pszEvent, "player_changename"))
	{
		if (!shownamechange.GetBool())
			return;

		EventString.Add(pEvent->GetString("oldname"), iTeamColorID);
		EventString.Add(" is now known as ");
		EventString.Add(pEvent->GetString("newname"), iTeamColorID);
	}
	else if (FStrEq(pszEvent, "player_disconnect"))
	{
		if (!showdisconnects.GetBool())
			return;

		EventString.Add(pPR->GetPlayerName(iPlayerID), iTeamColorID);
		EventString.Add(" has Disconnected");
	}
	else if (FStrEq(pszEvent, "player_death"))
	{
		C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();

		if ((iPlayerID == pLocalPlayer->entindex()) || (iTeamID != pLocalPlayer->GetTeamID()))
			return;

		// ... find type
		int iType = pEvent->GetInt("type");

		// ... handle by type
		EventString.Add(pPR->GetPlayerName(iPlayerID), iTeamColorID);

		switch (iType)
		{
		case PDEATHTYPE_SOULSTOLEN:
		{
			EventString.Add(" had their Soul Stolen");
			break;
		}

		case PDEATHTYPE_SELF:
		{
			EventString.Add(" Commited Suicide");
			break;
		}

		case PDEATHTYPE_TELEFRAG:
		{
			EventString.Add(" was Telefragged");
			break;
		}

		case PDEATHTYPE_FF:
		{
			EventString.Add(" was TK'ed");
			break;
		}

		default:
		{
			EventString.Add(" was KIA");
			break;
		}
		}

		// ... add extra info for friendly kill
		if (iType == PDEATHTYPE_FF)
		{
			int iAttackerID = engine->GetPlayerForUserID(pEvent->GetInt("attacker"));

			if (iAttackerID == 0)
				return;

			EventString.Add(" by ");
			EventString.Add(pPR->GetPlayerName(iAttackerID), iTeamColorID);
		}
	}
	else if (FStrEq(pszEvent, "player_team"))
	{
		int iOldTeamID, iTeamID;
		iOldTeamID = pEvent->GetInt("oldteam");
		iTeamID = pEvent->GetInt("team");

		C_Team* pTeam = GetGlobalTeam(iTeamID);

		if (!pTeam)
			return;

		EventString.Add(pPR->GetPlayerName(iPlayerID), UTIL_TeamColorLookup(iOldTeamID));
		EventString.Add(" has joined the ");
		EventString.Add(pTeam->GetName(), UTIL_TeamColorLookup(iTeamID));
	}
	else
	{
		return;
	}

	Print(EventString);
}

//=========================================================
//=========================================================
void CHudMessages::OnThink(void)
{
	BaseClass::OnThink();
	DeadUpdate();
}

//=========================================================
//=========================================================
void CHudMessages::MsgFunc_FFMsg(bf_read& msg)
{
	C_PlayerResource* pPR = g_PR;

	if (!pPR)
		return;

	CColoredString FFMessage;

	int iPlayerID = msg.ReadShort();

	if (iPlayerID == 0)
		return;

	FFMessage.Add(pPR->GetPlayerName(iPlayerID), UTIL_TeamColorLookup(pPR->GetTeamID(iPlayerID)));
	FFMessage.Add(" Attacked a Teammate");

	Print(FFMessage);
}