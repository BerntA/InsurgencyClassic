//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapon selection handling
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud_weaponselection.h"
#include "vgui/ilocalize.h"
#include "hud_macros.h"
#include "in_buttons.h"
#include "iinput.h"
#include "vgui_entitypanel.h"
#include "clientmode_shared.h"
#include <vgui/ivgui.h>
#include "basic_colors.h"
#include "weapon_ballistic_base.h"
#include "ins_player_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// TODO: needs to use "UseDeploy" for weapons blah

//=========================================================
//=========================================================
#define WPNSELECT_PATH			"sprites/HUD/wpnselect/"

#define WPNSELECT_BG			"bg"
#define WPNSELECT_AMMO_X		"ammo_x"

#define WPNSELECT_WIDE			128
#define WPNSELECT_TALL			40

const char *g_pszSelectFireTex[SELECTFIRE_COUNT] = {
	"sf_full",		// SELECTFIRE_RANGED
	"sf_single",	// SELECTFIRE_SINGLE
	"sf_burst",		// SELECTFIRE_BURST
	"sf_full",		// SELECTFIRE_FULL
};

#define WPNSELECT_XBARPOS		42
#define WPNSELECT_BARWIDE		81
#define WPNSELECT_XBARGAP		5
#define WPNSELECT_YBARGAP		15
#define WPNSELECT_BGCOLOR		Color(52, 52, 52, 192)

#define WPNSELECT_AMMONUM_XPOS	24
#define WPNSELECT_AMMONUM_YPOS	86

#define WPNSELECT_SHOWTIME		2.5f

//=========================================================
//=========================================================
DECLARE_HUD_COMMAND_NAME(CHudWeaponSelection, Slot1, "CHudWeaponSelection");
DECLARE_HUD_COMMAND_NAME(CHudWeaponSelection, Slot2, "CHudWeaponSelection");
DECLARE_HUD_COMMAND_NAME(CHudWeaponSelection, Slot3, "CHudWeaponSelection");
DECLARE_HUD_COMMAND_NAME(CHudWeaponSelection, Slot4, "CHudWeaponSelection");
DECLARE_HUD_COMMAND_NAME(CHudWeaponSelection, Slot5, "CHudWeaponSelection");
DECLARE_HUD_COMMAND_NAME(CHudWeaponSelection, Slot6, "CHudWeaponSelection");
DECLARE_HUD_COMMAND_NAME(CHudWeaponSelection, Slot7, "CHudWeaponSelection");
DECLARE_HUD_COMMAND_NAME(CHudWeaponSelection, Close, "CHudWeaponSelection");
DECLARE_HUD_COMMAND_NAME(CHudWeaponSelection, NextWeapon, "CHudWeaponSelection");
DECLARE_HUD_COMMAND_NAME(CHudWeaponSelection, PrevWeapon, "CHudWeaponSelection");
DECLARE_HUD_COMMAND_NAME(CHudWeaponSelection, LastWeapon, "CHudWeaponSelection");

HOOK_COMMAND(slot1, Slot1);
HOOK_COMMAND(slot2, Slot2);
HOOK_COMMAND(slot3, Slot3);
HOOK_COMMAND(slot4, Slot4);
HOOK_COMMAND(slot5, Slot5);
HOOK_COMMAND(slot6, Slot6);
HOOK_COMMAND(slot7, Slot7);
HOOK_COMMAND(cancelselect, Close);
HOOK_COMMAND(invnext, NextWeapon);
HOOK_COMMAND(invprev, PrevWeapon);
HOOK_COMMAND(lastinv, LastWeapon);

//=========================================================
//=========================================================
DECLARE_HUDELEMENT(CHudWeaponSelection);
DECLARE_HUD_MESSAGE(CHudWeaponSelection, FireMode);

CHudWeaponSelection::CHudWeaponSelection(const char *pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudWeaponSelection")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetPaintBackgroundEnabled(false);

	SetHiddenBits(HIDEHUD_SPECTATOR);
}

