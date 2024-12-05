//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_SLOTSELECT_H
#define VGUI_SLOTSELECT_H
#ifdef _WIN32
#pragma once
#endif

#include "imc_format.h"

#include <vgui_controls/editablepanel.h>
#include <vgui_controls/propertypanel.h>

#include <igameevents.h>

namespace vgui
{
	class INSFrame;
	class Button;
	class RichText;
	class HTML;
	class ImagePanel;
	class ImagePanelGroup;
	class CChoiceButton;
	class CTeamChoiceButton;
	class CPlayTeamChoiceButton;
	class PropertySheet;
	class EmbeddedImage;
}

class CSlotChoiceHolder;
class CSquadsHolder;
class CSquadMenu;
class CHTMLWindow;

#include "imc_format.h"

//=========================================================
//=========================================================
class CSlotSelect : public vgui::EditablePanel, public IGameEventListener2
{
	DECLARE_CLASS_SIMPLE(CSlotSelect, vgui::EditablePanel);

public:
	CSlotSelect(Panel *pParent);
	virtual ~CSlotSelect();

	void Reset(void);
	void ResetMenus(void);

	void Update(void);

	virtual void FireGameEvent(IGameEvent *pEvent);

	MESSAGE_FUNC(OnResetData, "ResetData");
	//MESSAGE_FUNC_PARAMS(OnFinishSlot, "FinishSlot", pData);

protected:
	virtual void OnVisibilityChange(int iOldVisible);

private:
	void UpdateTeam(int iNewTeam);

private:
	vgui::PropertySheet *m_pSquads;
	CSquadsHolder *m_pSquadsHolder;
	CSquadMenu *m_pSquadMenus[MAX_SQUADS];

	bool m_bNeedsUpdate;

	bool m_bSetupOnce;
};

//=========================================================
//=========================================================
class CSquadMenu;
class CSlotButton;

class CSlotHolder : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CSlotHolder, vgui::EditablePanel);

public:
	CSlotHolder(Panel *pParent, CSquadMenu *pSquadMenu, int iFireteamID);

	void Reset(void);

	void Update(void);
	void UpdateTeam(int iNewTeam);

	void SetButtonSize(int iWide, int iTall);

	const C_INSSquad *GetSquad(void) const;

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void ApplySettings(KeyValues *inResourceData);
	virtual void PerformLayout(void);
	MESSAGE_FUNC_INT(OnSlotSelect, "SlotSelect", iSlot);

private:
	void LayoutSlots(void);

	int GetSquadID(void) const;

private:
	CSquadMenu *m_pSquadMenu;

	int m_iFireteamID;

	CSlotButton *m_pSlotButtons[MAX_FIRETEAM_SLOTS];

	int m_iSBYPos, m_iSBGap;
};

//=========================================================
//=========================================================
class CFireTeamTabHolder;

class CFireTeamHolder : public vgui::PropertyPanel
{
	DECLARE_CLASS_SIMPLE(CFireTeamHolder, vgui::PropertyPanel);

public:
	CFireTeamHolder(CSquadMenu *pParent, CFireTeamTabHolder *pTabHolder);

	void Reset(void);
	void Update(void);
	void UpdateTeam(int iNewTeam);
	void UpdateSquad(void);

private:
	CSquadMenu *m_pSquadMenu;

	CSlotHolder *m_pSoltHolders[MAX_FIRETEAMS];
};

//=========================================================
//=========================================================
class CSlotInformation : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CSlotInformation, vgui::EditablePanel);

public:
	CSlotInformation(CSquadMenu *pParent);

	void Reset(void);
	void Update(void);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	CSquadMenu *m_pSquadMenu;

	vgui::ImagePanel *m_pTitle;
	vgui::RichText *m_pText;
	//CHTMLWindow *m_pPlayerInfo;
};

#endif // VGUI_SLOTSELECT_H
