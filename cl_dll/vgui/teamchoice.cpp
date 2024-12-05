//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <vgui/iborder.h>
#include <vgui/iinput.h>
#include <vgui/ilocalize.h>
#include <vgui/ivgui.h>

#include <cdll_client_int.h>

#include "teamchoice.h"

#include <vgui/ischeme.h>
#include <vgui/isurface.h>
#include <keyvalues.h>
#include <filesystem.h>

#include <vgui_controls/editablepanel.h>
#include <vgui_controls/button.h>
#include <vgui_controls/imagepanel.h>
#include <vgui_controls/imagecyclergroup.h>
#include <vgui_controls/imagepanelgroup.h>
#include <vgui_controls/richtext.h>
#include <vgui_controls/insframe.h>

#include "ins_gamerules.h"
#include "c_play_team.h"
#include "changeteam.h"
#include "c_team.h"
#include "c_play_team.h"
#include "team_lookup.h"

#include <cl_dll/iviewport.h>

#include "tier0/memdbgon.h"

using namespace vgui;


//-----------------------------------------------------------------------------
// Purpose: A Button for Team Selection
//-----------------------------------------------------------------------------
#define TEAMCHOICEBUTTON_NUMBERIMAGE_IMAGEPREFIX "interface/changeteam/cb"

#define TEAMCHOICEBUTTON_NUMBERIMAGE_WIDE 36
#define TEAMCHOICEBUTTON_NUMBERIMAGE_TALL TEAMCHOICEBUTTON_NUMBERIMAGE_WIDE
#define TEAMCHOICEBUTTON_NUMBERIMAGE_XPOS 5
#define TEAMCHOICEBUTTON_NUMBERIMAGE_YPOS 5

#define TEAMCHOICEBUTTON_NAMETEXT_XPOS 48
#define TEAMCHOICEBUTTON_NAMETEXT_YPOS 15

class CChoiceButton : public Button
{
private:
	bool m_bActive;

	int m_iNumberImageID;
	ImagePanel *m_pNumberImage;

	int m_iNameTextXPos;
	int m_iNameTextYPos;
	HFont m_NameFont;
	
	Color m_NameColor;
	IBorder *m_pSelectBorder;
	IBorder *m_pNormalBorder;

protected:
	CChangeTeam *m_pChangeTeam;

	int m_iNameLength;
	wchar_t m_szName[256];

public:
	CChoiceButton(CChangeTeam *pParent, const char *pszPanelName, int iNumberImageID, const char *pszName) : Button(pParent, pszPanelName, "")
	{
		m_pChangeTeam = pParent;

		m_bActive = false;

		m_iNumberImageID = iNumberImageID;
		m_pNumberImage = new ImagePanel(this, NULL);
		m_pNumberImage->SetShouldScaleImage(true);

		m_iNameLength = 0;
		m_iNameTextXPos = m_iNameTextYPos = 0;

		m_pSelectBorder = m_pNormalBorder = NULL;

		SetupName(pszName);
		SetupNameImage();
	}

	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		Button::ApplySchemeSettings(pScheme);

		SetDefaultColor(GetSchemeColor("TeamChoiceButton.BgColor", pScheme));

		m_NameColor = GetSchemeColor("TeamChoiceButton.NameColor", GetFgColor(), pScheme);

		m_pSelectBorder = pScheme->GetBorder("TeamSelectBorder");
		m_pNormalBorder = pScheme->GetBorder("FrameBorder");