//=========================================================
//=========================================================
void CHudWeaponSelection::Init(void)
{
	HOOK_HUD_MESSAGE(CHudWeaponSelection, FireMode);

	Reset();

	gWR.Init();
}

//=========================================================
//=========================================================
void CHudWeaponSelection::VidInit(void)
{
	char szBuffer[128];

	// load bg tex
	CreateTexPath(WPNSELECT_PATH, WPNSELECT_BG, szBuffer, sizeof(szBuffer));
	m_iBGTexID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_iBGTexID, szBuffer, false, false);

	// load x tex
	CreateTexPath(WPNSELECT_PATH, WPNSELECT_AMMO_X, szBuffer, sizeof(szBuffer));
	m_iXTexID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_iXTexID, szBuffer, false, false);

	// load select fire tex
	for(int i = 0; i < SELECTFIRE_COUNT; i++)
	{
		CreateTexPath(WPNSELECT_PATH, g_pszSelectFireTex[i], szBuffer, sizeof(szBuffer));
		m_iSelectFireTexID[i] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_iSelectFireTexID[i], szBuffer, false, false);
	}

	// load weapon sprites
	gWR.LoadAllWeaponSprites();

	// reset
	Reset();
}

//=========================================================
//=========================================================
void CHudWeaponSelection::LevelInit(void)
{
	m_flSoonestNextInput = 0.0f;
	m_flShowTime = 0.0f;
}

//=========================================================
//=========================================================
void CHudWeaponSelection::Reset(void)
{
	m_flShowTime = 0.0f;
	m_bSelectionVisible = false;

	gWR.Reset();
}

//=========================================================
//=========================================================
void CHudWeaponSelection::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_AmmoFont = pScheme->GetFont("DefaultBig", true);
	m_BarFont = pScheme->GetFont("Default", true);

	m_iBGWide = scheme()->GetProportionalScaledValue(WPNSELECT_WIDE);
	m_iBGTall = scheme()->GetProportionalScaledValue(WPNSELECT_TALL);

	m_iAmmoNumXPos = scheme()->GetProportionalScaledValue(WPNSELECT_AMMONUM_XPOS);
	m_iAmmoNumYPos = scheme()->GetProportionalScaledValue(WPNSELECT_AMMONUM_YPOS);

	m_iBarXPos = scheme()->GetProportionalScaledValue(WPNSELECT_XBARPOS);
	m_iBarWide = scheme()->GetProportionalScaledValue(WPNSELECT_BARWIDE);
	m_iBarXGap = scheme()->GetProportionalScaledValue(WPNSELECT_XBARGAP);
	m_iBarYGap = scheme()->GetProportionalScaledValue(WPNSELECT_YBARGAP);

	m_iBGYOffset = m_iBarYGap * WPNSELECT_COUNT;
}

//=========================================================
//=========================================================
void CHudWeaponSelection::ProcessInput(void)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	if(pPlayer->IsInVGuiInputMode())
	{
		if(gHUD.m_iKeyBits & IN_ATTACK)
		{
			gHUD.m_iKeyBits &= ~IN_ATTACK;
			::input->ClearInputButton( IN_ATTACK );

			SetSelection(false);
		}

		return;
	}

	if(gHUD.m_iKeyBits & IN_ATTACK)
	{
		if(IsSelecting())
		{
			gHUD.m_iKeyBits &= ~(IN_ATTACK);
			::input->ClearInputButton( IN_ATTACK );

			SelectWeapon();
		}
	}
}

//=========================================================
//=========================================================
bool CHudWeaponSelection::ShouldDraw(void)
{
	if(!CHudElement::ShouldDraw())
		return false;

	CBaseCombatWeapon *pActiveWeapon = GetActiveWeapon();

	// can always draw when holding status (with a valid weapon)
	if((m_flShowTime > gpGlobals->curtime || g_bInStatus) && pActiveWeapon)
		return true;

	// work out if we can select etc
	if(pActiveWeapon && !pActiveWeapon->IsAllowedToSwitch())
	{
		if(IsSelecting())
			SetSelection(false);

		return false;
	}

	return m_bSelectionVisible;
}

