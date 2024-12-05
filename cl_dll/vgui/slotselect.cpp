//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "filesystem.h"
#include <vgui/ischeme.h>
#include <vgui/isurface.h>
#include <vgui/iinput.h>
#include <vgui/ilocalize.h>
#include <vgui/ivgui.h>

#include <keyvalues.h>

#include <vgui_controls/propertysheet.h>
#include <vgui_controls/imagepanel.h>
#include <vgui_controls/embeddedimage.h>
#include <vgui_controls/scrollbar.h>
#include <vgui_controls/richtext.h>
#include <cl_dll/iviewport.h>

#include <vgui_controls/textimage.h>

#include "ins_player_shared.h"
#include "squadsetup.h"
#include "changesquad.h"
#include "ins_headers.h"
#include "play_team_shared.h"
#include "ins_squad_shared.h"
#include "team_lookup.h"
#include "slotselect.h"
#include "htmlwindow.h"
#include <igameresources.h>

#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define INVALID_PLAYERID -1

enum ClassImageType_t
{
	CLASSIMAGE_ICON = 0,
	CLASSIMAGE_TITLE,
	CLASSIMAGE_PORTRAIT
};

void GetClassImagePath(CTeamLookup *pTeamLookup, CPlayerClass *pPlayerClass, int iType,
					   char *pszBuffer, int iLength)
{
 	Q_snprintf(pszBuffer, iLength, "teams/%s/class/%s",
		pTeamLookup->GetFileName(),
		pPlayerClass->GetFileName());

	const char *pszExtension = NULL;

	switch(iType)
	{
	case CLASSIMAGE_ICON:
		break;
	case CLASSIMAGE_TITLE:
		pszExtension = "_title";
		break;
	case CLASSIMAGE_PORTRAIT:
		pszExtension = "_port";
		break;
	}

	if(pszExtension)
		Q_strncat(pszBuffer, pszExtension, iLength, COPY_ALL_CHARACTERS);
}

//=========================================================
//=========================================================
class CSquadsHolder : public Panel
{
	DECLARE_CLASS_SIMPLE(CSquadsHolder, Panel);

public:
	CSquadsHolder(Panel *pParent, const char *pszName = "SquadsHolder")
		: Panel(pParent, pszName)
	{
	}

protected:
	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);
		SetBgColor(GetSchemeColor("SquadHolder.BgColor", GetBgColor(), pScheme));
	}
};

class CFireTeamTabHolder : public CSquadsHolder
{
	DECLARE_CLASS_SIMPLE(CFireTeamTabHolder, CSquadsHolder);

public:
	CFireTeamTabHolder(Panel *pParent)
		: CSquadsHolder(pParent, "FireTeamTabHolder")
	{
	}
};

//=========================================================
//=========================================================
class CSquadMenu : public EditablePanel
{
	DECLARE_CLASS_SIMPLE(CSquadMenu, Panel);

public:
	CSquadMenu(CSlotSelect *pParent, int iSquadID);

	void Reset(void);

	void Update(bool bInformation = false);
	void UpdateTeam(int iNewTeam);
	void UpdateSquad(void);

	void SetSlotSelected(int iSlotID);
	int GetSlotSelected(void) const { return m_iSelectedSlot; }

	int GetSquadID(void) const { return m_iSquadID; }
	const C_INSSquad *GetSquad(void) const;

	CSlotSelect *GetSlotSelect(void) { return m_pSlotSelect; }

protected:
	MESSAGE_FUNC(OnResetData, "ResetData");
	
private:
	CSlotSelect *m_pSlotSelect;

	CFireTeamHolder *m_pFireTeamHolder;
	CSlotInformation *m_pSlotInformation;

	int m_iSelectedSlot;

	int m_iSquadID;
	const C_INSSquad *m_pSquad;
};