		m_NameFont = pScheme->GetFont("DefaultBig");
	}

	virtual void GetNameTextPos(int &iXPos, int &iYPos)
	{
		iXPos = TEAMCHOICEBUTTON_NAMETEXT_XPOS;
		iYPos = TEAMCHOICEBUTTON_NAMETEXT_YPOS;
	}

	void SetupNameImage(void)
	{
		char szNumberImagePath[256];
		sprintf(szNumberImagePath, "%s0%i", TEAMCHOICEBUTTON_NUMBERIMAGE_IMAGEPREFIX, m_iNumberImageID);
		m_pNumberImage->SetImage(szNumberImagePath);

		int iNumberImageWide = scheme()->GetProportionalScaledValue(TEAMCHOICEBUTTON_NUMBERIMAGE_WIDE);
		int iNumberImageTall = scheme()->GetProportionalScaledValue(TEAMCHOICEBUTTON_NUMBERIMAGE_TALL);

		int iNumberImageXPos = scheme()->GetProportionalScaledValue(TEAMCHOICEBUTTON_NUMBERIMAGE_XPOS);
		int iNumberImageYPos = scheme()->GetProportionalScaledValue(TEAMCHOICEBUTTON_NUMBERIMAGE_YPOS);

		m_pNumberImage->SetSize(iNumberImageWide, iNumberImageTall);
		m_pNumberImage->SetPos(iNumberImageXPos, iNumberImageYPos);
	}

	virtual void SetupName(const char *pszName)
	{
		int iNameTextXPos, iNameTextYPos;
		GetNameTextPos(iNameTextXPos, iNameTextYPos);

		m_iNameTextXPos = scheme()->GetProportionalScaledValue(iNameTextXPos);
		m_iNameTextYPos = scheme()->GetProportionalScaledValue(iNameTextYPos);

		if(pszName != NULL)
		{
			vgui::localize()->ConvertANSIToUnicode(pszName, m_szName, sizeof(m_szName));	
			m_iNameLength = Q_strlen(pszName);
		}
	}

	virtual void Paint()
	{
		surface()->DrawSetTextFont(m_NameFont);
		surface()->DrawSetTextPos(m_iNameTextXPos, m_iNameTextYPos);
		surface()->DrawSetTextColor(m_NameColor);

		surface()->DrawPrintText(m_szName, m_iNameLength);
	}

	virtual IBorder *GetBorder(bool depressed, bool armed, bool selected, bool keyfocus)
	{
		if(m_bActive)
			return m_pSelectBorder;

		return m_pNormalBorder;
	}

	virtual void SetActive(bool state)
	{
		m_bActive = state;
		InvalidateLayout();
		Repaint();
	}

    virtual bool CanBeDefaultButton(void)
    {
        return false;
    }

	virtual void OnMousePressed(MouseCode code) 
	{
		if(code != MOUSE_LEFT)
			return;

		if (!IsEnabled())
			return;
			
		if (!IsMouseClickEnabled(code))
			return;
			
		if (IsUseCaptureMouseEnabled())
		{
			RequestFocus();

			//PostActionSignal(new KeyValues("TeamSelect"));
			FireActionSignal();

			SetSelected(true);
			Repaint();
		}
			
		vgui::input()->SetMouseCapture(GetVPanel());
	}

	virtual void OnMouseReleased(MouseCode code)
	{
		if(IsUseCaptureMouseEnabled())
			vgui::input()->SetMouseCapture(NULL);

		SetSelected(false);
		Repaint();
	}
};

class CTeamChoiceButton : public CChoiceButton
{
protected:
	int m_iTeam;
	
public:
	CTeamChoiceButton(CChangeTeam *pParent, const char *pszPanelName, int iTeam, int iNumberImageID)
		: CChoiceButton(pParent, pszPanelName, iNumberImageID, "")
	{
		m_iTeam = iTeam;
	}

	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		CChoiceButton::ApplySchemeSettings(pScheme);

		SetupName(NULL);
	}

	virtual void SetupName(const char *pszName)
	{
		if(g_Teams.Count() == 0)
			return;

		CChoiceButton::SetupName(NULL);

		const char *pszTeamName = g_Teams[m_iTeam]->Get_Name();
		m_iNameLength = Q_strlen(pszTeamName);

		localize()->ConvertANSIToUnicode(pszTeamName, m_szName, sizeof(m_szName));
	}

};

#define TEAMCHOICEBUTTON_NAMETEXT_PT_XPOS TEAMCHOICEBUTTON_NAMETEXT_XPOS
#define TEAMCHOICEBUTTON_NAMETEXT_PT_YPOS 9