//=========================================================
//=========================================================
void CHudWeaponSelection::OnThink(void)
{
	UpdateSelectedWeapon();

	if(!IsSelecting())
		return;

	if(!UpdateWeapons())
		SetSelection(false);
}

//=========================================================
//=========================================================
void CHudWeaponSelection::Paint(void)
{
	C_BaseCombatWeapon *pWeapon = GetSelectedWeapon();

	if(!IsSelecting())
	{
		C_BaseCombatWeapon *pActiveWeapon = GetActiveWeapon();

		if(pActiveWeapon != pWeapon)
		{
			SetSelectedWeapon(pActiveWeapon);
			pWeapon = pActiveWeapon;
		}
	}

	surface()->DrawSetColor(COLOR_WHITE);

	surface()->DrawSetTexture(m_iBGTexID);
	surface()->DrawTexturedRect(0, m_iBGYOffset, m_iBGWide, m_iBGTall + m_iBGYOffset);

	if(pWeapon && m_SelectedWeapon.m_bDrawX)
	{
		surface()->DrawSetTexture(m_iXTexID);
		surface()->DrawTexturedRect(0, m_iBGYOffset, m_iBGWide, m_iBGTall + m_iBGYOffset);
	}

	if(pWeapon && m_SelectedWeapon.m_iAmmo != AMMO_INVALID)
	{
		wchar_t wszAmmo[2];
		_snwprintf(wszAmmo, sizeof(wszAmmo), L"%i", m_SelectedWeapon.m_iAmmo);

		surface()->DrawSetTextColor(COLOR_WHITE);
		surface()->DrawSetTextFont(m_AmmoFont);
		surface()->DrawSetTextPos(m_iAmmoNumXPos, m_iAmmoNumYPos);
		surface()->DrawPrintText(wszAmmo, 1);
	}

	if(pWeapon && m_SelectedWeapon.m_iSelectedFire != SELECTFIRE_INVALID)
	{
		surface()->DrawSetTexture(m_iSelectFireTexID[m_SelectedWeapon.m_iSelectedFire]);
		surface()->DrawTexturedRect(0, m_iBGYOffset, m_iBGWide, m_iBGTall + m_iBGYOffset);
	}

	if(pWeapon)
	{
		const CHudTexture *pSelectionTex = pWeapon->GetSpriteSelection();

		if(pSelectionTex)
		{
			Color DrawColor = (m_SelectedWeapon.m_bHasAmmo == 0 ? COLOR_RED : COLOR_WHITE);
			pSelectionTex->DrawSelf(0, m_iBGYOffset, m_iBGWide, m_iBGTall, DrawColor);
		}
	}

	if(pWeapon && IsSelecting())
	{
		int iYOffset = m_iBGYOffset - (m_iBarYGap * m_iWeaponNamesCount);

		// draw background
       	surface()->DrawSetColor(WPNSELECT_BGCOLOR);
		surface()->DrawFilledRect(m_iBarXPos, iYOffset, m_iBarXPos + m_iBarWide, m_iBGYOffset);

		// draw lines
		surface()->DrawSetColor(COLOR_DGREY);
		surface()->DrawFilledRect(m_iBarXPos, iYOffset, m_iBarXPos + 1, m_iBGYOffset);
		surface()->DrawFilledRect(m_iBarXPos, iYOffset, m_iBarXPos + m_iBarWide, iYOffset + 1);
		surface()->DrawFilledRect(m_iBarXPos + m_iBarWide - 1, iYOffset, m_iBarXPos + m_iBarWide, m_iBGYOffset);

		// draw text
		int iLineGap, iYPos, iXPos;
		iLineGap = RoundFloatToInt(m_iBarYGap * 0.25f);
		iYPos = m_iBGYOffset - m_iBarYGap + iLineGap;
		iXPos = m_iBarXPos + m_iBarXGap;

		for(int i = 1; i < WPNSELECT_COUNT; i++)
		{
			wchar_t *pwszWeaponName = m_wszWeaponNames[i];

			if(*pwszWeaponName == L'\0')
				continue;

			// draw text
			surface()->DrawSetTextColor(COLOR_WHITE);
			surface()->DrawSetTextFont(m_BarFont);
			surface()->DrawSetTextPos(iXPos, iYPos);
			surface()->DrawPrintText(pwszWeaponName, m_iWeaponNamesLength[i]);

			// draw lines inbetween
			if(i != m_iWeaponNamesCount)
			{
				surface()->DrawSetColor(COLOR_VDGREY);
				surface()->DrawFilledRect(m_iBarXPos, iYPos - iLineGap - 1, m_iBarXPos + m_iBarWide, iYPos - iLineGap);
			}

			iYPos -= m_iBarYGap;
		}
	}
}