//=========================================================
//=========================================================
CSquadMenu::CSquadMenu(CSlotSelect *pParent, int iSquadID)
	: EditablePanel(pParent, "SquadMenu")
{
	m_pSlotSelect = pParent;

	m_iSquadID = iSquadID;
	m_pSquad = NULL;

	//m_pSlotHolder = new CSlotHolder(this);

	m_pFireTeamHolder = new CFireTeamHolder(this, new CFireTeamTabHolder(this));
	m_pSlotInformation = new CSlotInformation(this);

	//m_pSlotHolder->AddActionSignalTarget(this);
	m_pFireTeamHolder->AddActionSignalTarget(this);

	LoadControlSettings("Resource/UI/Frames/Panels/Controls/SquadMenu.res");
}

//=========================================================
//=========================================================
void CSquadMenu::OnResetData(void)
{
	Reset();
}

//=========================================================
//=========================================================
void CSquadMenu::Reset(void)
{
	//m_pSlotHolder->Reset();
	m_pFireTeamHolder->Reset();
	m_pSlotInformation->Reset();
}

//=========================================================
//=========================================================
void CSquadMenu::Update(bool bInformation)
{
	//m_pSlotHolder->Update();
	m_pFireTeamHolder->Update();

	if(bInformation)
		m_pSlotInformation->Update();
}

//=========================================================
//=========================================================
void CSquadMenu::UpdateTeam(int iNewTeam)
{
	// update squad pointer
	m_pSquad = GetGlobalPlayTeam(iNewTeam)->GetSquad(m_iSquadID);

	// update slots
	m_pFireTeamHolder->UpdateTeam(iNewTeam);

	// update info
	m_pSlotInformation->Reset();
}

//=========================================================
//=========================================================
void CSquadMenu::UpdateSquad(void)
{
	// make sure its on the right page
	m_pFireTeamHolder->UpdateSquad();
}

//=========================================================
//=========================================================
void CSquadMenu::SetSlotSelected(int iSlotID)
{
	m_iSelectedSlot = iSlotID;
	m_pSlotInformation->Update();
}

//=========================================================
//=========================================================
const C_INSSquad *CSquadMenu::GetSquad(void) const
{
	Assert(m_pSquad);
	return m_pSquad;
}

//=========================================================
//=========================================================
class CSlotButton : public Button
{
	DECLARE_CLASS_SIMPLE(CSlotButton, Button);

	#define SLOTBUTTON_CLASSIMAGE_WIDE 40
	#define SLOTBUTTON_CLASSIMAGE_TALL 40
	#define SLOTBUTTON_CLASSIMAGE_XPOS 5
	#define SLOTBUTTON_CLASSIMAGE_YPOS 5

	#define SLOTBUTTON_NAMETEXT_XPOS 52
	#define SLOTBUTTON_NAMETEXT_YPOS 12
	#define SLOT_BUTTON_NAMETEXT_SELECT "SLOT AVAILABLE"
	#define SLOT_BUTTON_NAMETEXT_NOT "NOT AVAILABLE"

	#define SLOTBUTTON_CLASSTEXT_XPOS 52
	#define SLOTBUTTON_CLASSTEXT_YPOS 28

private:
	class ButtonText : public TextImage
	{
		DECLARE_CLASS_SIMPLE(ButtonText, TextImage);

	public:
		ButtonText()
			: TextImage("") { }
	};

	class ButtonTextName : public ButtonText
	{
	public:
		Color m_PlayerColor;
		Color m_SelectColor;

		enum
		{
			COLOR_PLAYER = 0,
			COLOR_SELECT
		};

	public:
		ButtonTextName()
			: ButtonText() { }

		void SetPlayerColor(Color &PlayerColor)
		{
			m_PlayerColor = PlayerColor;
		}

		void SetSelectColor(Color &SelectColor)
		{ 
			m_SelectColor = SelectColor;
		}