#define TEAMCHOICEBUTTON_STATUSTEXT_XPOS TEAMCHOICEBUTTON_NAMETEXT_XPOS
#define TEAMCHOICEBUTTON_STATUSTEXT_YPOS 25

class CPlayTeamChoiceButton : public CTeamChoiceButton
{
private:
	int m_iStatusLength;
	wchar_t m_szStatus[256];

	int m_iStatusTextXPos;
	int m_iStatusTextYPos;

	HFont m_StatusFont;
	Color m_StatusColor;

public:
	CPlayTeamChoiceButton(CChangeTeam *pParent, const char *pszPanelName, int iTeam, int iNumberImageID)
		: CTeamChoiceButton(pParent, pszPanelName, iTeam, iNumberImageID)
	{
		ivgui()->AddTickSignal(GetVPanel(), 1000);
	}

	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		CTeamChoiceButton::ApplySchemeSettings(pScheme);

		m_StatusColor = GetSchemeColor("TeamChoiceButton.StatusColor", GetFgColor(), pScheme);
		m_StatusFont = pScheme->GetFont("Default");

		SetupStatus();
	}

	virtual void GetNameTextPos(int &iXPos, int &iYPos)
	{
		iXPos = TEAMCHOICEBUTTON_NAMETEXT_PT_XPOS;
		iYPos = TEAMCHOICEBUTTON_NAMETEXT_PT_YPOS;
	}

	virtual void OnTick(void)
	{
		if(m_pChangeTeam->IsVisible())
			UpdateStatus();
	}

	void SetupStatus(void)
	{
		m_iStatusTextXPos = scheme()->GetProportionalScaledValue(TEAMCHOICEBUTTON_STATUSTEXT_XPOS);
		m_iStatusTextYPos = scheme()->GetProportionalScaledValue(TEAMCHOICEBUTTON_STATUSTEXT_YPOS);
	}

	void UpdateStatus(void)
	{
		if(g_Teams.Count() == 0)
			return;

		C_PlayTeam *pTeam = (C_PlayTeam*)g_Teams[m_iTeam];

		// NOTE: write the OBJ code later

		char szStatus[256];

		int iCapturedObjs, iNumObjs;
		iCapturedObjs = iNumObjs = 0;

		if(INSRules()->IsStatusRunning())
		{
			// caculate real values
		}

		sprintf(szStatus, "[%i Players] [%i/%i OBJS]", iCapturedObjs, iNumObjs, pTeam->Get_Number_Players());
		m_iStatusLength = Q_strlen(szStatus);

		localize()->ConvertANSIToUnicode(szStatus, m_szStatus, sizeof(m_szStatus));
	}

	virtual void Paint()
	{
		CChoiceButton::Paint();

		surface()->DrawSetTextFont(m_StatusFont);
		surface()->DrawSetTextPos(m_iStatusTextXPos, m_iStatusTextYPos);
		surface()->DrawSetTextColor(m_StatusColor);

		surface()->DrawPrintText(m_szStatus, m_iStatusLength);
	}
};

//=========================================================
//=========================================================
CTeamChoice::CTeamChoice(CChangeTeam *pParent) : EditablePanel(pParent, PANEL_TEAM)
{
	SetScheme("ClientScheme");
	SetPaintBackgroundEnabled(false);

	// add buttons
	m_pJoinOne = new CPlayTeamChoiceButton(this, "TeamOne", TEAM_ONE, 1);
	m_pJoinTwo = new CPlayTeamChoiceButton(this, "TeamTwo", TEAM_TWO, 2);
	m_pAutoAssign = new CChoiceButton(this, "AutoAssign", 3, "Auto Assign");
	m_pSpectator = new CTeamChoiceButton(this, "Spectator", TEAM_SPECTATOR, 4);

	m_pJoinOne->AddActionSignalTarget(this);
	m_pJoinTwo->AddActionSignalTarget(this);
	m_pAutoAssign->AddActionSignalTarget(this);
	m_pSpectator->AddActionSignalTarget(this);

	// add images
	m_pPreview = new ImageCyclerGroup(this, "TeamPreview");

	Reset();

	LoadControlSettings("Resource/UI/Frames/Panels/TeamChoice.res");
}