//=========================================================
//=========================================================
void CHudWeaponSelection::CancelWeaponSelection(void)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer || !ShouldDraw())
		return;

	SetSelectedWeapon(pPlayer->GetActiveWeapon());

	SetSelection(false);

	pPlayer->EmitSound("Player.WeaponSelectionClose");
}

//=========================================================
//=========================================================
bool CHudWeaponSelection::SetSelectedWeapon(C_BaseCombatWeapon *pSelectedWeapon, bool bStartSelect)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return false;

	// ensure we can switch
	C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();

	if(pWeapon && !pWeapon->IsAllowedToSwitch())
		return false;

	// update weapons
	m_hWeapons[0] = pSelectedWeapon;

	if(!UpdateWeapons())
		return false;

	// update current data
	UpdateSelectedWeapon();

	// set selecting
	if(!IsSelecting() && bStartSelect)
		SetSelection(true);

	return true;
}

//=========================================================
//=========================================================
void CHudWeaponSelection::UpdateSelectedWeapon(void)
{
	C_BaseCombatWeapon *pWeapon = GetSelectedWeapon();

	if(!pWeapon)
		return;

	//pWeapon->UpdateWeaponHUD(&m_SelectedWeapon);
}

//=========================================================
//=========================================================
void CHudWeaponSelection::SelectWeapon(void)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	C_BaseCombatWeapon *pSelectedWeapon = GetSelectedWeapon();

    if(pSelectedWeapon)
	{
		::input->MakeWeaponSelection(pSelectedWeapon);
		SetSelectedWeapon(NULL);
	}

	SetSelection(false);
}

//=========================================================
//=========================================================
void CHudWeaponSelection::SelectType(int iWeaponSlot)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	C_BaseCombatWeapon *pWeapon = pPlayer->GetWeapon(iWeaponSlot);

	if(!pWeapon)
		return;

	SetSelectedWeapon(pWeapon, true);
}

//=========================================================
//=========================================================
void CHudWeaponSelection::MsgFunc_FireMode(bf_read &msg)
{
	if(IsSelecting())
		return;

	m_flShowTime = gpGlobals->curtime + WPNSELECT_SHOWTIME;
}

//=========================================================
//=========================================================
void CHudWeaponSelection::CycleToNextWeapon(void)
{
	CycleThroughWeapons(false);
}

//=========================================================
//=========================================================
void CHudWeaponSelection::CycleToPrevWeapon(void)
{
	CycleThroughWeapons(true);
}