		void UseColor(int iUseColor)
		{
			switch(iUseColor)
			{
			case COLOR_PLAYER:
				SetColor(m_PlayerColor);
				break;
			case COLOR_SELECT:
				SetColor(m_SelectColor);
				break;
			}
		}
	};

private:
	// data
	CSlotHolder *m_pSlotHolder;
	int m_iSlotID;

	bool m_bSelected, m_bValid;

	Color m_BgColor, m_ArmedColor;

	// layout
	int m_iCurrentPlayerID;
	bool m_bCurrentEnabled;

	EmbeddedImage *m_pClassImage;
	ImagePanel *m_pRankImage;

	ButtonTextName *m_pName;
	ButtonText *m_pClass;

	IBorder *m_pActiveBorder;
	IBorder *m_pDimBorder;

public:
	//=========================================================
	//=========================================================
	CSlotButton(CSlotHolder *pParent, int iSlotID)
		: Button(pParent, "SlotButton", "", pParent, "SlotSelect")
	{
		// int vars
		m_pSlotHolder = pParent;

		m_iSlotID = iSlotID;

		m_bValid = false;
		m_bSelected = false;

		m_iCurrentPlayerID = INVALID_PLAYERID;
		m_bCurrentEnabled = false;
		
		// setup panels
		m_pClassImage = new EmbeddedImage(this, NULL);

		m_pName = new ButtonTextName;
		m_pClass = new ButtonText;

		m_pActiveBorder = m_pDimBorder = NULL;
	}

	//=========================================================
	//=========================================================
	void SetValid(bool bState)
	{
		m_bValid = bState;

		InvalidateLayout();
		Repaint();
	}

	//=========================================================
	//=========================================================
	void SetActive(bool bState)
	{
		m_bSelected = bState;

		InvalidateLayout();
		Repaint();
	}

	//=========================================================
	//=========================================================
	bool Update(void)
	{
		if(!m_bValid)
			return false;

		// HACK [
		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();

		if(!pPlayer)
			return false;

		const C_INSSquad *pSquad = m_pSlotHolder->GetSquad();

		if(!pSquad)
			return false;
		// HACK ]

		int iPlayerID;

		if(!pSquad->IsSlotEmpty(m_iSlotID))
			iPlayerID = pSquad->GetPlayerID(m_iSlotID);
		else
			iPlayerID = INVALID_PLAYERID;

		const C_SquadConventional *pConventional = dynamic_cast<const C_SquadConventional*>(pSquad);

		bool bEnabled;

		if(pConventional)
		{
			SlotData_t SlotData;
			C_SquadConventional::TranslateSlot(m_iSlotID, SlotData);

			bEnabled = pConventional->IsFireteamEnabled(SlotData.m_iFireteamID);
		}
		else
		{
			bEnabled = pSquad->IsEnabled();
		}

		if(iPlayerID == m_iCurrentPlayerID && m_bCurrentEnabled == bEnabled)
			return false;

		m_iCurrentPlayerID = iPlayerID;
		m_bCurrentEnabled = bEnabled;

		Setup();

		return true;
	}

	//=========================================================
	//=========================================================
	void Setup(void)
	{
		// get player
		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();
		C_PlayTeam *pTeam = GetGlobalPlayTeam(pPlayer->GetTeamID());

		Setup(pTeam);

		// setup enabled
		SetEnabled(m_bCurrentEnabled);
	}