//=========================================================
//=========================================================
void CTeamChoice::Reset(void)
{
	m_bUsed = false;

	m_pJoinOne->SetActive(false);
	m_pJoinTwo->SetActive(false);
	m_pAutoAssign->SetActive(false);
	m_pSpectator->SetActive(false);
}

//=========================================================
//=========================================================
void CTeamChoice::PerformLayout(void)
{
	BaseClass::PerformLayout();

	m_pPreview->Reset();

	char szTeamImages[MAX_PATH];

	CreateTeamSelectPath(szTeamImages, "unknown");
	m_pPreview->AddImages(szTeamImages, GROUP_UNKNOWN);

	for(int i = TEAM_ONE; i <= TEAM_TWO; i++)
	{
		int iGroupID = (i == TEAM_ONE) ? GROUP_TEAMONE : GROUP_TEAMTWO;
	
		CTeamLookup *pTeamLookup = GetGlobalPlayTeam(i)->GetTeamLookup();
		CreateTeamSelectPath(szTeamImages, pTeamLookup->GetFileName());
		m_pPreview->AddImages(szTeamImages, iGroupID);
	}

	ShowTeam(GROUP_UNKNOWN, true);
}

//=========================================================
//=========================================================
void CTeamChoice::OnCommand(const char *command)
{
    if(stricmp(command, "TeamOne") == 0)
    {
		ShowTeam(GROUP_TEAMONE, true);
		SetTeam(TEAM_SELECT_ONE);
		return;
	}
    else if(stricmp(command, "TeamTwo") == 0)
    {
		ShowTeam(GROUP_TEAMTWO, true);
		SetTeam(TEAM_SELECT_TWO);
		return;
	}
    else if(stricmp(command, "AutoAssign") == 0)
    {
		ShowTeam(GROUP_UNKNOWN, true);
		SetTeam(TEAM_SELECT_AUTOASSIGN);
		return;
	}
	else if(stricmp(command, "Spectator") == 0)
	{
		ShowTeam(GROUP_UNKNOWN, true);
		SetTeam(TEAM_SELECT_SPECTATOR);
		return;
	}
	// mouse movement
	else if(!m_bUsed)
	{
		if(stricmp(command, "ShowTeamOne") == 0)
		{
			ShowTeam(GROUP_TEAMONE, false);
			return;
		}
	    else if(stricmp(command, "ShowTeamTwo") == 0)
	    {
			ShowTeam(GROUP_TEAMTWO, false);
			return;
		}
		else if(stricmp(command, "HideTeam"))
		{
			ShowTeam(GROUP_UNKNOWN, false);
			return;
		}
	}

	BaseClass::OnCommand(command);
}

//=========================================================
//=========================================================
void CTeamChoice::ShowTeam(int index, bool bForceOut)
{
	Assert(index >= GROUP_UNKNOWN && index <= GROUP_TEAMTWO);
	m_pPreview->SetActiveGroup(index, bForceOut);
}

//=========================================================
//=========================================================
void CTeamChoice::SetTeam(int iTeam)
{
	Reset();

	switch(iTeam)
	{
	case TEAM_SELECT_ONE:
		m_pJoinOne->SetActive(true);
		break;
	case TEAM_SELECT_TWO:
		m_pJoinTwo->SetActive(true);
		break;
	case TEAM_SELECT_AUTOASSIGN:
		m_pAutoAssign->SetActive(true);
		break;
	case TEAM_SELECT_SPECTATOR:
		m_pSpectator->SetActive(true);
		break;
	}

	KeyValues *pMsg = new KeyValues("FinishTeam");
	pMsg->SetInt("team", iTeam);
	PostActionSignal(pMsg);

	m_bUsed = true;
}

//=========================================================
//=========================================================
void CTeamChoice::CreateTeamSelectPath(char *pszBuffer, const char *pszName)
{
	Q_snprintf(pszBuffer, MAX_PATH, "teams/%s/tselect/", pszName);
}