//=========================================================
//=========================================================
void CHudWeaponSelection::CycleThroughWeapons(bool bForward)
{
	if(gpGlobals->curtime <= m_flSoonestNextInput)
		return;

	// find local player
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	// update weapons
	if(!UpdateWeapons())
		return;

	// find selected weapon
	C_BaseCombatWeapon *pSelectedWeapon, *pNextWeapon;
	pSelectedWeapon = GetSelectedWeapon();
	pNextWeapon = NULL;

	// if we don't have a weapon selected, set the next one to the player's active weapon
	if(!pSelectedWeapon)
	{
		C_BaseCombatWeapon *pActiveWeapon = pPlayer->GetActiveWeapon();

		if(pActiveWeapon)
			pNextWeapon = pActiveWeapon;
		else
			return;
	}
	else
	{
		// ... otherwise, find the next one in the list
		if(bForward)
		{
			pNextWeapon = m_hWeapons[1];

			if(!pNextWeapon)
				return;
		}
		else
		{
			int iCurrentWeaponID, iCurrentID;
			iCurrentWeaponID = GetSelectedWeaponID();
			iCurrentID = MAX_PWEAPONS;

			if(iCurrentWeaponID == WEAPON_INVALID)
				return;

			bool bFoundCurrentWeapon = false;

			while(!pNextWeapon)
			{
				// find previous weapon
				if(iCurrentID > 0)
				{
					iCurrentID--;
				}
				else
				{
					iCurrentID = MAX_PWEAPONS - 1;

					// can't find it!
					if(!bFoundCurrentWeapon)
						return;
				}

				// find weapon
				C_BaseCombatWeapon *pWeapon = pPlayer->GetWeapon(iCurrentID);

				if(!pWeapon || !pWeapon->CanBeSelected())
					continue;

				// see if the current weapon is our current selection
				if(!bFoundCurrentWeapon)
				{
					if(pWeapon->GetWeaponID() == iCurrentWeaponID)
						bFoundCurrentWeapon = true;

					continue;
				}

				// found it
				pNextWeapon = pWeapon;
			}
		}
	}

	// start selection process
	if(!pNextWeapon)
		return;

	// ensure that the player can't scroll too fast
	m_flSoonestNextInput = gpGlobals->curtime + 0.1f;

	// set the selected weapon
	if(!SetSelectedWeapon(pNextWeapon, true))
		return;

	// print log when debugging
#ifdef _DEBUG
	Msg("Scrolled Weapon: %s\n", pNextWeapon->GetClassname());
#endif

	// emit a clicky move sound
	pPlayer->EmitSound("Player.WeaponSelectionMoveSlot");
}

//=========================================================
//=========================================================
void CHudWeaponSelection::SwitchToLastWeapon(void)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	::input->MakeWeaponSelection(pPlayer->GetLastWeapon());
}

//=========================================================
//=========================================================
void CHudWeaponSelection::SetSelection(bool bState)
{
	if(bState)
		m_flShowTime = 0.0f;

	m_bSelectionVisible = bState;
}

//=========================================================
//=========================================================
C_BaseCombatWeapon *CHudWeaponSelection::GetSelectedWeapon(void) const
{
	return m_hWeapons[0];
}

//=========================================================
//=========================================================
int CHudWeaponSelection::GetSelectedWeaponID(void) const
{
	C_BaseCombatWeapon *pWeapon = GetSelectedWeapon();

	if(!pWeapon)
		return WEAPON_INVALID;

	return pWeapon->GetWeaponID();
}

//=========================================================
//=========================================================
bool CHudWeaponSelection::UpdateWeapons(void)
{
	if(!UpdateWeaponHandles())
		return false;

	UpdateWeaponNames();
	return true;
}