	void Setup(C_PlayTeam *pTeam)
	{
		CTeamLookup *pTeamLookup = pTeam->GetTeamLookup();
		CPlayerClass *pPlayerClass = m_pSlotHolder->GetSquad()->GetClass(m_iSlotID);

		if(!pPlayerClass)
		{
			SetEnabled(false);
			return;
		}

		// setup class image
		char szClassImagePath[256];
		GetClassImagePath(pTeamLookup, pPlayerClass, CLASSIMAGE_ICON, szClassImagePath, sizeof(szClassImagePath));
		m_pClassImage->SetImage(szClassImagePath);

		// setup name
		char szName[256];
		int iUseColor;

		if(!m_bCurrentEnabled)
		{
			Q_strcpy(szName, SLOT_BUTTON_NAMETEXT_NOT);
			iUseColor = ButtonTextName::COLOR_SELECT;
		}
		else if(m_iCurrentPlayerID == INVALID_PLAYERID)
		{
			Q_strcpy(szName, SLOT_BUTTON_NAMETEXT_SELECT);
			iUseColor = ButtonTextName::COLOR_SELECT;
		}
		else
		{
			IGameResources *gr = GameResources();
			UTIL_MakeSafeName(gr->GetPlayerName(m_iCurrentPlayerID), szName);
			iUseColor = ButtonTextName::COLOR_PLAYER;			
		}

		char szNewName[256];
		Q_snprintf(szNewName, sizeof(szNewName), "> %s", szName);

		m_pName->SetText(szNewName);
		m_pName->UseColor(iUseColor);

		// setup class name
		m_pClass->SetText(pPlayerClass->GetName());

		m_pName->ResizeImageToContent();
		m_pClass->ResizeImageToContent();
	}

protected:
	//=========================================================
	//=========================================================
	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		Button::ApplySchemeSettings(pScheme);

		// set bgcolor
		m_BgColor = GetSchemeColor("SquadChoiceButton.BgColor", pScheme);

		// set fgcolor
		m_pActiveBorder = pScheme->GetBorder("TeamSelectBorder");
		m_pDimBorder = pScheme->GetBorder("SquadChoiceDimBorder");

		// set armed colour
		m_ArmedColor = GetSchemeColor("TeamChoiceButton.BgColor", GetBgColor(), pScheme);

		// setup images
		m_pClassImage->SetBounds(scheme()->GetProportionalScaledValue(SLOTBUTTON_CLASSIMAGE_XPOS),
			scheme()->GetProportionalScaledValue(SLOTBUTTON_CLASSIMAGE_YPOS),
			scheme()->GetProportionalScaledValue(SLOTBUTTON_CLASSIMAGE_WIDE),
			scheme()->GetProportionalScaledValue(SLOTBUTTON_CLASSIMAGE_TALL));

		// setup text
		m_pName->SetPos(scheme()->GetProportionalScaledValue(SLOTBUTTON_NAMETEXT_XPOS),
			scheme()->GetProportionalScaledValue(SLOTBUTTON_NAMETEXT_YPOS));

		m_pClass->SetPos(scheme()->GetProportionalScaledValue(SLOTBUTTON_CLASSTEXT_XPOS),
			scheme()->GetProportionalScaledValue(SLOTBUTTON_CLASSTEXT_YPOS));

		m_pName->SetFont(pScheme->GetFont("DefaultBold"));
		m_pClass->SetFont(pScheme->GetFont("Default"));
		m_pName->ResizeImageToContent();
		m_pClass->ResizeImageToContent();

		m_pName->SetPlayerColor(GetSchemeColor("SquadChoiceButton.PlayerColor", GetFgColor(), pScheme));
		m_pName->SetSelectColor(GetSchemeColor("SquadChoiceButton.SelectSlot", GetFgColor(), pScheme));
		m_pClass->SetColor(GetSchemeColor("SquadChoiceButton.ClassColor", GetFgColor(), pScheme));

