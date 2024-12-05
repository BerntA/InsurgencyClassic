//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <vgui/vgui.h>
#include <vgui/iinput.h>
#include <vgui_controls/frame.h>
#include <vgui_controls/textentry.h>
#include "gameuipanel.h"
#include <convar.h>
#include <keyvalues.h>
#include "ins_player_shared.h"
#include <vgui/ivgui.h>
#include <vgui/ilocalize.h>
#include <vgui/isurface.h>
#include "basic_colors.h"
#include "ins_stats_shared.h"
#include "stats_protocal.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
using namespace vgui;

class CUsernameInfo;
class CPasswordInfo;
class CLoginTicker;
class CLoginResult;

//=========================================================
//=========================================================
#define ROT13_SHIFT 10

#define ROT13_LIMIT_LOWER 65
#define ROT13_LIMIT_UPPER 122

void EncodeROT13Element(char *pcTarget)
{
	if(*pcTarget > ROT13_LIMIT_UPPER)
	{
		*pcTarget = ROT13_LIMIT_LOWER + (*pcTarget - ROT13_LIMIT_UPPER);
		return;
	}
}

int WithinLimitROT13(char cElement)
{
	return (cElement >= ROT13_LIMIT_LOWER && cElement <= ROT13_LIMIT_UPPER);
}

void EncodeROT13(char *pszBuffer)
{
	int i;
	char *pcElement;

	for(i = 0, pcElement = &pszBuffer[0]; *pcElement != '\0'; i++, pcElement = &pszBuffer[i])
	{
		if(!WithinLimitROT13(*pcElement))
			continue;

		*pcElement += ROT13_SHIFT;

		if(WithinLimitROT13(*pcElement))
			continue;

		EncodeROT13Element(pcElement);
	}
}

void DecodeROT13Element(char *pcTarget)
{
	if(*pcTarget < ROT13_LIMIT_LOWER)
	{
		*pcTarget = ROT13_LIMIT_UPPER - (ROT13_LIMIT_LOWER - *pcTarget);
		return;
	}
}

void DecodeROT13(char *pszBuffer)
{
	int i;
	char *pcElement;

	for(i = 0, pcElement = &pszBuffer[0]; *pcElement != '\0'; i++, pcElement = &pszBuffer[i])
	{
		if(!WithinLimitROT13(*pcElement))
			continue;

		*pcElement -= ROT13_SHIFT;

		if(WithinLimitROT13(*pcElement))
			continue;

		DecodeROT13Element(pcElement);
	}
}

//=========================================================
//=========================================================
class OptionsSubMultiplayerStatsDlg : public Frame, public CStatsHelper
{
	DECLARE_CLASS_SIMPLE(OptionsSubMultiplayerStatsDlg, Frame);

public:
	OptionsSubMultiplayerStatsDlg();
	~OptionsSubMultiplayerStatsDlg();

	void OnCommand(const char *command);
	void Activate(void);

	void UpdateCanLogin(void);

	void LoginCallback(int iMemberID, int iType);

private:
	bool IsUsingStats(void);

private:
	CUsernameInfo *m_pUsername;
	CPasswordInfo *m_pPassword;
	CLoginTicker *m_pTicker;
	CLoginResult *m_pResult;

	Panel *m_pOKButton, *m_pResetButton, *m_pLoginButton;
};

//=========================================================
//=========================================================
class CLoginEntry : public TextEntry
{
	DECLARE_CLASS_SIMPLE(CLoginEntry, TextEntry);

public:
	CLoginEntry(OptionsSubMultiplayerStatsDlg *pParent, const char *pszPanelName)
		: TextEntry(pParent, pszPanelName)
	{
		m_pParent = pParent;

		Reset();

		SetVerticalScrollbar(false);
		SetAllowNonAsciiCharacters(false);
		SetMultiline(false);
	}

	void Reset(void)
	{
		m_bShowTitle = false;
	}

	bool ShowTitle(void) const
	{
		return m_bShowTitle;
	}

protected:
	void OnMousePressed(MouseCode code)
	{
		if(!IsEnabled())
			return;

		RemoveTitle();

		BaseClass::OnMousePressed(code);
	}

	void OnKeyCodeTyped(KeyCode code)
	{
		BaseClass::OnKeyCodeTyped(code);

		m_pParent->UpdateCanLogin();
	}

