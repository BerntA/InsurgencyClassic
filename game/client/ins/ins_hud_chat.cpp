//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_hud_chat.h"
#include "hud_macros.h"
#include "text_message.h"
#include "vguicenterprint.h"
#include "vgui/ILocalize.h"
#include "c_team.h"
#include "c_playerresource.h"

DECLARE_HUDELEMENT(CHudChat);

DECLARE_HUD_MESSAGE(CHudChat, SayText);
DECLARE_HUD_MESSAGE(CHudChat, SayText2);
DECLARE_HUD_MESSAGE(CHudChat, TextMsg);

void CHudChatLine::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CHudChatInputLine::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

CHudChat::CHudChat(const char* pElementName) : BaseClass(pElementName)
{

}

void CHudChat::CreateChatInputLine(void)
{
	m_pChatInput = new CHudChatInputLine(this, "ChatInputLine");
	m_pChatInput->SetVisible(false);
}

void CHudChat::CreateChatLines(void)
{
	m_ChatLine = new CHudChatLine(this, "ChatLine1");
	m_ChatLine->SetVisible(false);
}

void CHudChat::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CHudChat::Init(void)
{
	BaseClass::Init();

	HOOK_HUD_MESSAGE(CHudChat, SayText);
	HOOK_HUD_MESSAGE(CHudChat, SayText2);
	HOOK_HUD_MESSAGE(CHudChat, TextMsg);
}

void CHudChat::Reset(void)
{
}

int CHudChat::GetChatInputOffset(void)
{
	if (m_pChatInput->IsVisible())
	{
		return m_iFontHeight;
	}
	else
		return 0;
}

Color CHudChat::GetClientColor(int clientIndex)
{
	if (clientIndex == 0) // console msg
	{
		return g_ColorYellow;
	}
	else if (g_PR)
	{
		switch (g_PR->GetTeam(clientIndex))
		{
		case TEAM_ONE: return g_ColorBlue;
		case TEAM_TWO: return g_ColorRed;
		case TEAM_SPECTATOR: return g_ColorDarkGreen;
		default: return g_ColorYellow;
		}
	}

	return g_ColorYellow;
}