		// reun setup (when valid)
		if(m_bValid)
			Setup();
	}

	//=========================================================
	//=========================================================
	virtual void ApplySettings(KeyValues *inResourceData)
	{
		BaseClass::ApplySettings(inResourceData);

		m_pSlotHolder->SetButtonSize(GetWide(), GetTall());
	}

	//=========================================================
	//=========================================================
	virtual IBorder *GetBorder(bool depressed, bool armed, bool selected, bool keyfocus)
	{
		if(m_bSelected)
			return m_pActiveBorder;

		return m_pDimBorder;
	}

	//=========================================================
	//=========================================================
	virtual Color GetButtonBgColor(void)
	{
		if(m_bValid && IsArmed())
			return m_ArmedColor;

		return m_BgColor;
	}

	//=========================================================
	//=========================================================
	virtual void Paint(void)
	{
		BaseClass::Paint();

		if(!m_bValid)
			return;

		m_pName->Paint();
		m_pClass->Paint();
	}

	//=========================================================
	//=========================================================
    virtual bool CanBeDefaultButton(void)
    {
        return false;
    }

	//=========================================================
	//=========================================================
	virtual void OnMousePressed(MouseCode code) 
	{
		if(code != MOUSE_LEFT)
			return;

		if(!IsEnabled() || !m_bValid)
			return;

		if(!IsMouseClickEnabled(code))
			return;
			
		if(IsUseCaptureMouseEnabled())
		{
			RequestFocus();

			PostActionSignal(new KeyValues("SlotSelect", "iSlot", m_iSlotID));

			SetSelected(true);
			Repaint();
		}
			
		vgui::input()->SetMouseCapture(GetVPanel());
	}

	//=========================================================
	//=========================================================
	virtual void OnMouseReleased(MouseCode code)
	{
		if(IsUseCaptureMouseEnabled())
			input()->SetMouseCapture(NULL);

		SetSelected(false);
		Repaint();
	}
};

//=========================================================
//=========================================================
CFireTeamHolder::CFireTeamHolder(CSquadMenu *pParent, CFireTeamTabHolder *pTabHolder)
	: PropertyPanel(pParent, "FireTeamHolder", TABTYPE_SCROLL)
{
	m_pSquadMenu = pParent;

	GetPropertySheet()->CreateTabParent(pTabHolder);

	for(int i = 0; i < MAX_FIRETEAMS; i++)
	{
		m_pSoltHolders[i] = new CSlotHolder(this, m_pSquadMenu, i);
		m_pSoltHolders[i]->AddActionSignalTarget(this);
	}

	AddPage(m_pSoltHolders[FIRETEAM_ONE], "FireTeam One");
	AddPage(m_pSoltHolders[FIRETEAM_TWO], "FireTeam Two");

	LoadControlSettings("Resource/UI/Frames/Panels/Controls/FireTeamHolder.res");
}

//=========================================================
//=========================================================
void CFireTeamHolder::Reset(void)
{
	for(int i = 0; i < MAX_FIRETEAMS; i++)
		m_pSoltHolders[i]->Reset();
}

//=========================================================
//=========================================================
void CFireTeamHolder::Update(void)
{
	for(int i = 0; i < MAX_FIRETEAMS; i++)
		m_pSoltHolders[i]->Update();
}

//=========================================================
//=========================================================
void CFireTeamHolder::UpdateTeam(int iNewTeam)
{
	for(int i = 0; i < MAX_FIRETEAMS; i++)
		m_pSoltHolders[i]->UpdateTeam(iNewTeam);
}

//=========================================================
//=========================================================
void CFireTeamHolder::UpdateSquad(void)
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	SlotData_t SlotData;
	CSquadConventional::TranslateSlot(pPlayer->GetSlotID(), SlotData);

	Panel *pSlotPanel = GetPropertySheet()->GetPage(SlotData.m_iFireteamID);

	if(!pSlotPanel)
		return;

	GetPropertySheet()->SetActivePage(pSlotPanel);
}