	void OnKeyTyped(wchar_t unichar)
	{
		BaseClass::OnKeyTyped(unichar);

		m_pParent->UpdateCanLogin();
	}

	void OnSetFocus(void)
	{
		BaseClass::OnSetFocus();

		RemoveTitle();
	}

	void InsertConVar(ConVar &ConVar, bool bDecode)
	{
		char szString[128];
		const char *pszString = ConVar.GetString();

		if(bDecode)
		{
			Q_strncpy(szString, pszString, sizeof(szString));
			DecodeROT13(szString);

			pszString = szString;
		}

		if(*pszString)
		{
			SetText(pszString);
		}
		else
		{
			SetTitle();
			m_bShowTitle = true;
		}
	}

	virtual void SetTitle(void) = 0;

	void RemoveTitle(void)
	{
		if(m_bShowTitle)
		{
			SetText("");
			m_bShowTitle = false;
		}
	}

private:
	OptionsSubMultiplayerStatsDlg *m_pParent;
	bool m_bShowTitle;
};

//=========================================================
//=========================================================
class CUsernameInfo : public CLoginEntry
{
	DECLARE_CLASS_SIMPLE(CUsernameInfo, CLoginEntry);

public:
	CUsernameInfo(OptionsSubMultiplayerStatsDlg *pParent)
		: CLoginEntry(pParent, "Username")
	{
	}

	void Update(void)
	{
		Reset();

		InsertConVar(cl_stats_username, false);
	}

private:
	void SetTitle(void)
	{
		SetText("<username>");
	}
};

//=========================================================
//=========================================================
class CPasswordInfo : public CLoginEntry
{
	DECLARE_CLASS_SIMPLE(CPasswordInfo, CLoginEntry);

public:
	CPasswordInfo(OptionsSubMultiplayerStatsDlg *pParent)
		: CLoginEntry(pParent, "Password")
	{
	}

	void Update(void)
	{
		Reset();

		InsertConVar(cl_stats_password, true);
	}

private:
	void SetText(const char *pszText)
	{
		BaseClass::SetText(pszText);
		SetTextHidden(true);
	}

	void SetTitle(void)
	{
		SetText("<password>");
		SetTextHidden(false);
	}
};

//=========================================================
//=========================================================
class CLoginTicker : public Panel
{
	DECLARE_CLASS_SIMPLE(CLoginTicker, Panel);

	#define INVALID_BLOCK -1

	#define TICKER_COUNT 3
	#define TICKER_TICK 25

private:
	int m_iBlockTexID;
	int m_iBlockWide, m_iBlockTall;

	bool m_bIsTicking;
	int m_iXOffsetTicker;

public:
	CLoginTicker(Panel *pParent)
		: Panel(pParent, "Progress")
	{
		SetPaintBackgroundEnabled(true);
		m_iBlockTexID = INVALID_BLOCK;
		m_bIsTicking = false;

		ivgui()->AddTickSignal(GetVPanel(), TICKER_TICK);
	}

	void SetTicking(bool bState)
	{
		if(bState)
			ResetTicker();

		m_bIsTicking = bState;
	}

protected:
	void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		SetBorder(pScheme->GetBorder("BorderNormal"));
	}

	void ApplySettings(KeyValues *inResourceData)
	{
		BaseClass::ApplySettings(inResourceData);

		const char *pszBlock = inResourceData->GetString("block");

		if(*pszBlock)
		{
			const char *pszBlockWide = inResourceData->GetString("block_wide");
			const char *pszBlockTall = inResourceData->GetString("block_tall");

			if(*pszBlockWide && *pszBlockTall)
			{
				m_iBlockWide = scheme()->GetProportionalScaledValue(atoi(pszBlockWide));
				m_iBlockTall = scheme()->GetProportionalScaledValue(atoi(pszBlockTall));

				m_iBlockTexID = surface()->CreateNewTextureID();
				surface()->DrawSetTextureFile(m_iBlockTexID, pszBlock, false, false);
			}
		}
	}

	void Paint(void)
	{
		if(m_iBlockTexID == INVALID_BLOCK || !m_bIsTicking)
			return;

		int iXOffset = m_iXOffsetTicker;

		surface()->DrawSetTexture(m_iBlockTexID);
		surface()->DrawSetColor(COLOR_WHITE);

		for(int i = 0; i < TICKER_COUNT; i++)
		{
			surface()->DrawTexturedRect(iXOffset, 0, iXOffset + m_iBlockWide, m_iBlockTall);
			iXOffset += m_iBlockWide;
		}
	}

	void OnTick(void)
	{
		if(!m_bIsTicking)
			return;

		if(m_iXOffsetTicker >= GetWide())
		{
			ResetTicker();
			return;
		}

		m_iXOffsetTicker += (m_iBlockWide / 2);
	}

