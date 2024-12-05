//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: HUD Target ID element
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_baseplayer.h"
#include "c_playerresource.h"
#include "vgui_entitypanel.h"
#include "iclientmode.h"
#include "vgui/ilocalize.h"
#include "gamerules.h"
#include "c_team.h"
#include <vgui/ischeme.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PLAYER_HINT_DISTANCE	150
#define PLAYER_HINT_DISTANCE_SQ	(PLAYER_HINT_DISTANCE*PLAYER_HINT_DISTANCE)

static ConVar hud_centerid("hud_centerid", "1");
static ConVar hud_showtargetid("hud_showtargetid", "1");

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTargetID : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CTargetID, vgui::Panel);

public:
	CTargetID(const char *pElementName);
	void Init(void);
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void Paint(void);
	void VidInit( void );

private:
	vgui::HFont		m_hFont;
	int				m_iLastEntIndex;
	float			m_flLastChangeTime;
};

DECLARE_HUDELEMENT(CTargetID);

using namespace vgui;

//=========================================================
//=========================================================
CTargetID::CTargetID(const char *pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "TargetID")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;
}

//=========================================================
//=========================================================
void CTargetID::Init(void)
{
}

//=========================================================
//=========================================================
void CTargetID::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont("Default", IsProportional());

	SetPaintBackgroundEnabled(false);
}

//=========================================================
//=========================================================
void CTargetID::VidInit()
{
	CHudElement::VidInit();

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;
}

//=========================================================
//=========================================================
#define MAX_ID_STRING 256
Color g_TargetIDColor(255, 0, 0, 255);

void CTargetID::Paint()
{
	wchar_t sIDString[MAX_ID_STRING];
	sIDString[0] = 0;

	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pLocalPlayer || !pLocalPlayer->OnPlayTeam())
		return;

	// Get our target's ent index
	int iEntIndex = pLocalPlayer->GetIDTarget();

	// Didn't find one?
	if (!iEntIndex)
	{
		// Check to see if we should clear our ID
		if (m_flLastChangeTime && (gpGlobals->curtime > (m_flLastChangeTime + 0.5)))
		{
			m_flLastChangeTime = 0;
			sIDString[0] = 0;
			m_iLastEntIndex = 0;
		}
		else
		{
			// Keep re-using the old one
			iEntIndex = m_iLastEntIndex;
		}
	}
	else
	{
		m_flLastChangeTime = gpGlobals->curtime;
	}

	// Is this an entindex sent by the server?
	if(iEntIndex)
	{
		C_BasePlayer *pTargetPlayer = static_cast<C_BasePlayer*>(cl_entitylist->GetEnt(iEntIndex));
		Assert(pTargetPlayer->OnPlayTeam() && pTargetPlayer->InSameTeam(pLocalPlayer));

		wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
		wchar_t wszHealthText[10];

		vgui::localize()->ConvertANSIToUnicode(pTargetPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName));

		_snwprintf(wszHealthText, ARRAYSIZE(wszHealthText) - 1, L"%.0f%%",  ((float)pTargetPlayer->GetHealth() / (float)pTargetPlayer->GetMaxHealth()));
		wszHealthText[ARRAYSIZE(wszHealthText)-1] = '\0';

		vgui::localize()->ConstructString(sIDString, sizeof(sIDString), vgui::localize()->Find("#INS_PlayerID"), 2, wszPlayerName, wszHealthText);

		if(sIDString[0])
		{
			int iWide, iTall;
			int iYPos = YRES(260);
			int iXPos = XRES(10);

			vgui::surface()->GetTextSize(m_hFont, sIDString, iWide, iTall);

			if(hud_centerid.GetInt() == 0)
				iYPos = YRES(420);
			else
				iXPos = (ScreenWidth() - iWide) / 2;
			
			vgui::surface()->DrawSetTextFont(m_hFont);
			vgui::surface()->DrawSetTextPos(iXPos, iYPos);
			vgui::surface()->DrawSetTextColor(g_TargetIDColor);
			vgui::surface()->DrawPrintText(sIDString, wcslen(sIDString));
		}
	}
}