//=========================================================
//=========================================================
CSlotHolder::CSlotHolder(Panel *pParent, CSquadMenu *pSquadMenu, int iFireteamID)
	: EditablePanel(pParent, "SlotHolder")
{
	// init
	m_pSquadMenu = pSquadMenu;

	m_iFireteamID = iFireteamID;

	// setup panels
	for(int i = 0; i < MAX_FIRETEAM_SLOTS; i++)
	{
		int iSlotID;
		SlotData_t SlotData(iFireteamID, i);
		C_SquadConventional::TranslateSlot(SlotData, iSlotID);

		m_pSlotButtons[i] = new CSlotButton(this, iSlotID);
		m_pSlotButtons[i]->AddActionSignalTarget(this);
	}

	// load resources
	LoadControlSettings("Resource/UI/Frames/Panels/Controls/SlotHolder.res");
}

//=========================================================
//=========================================================
void CSlotHolder::Reset(void)
{
	for(int i = 0; i < MAX_FIRETEAM_SLOTS; i++)
	{
		m_pSlotButtons[i]->SetActive(false);
	}
}

//=========================================================
//=========================================================
void CSlotHolder::Update(void)
{
	for(int i = 0; i < MAX_FIRETEAM_SLOTS; i++)
	{
		bool bUpdated = m_pSlotButtons[i]->Update();

		SlotData_t SlotData;
		
		if(!CSquadConventional::TranslateSlot(m_pSquadMenu->GetSlotSelected(), SlotData))
			continue;

		if(bUpdated && i == SlotData.m_iFireteamID)
			m_pSquadMenu->Update(true);
	}
}

//=========================================================
//=========================================================
void CSlotHolder::UpdateTeam(int iNewTeam)
{
	SlotData_t SlotData(m_iFireteamID);

	for(int i = 0; i < MAX_FIRETEAM_SLOTS; i++)
	{
		CSlotButton *pSlotButton = m_pSlotButtons[i];
		SlotData.m_iSlotID = i;

		pSlotButton->SetValid(true);
		pSlotButton->Setup(GetGlobalPlayTeam(iNewTeam));
	}
}

//=========================================================
//=========================================================
void CSlotHolder::SetButtonSize(int iWide, int iTall)
{
	for(int i = 0; i < MAX_FIRETEAM_SLOTS; i++)
	{
		m_pSlotButtons[i]->SetSize(iWide, iTall);
	}
}

//=========================================================
//=========================================================
void CSlotHolder::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// set border
	SetBorder(pScheme->GetBorder("SquadChoiceDimBorder"));
}

//=========================================================
//=========================================================
void CSlotHolder::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	m_iSBYPos = inResourceData->GetInt("sb_ypos");
	m_iSBGap = inResourceData->GetInt("sb_gap");
}

//=========================================================
//=========================================================
void CSlotHolder::PerformLayout(void)
{
	BaseClass::PerformLayout();
	LayoutSlots();
}

//=========================================================
//=========================================================
void CSlotHolder::LayoutSlots(void)
{
	int iSBXPos, iSBTall, iCurrentYPos;

	iSBXPos = m_pSlotButtons[0]->GetXPos();
	iSBTall = m_pSlotButtons[0]->GetTall();
	iCurrentYPos = m_iSBYPos;

	for(int i = 0; i < MAX_FIRETEAM_SLOTS; i++)
	{
		m_pSlotButtons[i]->SetPos(iSBXPos, iCurrentYPos);
		iCurrentYPos += (iSBTall + m_iSBGap);
	}
}

//=========================================================
//=========================================================
int CSlotHolder::GetSquadID(void) const
{
	return m_pSquadMenu->GetSquadID();
}

//=========================================================
//=========================================================
const C_INSSquad *CSlotHolder::GetSquad(void) const
{
	return m_pSquadMenu->GetSquad();
}

