//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapon selection handling
//
// $NoKeywords: $
//=============================================================================//
#ifndef HUD_WEAPONSELECTION_H
#define HUD_WEAPONSELECTION_H

#include "hudelement.h"
#include <vgui_controls/panel.h>

//=========================================================
//=========================================================
class C_BaseCombatWeapon;

#define WPNSELECT_COUNT 4

enum SelectFire_t
{
	SELECTFIRE_INVALID = -1,
	SELECTFIRE_RANGED = 0,
	SELECTFIRE_SINGLE,
	SELECTFIRE_BURST,
	SELECTFIRE_FULL,
	SELECTFIRE_COUNT
};

struct SelectedWeapon_t {
	bool m_bDrawX;
	int m_iSelectedFire;
	int m_iAmmo;
	bool m_bHasAmmo;
};

//=========================================================
//=========================================================
class CHudWeaponSelection : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudWeaponSelection, vgui::Panel);

public:
	CHudWeaponSelection(const char *pElementName);
	void Init(void);
	void VidInit(void);
	void LevelInit(void);
	void Reset(void);
	void ProcessInput(void);
	bool ShouldDraw(void);

	void UserCmd_Slot1(void);
	void UserCmd_Slot2(void);
	void UserCmd_Slot3(void);
	void UserCmd_Slot4(void);
	void UserCmd_Slot5(void);
	void UserCmd_Slot6(void);
	void UserCmd_Slot7(void);
	void UserCmd_Close(void);
	void UserCmd_NextWeapon(void);
	void UserCmd_PrevWeapon(void);
	void UserCmd_LastWeapon(void);
	void UserCmd_DropPrimary(void);

	void MsgFunc_FireMode(bf_read &msg);

private:
	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void OnThink(void);
	void Paint(void);

	void SetSelection(bool bState);
	bool IsSelecting(void) const { return m_bSelectionVisible; }
	bool SetSelectedWeapon(C_BaseCombatWeapon *pWeapon, bool bStartSelect = false);
	C_BaseCombatWeapon *GetSelectedWeapon(void) const;
	int GetSelectedWeaponID(void) const;
	void CancelWeaponSelection(void);

	void SelectWeapon(void);
	void SelectType(int iWeaponSlot);
	void CycleToNextWeapon(void);
	void CycleToPrevWeapon(void);
	void CycleThroughWeapons(bool bForward);

	void SwitchToLastWeapon(void);

	bool UpdateWeapons(void);
	bool UpdateWeaponHandles(void);
	void UpdateWeaponNames(void);
	void UpdateSelectedWeapon(void);

	void CreateTexPath(const char *pszPrimaryPath, const char *pszSecondaryPath, char *pszBuffer, int iLength);

public:
	typedef CHandle<C_BaseCombatWeapon> C_BaseCombatWeaponHandle;

private:
	bool m_bSelectionVisible;
	float m_flShowTime;

	float m_flSoonestNextInput;

	int m_iBGTexID;
	int m_iBGWide, m_iBGTall, m_iBGYOffset;
	int m_iBarXPos, m_iBarWide, m_iBarXGap, m_iBarYGap;
	int m_iXTexID;
	int m_iSelectFireTexID[SELECTFIRE_COUNT];
	vgui::HFont m_AmmoFont, m_BarFont;
	int m_iAmmoNumXPos, m_iAmmoNumYPos;

	SelectedWeapon_t m_SelectedWeapon;

	wchar_t m_wszWeaponNames[WPNSELECT_COUNT][32];
	int m_iWeaponNamesLength[WPNSELECT_COUNT];
	int m_iWeaponNamesCount;

	C_BaseCombatWeaponHandle m_hWeapons[WPNSELECT_COUNT];
};

#endif // HUD_WEAPONSELECTION_H