private:
	void ResetTicker(void)
	{
		m_iXOffsetTicker = -(m_iBlockWide * TICKER_COUNT);
	}
};

//=========================================================
//=========================================================
const char *g_pszLoginResult[SAC_PLAYERTYPE_COUNT] = {
	"Server Issues.",					// SAC_PLAYERTYPE_SERVER
	"Your Information is Invalid.",		// SAC_PLAYERTYPE_INVALID
	"Your Account is Banned.",			// SAC_PLAYERTYPE_BANNED
	"Login Successful!",				// SAC_PLAYERTYPE_VALID
	"Login Successful!"					// SAC_PLAYERTYPE_DEVELOPER
};

class CLoginResult : public EditablePanel
{
	DECLARE_CLASS_SIMPLE(CLoginResult, Panel);

private:
	bool m_bShowResult;

	HFont m_ResultFont;
	Color m_ResultColor;

	wchar_t m_szResult[128];
	int m_iResultLength;

public:
	CLoginResult(Panel *pParent)
		: EditablePanel(pParent, "LoginResult")
	{
		Reset();

		SetPaintBackgroundEnabled(true);
	}

	void Reset(void)
	{
		m_bShowResult = false;
	}

	void SetResult(int iType)
	{
		const char *pszResultText = g_pszLoginResult[iType];

		localize()->ConvertANSIToUnicode(pszResultText, m_szResult, sizeof(m_szResult));	
		m_iResultLength = Q_strlen(pszResultText);

		switch(iType)
		{
		case SAC_PLAYERTYPE_SERVER:
		case SAC_PLAYERTYPE_INVALID:
		case SAC_PLAYERTYPE_BANNED:
			m_ResultColor = COLOR_RED;
			break;
		case SAC_PLAYERTYPE_VALID:
		case SAC_PLAYERTYPE_DEVELOPER:
			m_ResultColor = COLOR_GREEN;
			break;
		default:
			return;
		}

		m_bShowResult = true;
	}

private:
	void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		m_ResultFont = pScheme->GetFont("DefaultBold");
	}

	void Paint(void)
	{
		if(!m_bShowResult)
			return;

		surface()->DrawSetTextFont(m_ResultFont);
		surface()->DrawSetTextPos(0, 0);
		surface()->DrawSetTextColor(m_ResultColor);
		surface()->DrawPrintText(m_szResult, m_iResultLength);
	}
};

//=========================================================
//=========================================================
OptionsSubMultiplayerStatsDlg::OptionsSubMultiplayerStatsDlg()
	: Frame(NULL, "OptionsSubMultiplayerStatsDlg")
{
	SetParent(g_pGameUIPanel->GetVParent());
	SetScheme("SourceScheme");
	SetSizeable(false);

	m_pOKButton = m_pResetButton = m_pLoginButton = NULL;

	m_pUsername = new CUsernameInfo(this);
	m_pPassword = new CPasswordInfo(this);
	m_pTicker = new CLoginTicker(this);
	m_pResult = new CLoginResult(this);

	LoadControlSettings("resource/OptionsSubMultiplayerStatsDlg.res");
}

//=========================================================
//=========================================================
OptionsSubMultiplayerStatsDlg::~OptionsSubMultiplayerStatsDlg()
{
	delete m_pUsername;
	delete m_pPassword;
	delete m_pTicker;
}