//=========================================================
//=========================================================
void CSlotHolder::OnSlotSelect(int iSlotID)
{
	// just in case ;)
	if(!g_pChangeSquad || !m_pSquadMenu || !m_pSquadMenu->GetSlotSelect() || !GetSquad())
		return;

	SquadInfo_t SquadInfo(m_pSquadMenu->GetSquadID(), iSlotID);

	// setup buttons
	m_pSquadMenu->GetSlotSelect()->ResetMenus();

	SlotData_t SlotData;
	CSquadConventional::TranslateSlot(iSlotID, SlotData);

	m_pSlotButtons[SlotData.m_iSlotID]->SetActive(true);

	if(GetSquad()->IsSlotEmpty(iSlotID))
	{
		// we can progress on now
		g_pChangeSquad->ProgressOn();

		// send new signal
		g_pChangeSquad->SetSquadSelect(SquadInfo.GetEncodedInfo());
	}
	else
	{
		// freeze
		g_pChangeSquad->FreezeProgress();
	}

	// update slots
	m_pSquadMenu->SetSlotSelected(iSlotID);
}

//=========================================================
//=========================================================
CSlotInformation::CSlotInformation(CSquadMenu *pParent)
	: EditablePanel(pParent, "SlotInformation")
{
	// init
	m_pSquadMenu = pParent;

	m_pTitle = new ImagePanel(this, "Title");
	m_pText = new RichText(this, "Text");
	m_pText->SetAllowMouse(false);

	//m_pPlayerInfo = new CHTMLWindow(this, "PlayerInfo");
	//m_pPlayerInfo->SetSizeParent(false);

	// setup
	SetPaintBackgroundEnabled(false);

	// load resources
	LoadControlSettings("Resource/UI/Frames/Panels/Controls/SlotInformation.res");
}

//=========================================================
//=========================================================
void CSlotInformation::Reset(void)
{
	m_pTitle->SetVisible(false);
	m_pText->SetVisible(false);
	//m_pPlayerInfo->SetVisible(false);
}

//=========================================================
//=========================================================
void CSlotInformation::Update(void)
{
	char szClassImagePath[256];

	int iSlotID = m_pSquadMenu->GetSlotSelected();

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();
	C_PlayTeam *pTeam = GetGlobalPlayTeam(pPlayer->GetTeamID());
	const C_INSSquad *pSquad = m_pSquadMenu->GetSquad();
	CTeamLookup *pTeamLookup = pTeam->GetTeamLookup();
	CPlayerClass *pPlayerClass = pSquad->GetClass(iSlotID);

	// set title image
	GetClassImagePath(pTeamLookup, pPlayerClass, CLASSIMAGE_TITLE, szClassImagePath, sizeof(szClassImagePath));
	m_pTitle->SetImage(szClassImagePath);

	// set text
	m_pText->ClearText();
	m_pText->SetText(pPlayerClass->GetDescription());

	GetClassImagePath(pTeamLookup, pPlayerClass, CLASSIMAGE_PORTRAIT, szClassImagePath, sizeof(szClassImagePath));
	m_pText->SetBackgroundImage(szClassImagePath);

	// make it visible
	m_pTitle->SetVisible(true);
	m_pText->SetVisible(true);
	//m_pPlayerInfo->SetVisible(true);
}

//=========================================================
//=========================================================
void CSlotInformation::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pText->SetBackgroundImageMask(pScheme->GetColor("SquadInformation.SlotMask", GetBgColor()));
}

//=========================================================
//=========================================================
CSlotSelect::CSlotSelect(Panel *pParent)
	 : EditablePanel(pParent, NULL)
{
	m_bSetupOnce = false;

	// setup property progress
	m_pSquads = new vgui::PropertySheet(this, "Squads", TABTYPE_SCROLL);
	m_pSquadsHolder = new CSquadsHolder(this);
	m_pSquads->CreateTabParent(m_pSquadsHolder);
	m_pSquadsHolder->AddActionSignalTarget(this);

	for(int i = 0; i < MAX_SQUADS; i++)
	{
		m_pSquadMenus[i] = new CSquadMenu(this, i);
		m_pSquads->AddPage(m_pSquadMenus[i], "");
	}

	// listen to a player changing team
	gameeventmanager->AddListener(this, "player_team", false);
	gameeventmanager->AddListener(this, "player_squad", false);
	gameeventmanager->AddListener(this, "game_squadupdate", false);

	// load resources
	LoadControlSettings("Resource/UI/Frames/Panels/SlotSelect.res");
}