//=========================================================
//=========================================================
bool CHudWeaponSelection::UpdateWeaponHandles(void)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return false;

	bool bFinishedUpdate, bFoundCurrent;
	int iCurrentWeaponID, iCurrentID, iRecordID;

	bFinishedUpdate = bFoundCurrent = false;
	iCurrentWeaponID = GetSelectedWeaponID();
	iCurrentID = 0;
	iRecordID = 1;

	// if invalid, end now
	if(iCurrentWeaponID == WEAPON_INVALID)
		return true;

	// update weapons
	while(!bFinishedUpdate)
	{
		C_BaseCombatWeapon *pWeapon = pPlayer->GetWeapon(iCurrentID);

		// find next ID
		if(iCurrentID + 1 < MAX_PWEAPONS)
		{
			iCurrentID++;
		}
		else
		{
			iCurrentID = 0;

			// stop now if we haven't found it
			if(!bFoundCurrent)
				return false;
		}

		// don't do anything if its broken
		if(!pWeapon || !pWeapon->CanBeSelected())
			continue;

		bool bCurrentWeapon = iCurrentWeaponID == pWeapon->GetWeaponID();

		// record if we've started
		if(bFoundCurrent && !bCurrentWeapon)
		{
			m_hWeapons[iRecordID] = pWeapon;
			iRecordID++;
		}

		// if this is the current weapon ...
		if(bCurrentWeapon)
		{
			// if we have not found it before, start recording and reset list
			if(!bFoundCurrent)
			{
				for(int i = 1; i < WPNSELECT_COUNT; i++)
					m_hWeapons[i].Term();

				bFoundCurrent = true;
			}
			else
			{
				// ... but if we have, we're done if they've found the current weapon
				// but we've failed if it hasn't
				return bFoundCurrent;
			}
		}

		// stop if we've finished recording
		if(iRecordID >= WPNSELECT_COUNT)
			bFinishedUpdate = true;
	}

	return true;
}

//=========================================================
//=========================================================
void CHudWeaponSelection::UpdateWeaponNames(void)
{
	m_iWeaponNamesCount = 0;

	for(int i = 1; i < WPNSELECT_COUNT; i++)
	{
		C_BaseCombatWeapon *pWeapon = m_hWeapons[i];

		if(!pWeapon)
			continue;

		const char *pszName = pWeapon->GetPrintName();

		if(!pszName || *pszName == '\0')
			continue;

		m_iWeaponNamesLength[i] = Q_strlen(pszName);
		localize()->ConvertANSIToUnicode(pszName, m_wszWeaponNames[i], sizeof(m_wszWeaponNames[i]));

		m_iWeaponNamesCount++;
	}
}

//=========================================================
//=========================================================
void CHudWeaponSelection::CreateTexPath(const char *pszPrimaryPath, const char *pszSecondaryPath, char *pszBuffer, int iLength)
{
	Q_strncpy(pszBuffer, pszPrimaryPath, iLength);
	Q_strncat(pszBuffer, pszSecondaryPath, iLength, COPY_ALL_CHARACTERS);
}

//=========================================================
//=========================================================
void CHudWeaponSelection::UserCmd_Slot1(void)
{
	SelectType(WEAPONSLOT_PRIMARY);
}

void CHudWeaponSelection::UserCmd_Slot2(void)
{
	SelectType(WEAPONSLOT_SECONDARY);
}

void CHudWeaponSelection::UserCmd_Slot3(void)
{
	SelectType(WEAPONSLOT_EQUIPMENT_ONE);
}

void CHudWeaponSelection::UserCmd_Slot4(void)
{
	SelectType(WEAPONSLOT_EQUIPMENT_TWO);
}

void CHudWeaponSelection::UserCmd_Slot5(void)
{
	SelectType(WEAPONSLOT_MELEE);
}

void CHudWeaponSelection::UserCmd_Slot6(void)
{
	SelectType(WEAPONSLOT_NONE_ONE);
}

void CHudWeaponSelection::UserCmd_Slot7(void)
{
	SelectType(WEAPONSLOT_NONE_TWO);
}

void CHudWeaponSelection::UserCmd_Close(void)
{
	CancelWeaponSelection();
}

void CHudWeaponSelection::UserCmd_NextWeapon(void)
{
	CycleToNextWeapon();
}

void CHudWeaponSelection::UserCmd_PrevWeapon(void)
{
	CycleToPrevWeapon();
}

void CHudWeaponSelection::UserCmd_LastWeapon(void)
{
	if(!CHudElement::ShouldDraw())
		return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();

	if(pWeapon && !pWeapon->IsAllowedToSwitch())
		return;

	SwitchToLastWeapon();
}