//=========================================================
//=========================================================
void OptionsSubMultiplayerStatsDlg::OnCommand(const char *pszCommand)
{
	if(FStrEq(pszCommand, "Login"))
	{
		// send message
		char szUsername[128], szPassword[128];

		m_pUsername->GetText(szUsername, sizeof(szUsername));
		m_pPassword->GetText(szPassword, sizeof(szPassword));

		// hide buttons
		if(m_pOKButton)
			m_pOKButton->SetEnabled(false);

		if(m_pResetButton)
			m_pResetButton->SetEnabled(false);

		if(m_pLoginButton)
			m_pLoginButton->SetEnabled(false);

		// show mask
		m_pUsername->SetEnabled(false);
		m_pPassword->SetEnabled(false);

		// make the ticker show
		m_pTicker->SetTicking(true);

		// clear past result
		m_pResult->Reset();

		// send off to server
		GetINSStats()->LoginPlayer(NULL, szUsername, szPassword);

		// login through the game if its running
		if(engine->IsInGame())
			GetINSStats()->SetupPlayer(szUsername, szPassword);
	}
	else if(FStrEq(pszCommand, "Reset"))
	{
		cl_stats_username.SetValue("");
		cl_stats_password.SetValue("");

		m_pUsername->Update();
		m_pPassword->Update();

		UpdateCanLogin();
	}
	else
	{
		input()->ReleaseAppModalSurface();
		SetVisible(false);
		PostMessage(this,new KeyValues("Delete"),0.0001f);
	}
}

//=========================================================
//=========================================================
void OptionsSubMultiplayerStatsDlg::Activate(void)
{
	BaseClass::Activate();

	input()->SetAppModalSurface(GetVPanel());

	// find buttons
	m_pOKButton = FindChildByName("Button1");
	m_pResetButton = FindChildByName("Button2");
	m_pLoginButton = FindChildByName("Button3");

	// update and reset input elements
	m_pUsername->Update();
	m_pPassword->Update();
	m_pResult->Reset();

	// setup tab positions
	m_pUsername->SetTabPosition(1);
	m_pPassword->SetTabPosition(2);

	if(m_pOKButton)
		m_pOKButton->SetTabPosition(3);

	if(m_pResetButton)
		m_pResetButton->SetTabPosition(4);

	if(m_pLoginButton)
		m_pLoginButton->SetTabPosition(5);

	// if stats are running, disable input
	if(IsUsingStats())
	{
		m_pUsername->SetEnabled(false);
		m_pPassword->SetEnabled(false);

		if(m_pResetButton)
			m_pResetButton->SetEnabled(false);
	}

	// update login button
	UpdateCanLogin();
}

//=========================================================
//=========================================================
void OptionsSubMultiplayerStatsDlg::UpdateCanLogin(void)
{
	if(!m_pLoginButton)
		return;

	bool bEnabled = false;

	if(!IsUsingStats() && !m_pUsername->ShowTitle() && !m_pPassword->ShowTitle() &&
		m_pUsername->GetTextLength() != 0 && m_pPassword->GetTextLength() != 0)
		bEnabled = true;

	m_pLoginButton->SetEnabled(bEnabled);
}

//=========================================================
//=========================================================
void OptionsSubMultiplayerStatsDlg::LoginCallback(int iMemberID, int iType)
{
	// stop ticking
	m_pTicker->SetTicking(false);

	// send off result
	m_pResult->SetResult(iType);

	// save off information if valid
	bool bIsValid = (iType == SAC_PLAYERTYPE_VALID || iType == SAC_PLAYERTYPE_DEVELOPER);

	if(bIsValid)
	{
		char szUsername[128], szPassword[128];

		// get values
		m_pUsername->GetText(szUsername, sizeof(szUsername));
		m_pPassword->GetText(szPassword, sizeof(szPassword));

		// encode password
		EncodeROT13(szPassword);

		// store values
		cl_stats_username.SetValue(szUsername);
		cl_stats_password.SetValue(szPassword);
	}
	else
	{
		// ... otherwise reset password
		m_pPassword->Reset();
	}

	// show buttons
	if(m_pOKButton)
		m_pOKButton->SetEnabled(true);

	if(m_pResetButton)
		m_pResetButton->SetEnabled(true);

	// only reset these elements if login failed
	if(!engine->IsInGame() || (engine->IsInGame() && !bIsValid))
	{
		if(m_pLoginButton)
			m_pLoginButton->SetEnabled(true);

		// show input
		m_pUsername->SetEnabled(true);
		m_pPassword->SetEnabled(true);
	}

	// update login button
	UpdateCanLogin();
}

//=========================================================
//=========================================================
bool OptionsSubMultiplayerStatsDlg::IsUsingStats(void)
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();

	if(!pPlayer)
		return false;

	return pPlayer->IsUsingStats();
}

//=========================================================
//=========================================================
CON_COMMAND_F(vgui_showstats, "Show Stats Dialog", FCVAR_CLIENTDLL)
{
	(new OptionsSubMultiplayerStatsDlg())->Activate();
}