//=========================================================
//=========================================================
CSlotSelect::~CSlotSelect()
{
	gameeventmanager->RemoveListener(this);
}

//=========================================================
//=========================================================
void CSlotSelect::Reset(void)
{
	m_pSquads->ResetAllData();
}

//=========================================================
//=========================================================
void CSlotSelect::ResetMenus(void)
{
	for(int i = 0; i < MAX_SQUADS; i++)
	{
		m_pSquadMenus[i]->Reset();
	}
}

//=========================================================
//=========================================================
void CSlotSelect::Update(void)
{
	if(!g_pChangeSquad || !g_pChangeSquad->IsVisible() || !m_bSetupOnce)
		return;

	for(int i = 0; i < MAX_SQUADS; i++)
	{
		m_pSquadMenus[i]->Update();
	}
}

//=========================================================
//=========================================================
void CSlotSelect::FireGameEvent(IGameEvent *pEvent)
{
	const char *pszType = pEvent->GetName();

	if(Q_strcmp(pszType, "player_team") == 0)
	{
		int iPlayerID = engine->GetPlayerForUserID(pEvent->GetInt("userid"));

		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

		if(!pPlayer || pPlayer->entindex() != iPlayerID)
			return;

		UpdateTeam(pEvent->GetInt("team"));
	}
	else/* if(Q_strcmp(pszType, "player_squad") == 0 || Q_strcmp(pszType, "squad_update") == 0)*/
	{
		Update();
	}
}

//=========================================================
//=========================================================
void CSlotSelect::OnVisibilityChange(int iOldVisible)
{
	BaseClass::OnVisibilityChange(iOldVisible);

	if(!IsVisible())
	{
		Reset();
		return;
	}

	Update();

	// update the current sheet
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	if(!pPlayer->IsValidSquad())
		return;

	int iSquadID = pPlayer->GetSquadID();

	Panel *pSquadPage = m_pSquads->GetPage(pPlayer->GetSquadID());

	if(!pSquadPage)
		return;

	m_pSquads->SetActivePage(pSquadPage);

	// now tell the fireteams to sort themselves out!
	m_pSquadMenus[iSquadID]->UpdateSquad();
}

//=========================================================
//=========================================================
void CSlotSelect::OnResetData(void)
{
	m_pSquads->ResetAllData();
}

//=========================================================
//=========================================================
void CSlotSelect::UpdateTeam(int iNewTeam)
{
	if(!IsPlayTeam(iNewTeam))
		return;

	// setup pages
	C_PlayTeam *pTeam = GetGlobalPlayTeam(iNewTeam);

	// what happens when C_PlayTeam is invalid?
	if(!pTeam)
		return;

	int iMaxSquads = pTeam->GetSquadCount();

	bool bFirstSquad = true;

	for(int i = 0; i < MAX_SQUADS; i++)
	{
		const char *pszName;
		bool bDisabled = false;

		if(i < iMaxSquads)
		{
			C_INSSquad *pSquad = pTeam->GetSquad(i);
			pszName = pSquad->GetName();
		}
		else
		{
			pszName = C_INSSquad::GetDefaultName(i);
			bDisabled = true;
		}

		m_pSquads->SetPageName(i, pszName);

		if(bDisabled)
		{
			m_pSquads->DisablePage(pszName);
		}
		else
		{
			if(bFirstSquad)
			{
				m_pSquads->SetActivePage(m_pSquads->GetPage(pszName));
				bFirstSquad = false;
			}

			m_pSquads->EnablePage(pszName);
			m_pSquadMenus[i]->UpdateTeam(iNewTeam);
		}
	}

	m_bSetupOnce = true;
}

