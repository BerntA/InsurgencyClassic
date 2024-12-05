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
#include <vgui/ivgui.h>
#include <vgui/ilocalize.h>

#include <vgui/cursor.h>

#include <keyvalues.h>


#include <vgui_controls/imagepanel.h>
#include <vgui_controls/embeddedimage.h>

#include <vgui_controls/textentry.h>
#include <vgui_controls/checkbutton.h>
#include <cl_dll/iviewport.h>

#include "mapname_utils.h"

#include "ins_player_shared.h"

#include "playerlogin.h"
#include "ins_headers.h"

#include "play_team_shared.h"
#include "team_lookup.h"

#include "vgui_controls/vgui_helper.h"

#include "filesystem.h"

#include "hud_macros.h"

#include "stats_protocal.h"

#include "basic_colors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
ConVar cl_stats_savepass("cl_stats_savepass", "0", FCVAR_ARCHIVE, "");

//=========================================================
//=========================================================
CreateBasicINSHeader(PlayerLogin);

class CLoginInfo : public TextEntry
{
	DECLARE_CLASS_SIMPLE(CLoginInfo, TextEntry);

public:
	CLoginInfo(CPlayerLogin *pParent, const char *pszPanelName)
		: TextEntry(pParent, pszPanelName)
	{
		m_pPlayerLogin = pParent;

		SetVerticalScrollbar(false);
		SetAllowNonAsciiCharacters(false);
		SetMultiline(false);
	}

	virtual void Reset(void)
	{
		SetEnabled(true);
		m_bShowTitle = false;
	}

	bool ShowTitle(void) const
	{
		return m_bShowTitle;
	}

protected:
	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		SetBorder(NULL);
	}

	virtual void OnMousePressed(MouseCode code)
	{
		if(!IsEnabled())
			return;

		if(m_bShowTitle)
		{
			SetText("");
			m_bShowTitle = false;
		}

		BaseClass::OnMousePressed(code);
	}

	virtual void OnKeyTyped(wchar_t unichar)
	{
		BaseClass::OnKeyTyped(unichar);

		m_pPlayerLogin->UpdateCanLogin();
	}

	void InsertConVar(ConVar &ConVar)
	{
		const char *pszString = ConVar.GetString();

		if(*pszString)
		{
			SetText(pszString);
		}
		else
		{
			m_bShowTitle = true;
			SetTitle();
		}
	}

	virtual void SetTitle(void) = 0;

private:
	CPlayerLogin *m_pPlayerLogin;
	bool m_bShowTitle;
};

class CUsernameInfo : public CLoginInfo
{
	DECLARE_CLASS_SIMPLE(CUsernameInfo, CLoginInfo);

public:
	CUsernameInfo(CPlayerLogin *pParent)
		: CLoginInfo(pParent, "Username")
	{
	}

	virtual void Reset(void)
	{
		BaseClass::Reset();

		InsertConVar(cl_stats_username);
	}

protected:
	virtual void SetTitle(void)
	{
		SetText("<username>");
	}
};

class CPasswordInfo : public CLoginInfo
{
	DECLARE_CLASS_SIMPLE(CPasswordInfo, CLoginInfo);

public:
	CPasswordInfo(CPlayerLogin *pParent)
		: CLoginInfo(pParent, "Password")
	{
	}

	virtual void Reset(void)
	{
		BaseClass::Reset();

		InsertConVar(cl_stats_password);
	}

protected:
	virtual void SetText(const char *text)
	{
		BaseClass::SetText(text);
		SetTextHidden(true);
	}

	virtual void SetTitle(void)
	{
		SetText("<password>");
		SetTextHidden(false);
	}
};

//=========================================================
//=========================================================
class CSavePassword : public ToggleButton
{
	DECLARE_CLASS_SIMPLE( CSavePassword, ToggleButton );

public:
	CSavePassword(Panel *pParent)
		: ToggleButton(pParent, "SavePassword", "Save Password")
	{
		SetPaintBackgroundEnabled(false);

		m_pCheckImage = new ImagePanel(this, NULL);
		m_pCheckImage->SetShouldScaleImage(true);
		m_pCheckImage->SetPos(0, 0);
		m_pCheckImage->SetVisible(false);

		_selectedFgColor = Color(255, 255, 255, 255);
	}

	virtual	~CSavePassword()
	{
		delete m_pCheckImage;
	}

	void Reset(void)
	{
		SetSelected(cl_stats_savepass.GetBool());
		SetVisible(true);
	}

protected:
	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		SetDefaultColor(GetSchemeColor("SavePassword.TextColor", pScheme), GetBgColor());
		_selectedFgColor = GetSchemeColor("SavePassword.SelectedTextColor", GetSchemeColor("SavePassword.TextColor", pScheme), pScheme);

		const char *pszSize = pScheme->GetResourceString("SavePassword.Size");

		if(*pszSize)
		{
			int iSize = scheme()->GetProportionalScaledValue(atoi(pszSize));
			m_pCheckImage->SetSize(iSize, iSize);
		}

		m_pCheckImage->SetImage("resources/playerlogin/tick");

		SetImagePreOffset(0, m_pCheckImage->GetWide() - 2);
	}

	virtual void SetSelected(bool state)
	{
		BaseClass::SetSelected(state);

		m_pCheckImage->SetVisible(state);
	}

	virtual Color GetButtonFgColor()
	{
		if(IsSelected())
		{
			return _selectedFgColor;
		}

		return BaseClass::GetButtonFgColor();
	}

	virtual IBorder *GetBorder(bool depressed, bool armed, bool selected, bool keyfocus)
	{
		return NULL;
	}

private:
	ImagePanel *m_pCheckImage;
	Color _selectedFgColor;
};

//=========================================================
//=========================================================
const char *g_pszLoginResult[SAC_PLAYERTYPE_COUNT] = {
	"Server Appears to be Down.",		// SAC_PLAYERTYPE_SERVER
	"Your Information is Invalid.",		// SAC_PLAYERTYPE_INVALID
	"Your Account is Banned.",			// SAC_PLAYERTYPE_BANNED
	"Login Successful!",				// SAC_PLAYERTYPE_VALID
	"Login Successful!"					// SAC_PLAYERTYPE_DEVELOPER
};

class CLoginImage : public EmbeddedImage
{
	DECLARE_CLASS_SIMPLE( CLoginImage, EmbeddedImage );

	friend class CLoginMask;

	class CLoginMask : public Panel
	{
		DECLARE_CLASS_SIMPLE( CLoginMask, Panel );

	public:
		CLoginMask(CLoginImage *pParent)
			: Panel(pParent)
		{
			m_pParent = pParent;

			Reset();
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

			m_iResultX = (GetWide() / 2) - (UTIL_ComputeStringWidth(m_ResultFont, pszResultText) / 2);

			switch(iType)
			{
			case SAC_PLAYERTYPE_SERVER:
			case SAC_PLAYERTYPE_INVALID:
			case SAC_PLAYERTYPE_BANNED:
				m_ResultColor = COLOR_RED;

			#ifdef STATS_PROTECTION
				m_pParent->m_pParent->m_bLoginSuccess = false;
			#endif

				break;
			default:
			//case SAC_PLAYERTYPE_VALID:
			//case SAC_PLAYERTYPE_DEVELOPER:
				m_ResultColor = COLOR_GREEN;

			#ifdef STATS_PROTECTION
				m_pParent->m_pParent->m_bLoginSuccess = true;
			#endif
			}

			m_bShowResult = true;
		}

	protected:
		virtual void ApplySchemeSettings(IScheme *pScheme)
		{
			BaseClass::ApplySchemeSettings(pScheme);

			SetBgColor(pScheme->GetColor("PlayerLogin.MaskColor", GetBgColor()));

			m_ResultFont = pScheme->GetFont("DefaultExtraBig");
		}

		virtual void OnSizeChanged(int newWide, int newTall)
		{
			BaseClass::OnSizeChanged(newWide, newTall);

			m_iResultY = (newTall / 2) + (newTall / 4);
		}

		virtual void Paint(void)
		{
			BaseClass::Paint();

			if(!m_bShowResult)
				return;

			surface()->DrawSetTextFont(m_ResultFont);
			surface()->DrawSetTextPos(m_iResultX, m_iResultY);
			surface()->DrawSetTextColor(m_ResultColor);
			surface()->DrawPrintText(m_szResult, m_iResultLength);
		}

	private:
		CLoginImage *m_pParent;
		bool m_bShowResult;
		HFont m_ResultFont;
		int m_iResultX, m_iResultY;
		Color m_ResultColor;
		wchar_t m_szResult[128];
		int m_iResultLength;
	};

public:
	CLoginImage(CPlayerLogin *pParent)
		: EmbeddedImage(pParent, "Background")
	{
		m_pParent = pParent;

		m_pMask = new CLoginMask(this);
		m_pMask->MoveToFront();

		Reset();		
	}

	void Reset(void)
	{
		m_pMask->Reset();
		SetMaskVisible(false);
	}

	void SetResult(int iType)
	{
		m_iResult = iType;
		m_pMask->SetResult(iType);
	}

	void SetMaskVisible(bool bState)
	{
		m_pMask->SetVisible(bState);
	}

protected:
	virtual void ApplySettings(KeyValues *inResourceData)
	{
		BaseClass::ApplySettings(inResourceData);

		m_pMask->SetSize(GetWide(), GetTall());
	}

private:
	CPlayerLogin *m_pParent;
	CLoginMask *m_pMask;
	int m_iResult;
};

//=========================================================
//=========================================================
class CLoginTicker : public Panel
{
	DECLARE_CLASS_SIMPLE( CLoginTicker, Panel );

	#define INVALID_BLOCK -1

	#define TICKER_COUNT 3
	#define TICKER_TICK 25

public:
	CLoginTicker(Panel *pParent)
		: Panel(pParent, "Progress")
	{
		SetPaintBackgroundEnabled(false);
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
	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		m_MaskColor = pScheme->GetColor("PlayerLogin.MaskColor", GetBgColor());
	}

	virtual void ApplySettings(KeyValues *inResourceData)
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
		surface()->DrawSetColor(m_MaskColor);

		for(int i = 0; i < TICKER_COUNT; i++)
		{
			surface()->DrawTexturedRect(iXOffset, 0, iXOffset + m_iBlockWide, m_iBlockTall);
			surface()->DrawFilledRect(iXOffset, 0, iXOffset + m_iBlockWide, m_iBlockTall);
			iXOffset += m_iBlockWide;
		}
	}

	virtual void OnTick(void)
	{
		if(!m_bIsTicking)
			m_bIsTicking = false;

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

private:
	int m_iBlockTexID;
	int m_iBlockWide, m_iBlockTall;

	bool m_bIsTicking;
	int m_iXOffsetTicker;

	Color m_MaskColor;
};

//=========================================================
//=========================================================
void __MsgFunc_PlayerLogin(bf_read &Msg)
{
	if(g_pPlayerLogin)
		g_pPlayerLogin->HandleMsg(Msg);
}

//=========================================================
//=========================================================
CPlayerLogin *g_pPlayerLogin = NULL;

CPlayerLogin::CPlayerLogin(IViewPort *pViewPort)
	: INSFrame(NULL, PANEL_PLAYERLOGIN)
{
	Assert(!g_pPlayerLogin);
	g_pPlayerLogin = this;

	SetScheme("ClientScheme");
	SetProportional(true);

	HOOK_MESSAGE(PlayerLogin);

	m_pBackground = new CLoginImage(this);
	m_pUsername = new CUsernameInfo(this);
	m_pPassword = new CPasswordInfo(this);
	m_pSavePassword = new CSavePassword(this);
	m_pTicker = new CLoginTicker(this);

	m_iButtonLogin = AddButton("Login", "Login", true);
	m_iButtonClose = AddButton("Close", "Close");
	m_iButtonReset = AddButton("Reset", "Reset");

	Panel *pButtonLogin = GetButton(m_iButtonLogin);
	GetFocusNavGroup().SetDefaultButton(pButtonLogin);

	pButtonLogin->SetTabPosition(1);
	m_pUsername->SetTabPosition(2);
	m_pPassword->SetTabPosition(3);
	m_pSavePassword->SetTabPosition(4);

	AddHeader(new CPlayerLoginHeader(this));

	LoadControlSettings("Resource/UI/Frames/PlayerLogin.res");

	Reset();

	m_pBackground->SetParent(GetClientArea());
	m_pUsername->SetParent(GetClientArea());
	m_pPassword->SetParent(GetClientArea());
	m_pSavePassword->SetParent(GetClientArea());
	m_pTicker->SetParent(GetClientArea());
}

//=========================================================
//=========================================================
CPlayerLogin::~CPlayerLogin()
{
	g_pPlayerLogin = NULL;
}

//=========================================================
//=========================================================
void CPlayerLogin::Reset( void )
{
	m_pUsername->Reset();
	m_pPassword->Reset();
	m_pSavePassword->Reset();
	m_pBackground->Reset();

	Panel *pButtonReset = GetButton(m_iButtonReset);
	pButtonReset->SetEnabled(false);

	UpdateCanLogin();

#ifdef STATS_PROTECTION
	m_bLoginSuccess = false;
#endif
}

//=========================================================
//=========================================================
void CPlayerLogin::HandleMsg(bf_read &Msg)
{
	Panel *pButtonClose = GetButton(m_iButtonClose);
	pButtonClose->SetEnabled(true);

	int iPlayerType = Msg.ReadByte();
	Assert(iPlayerType >= 0 && iPlayerType <= SAC_PLAYERTYPE_COUNT);

	m_pBackground->SetResult(iPlayerType);

	if(iPlayerType == SAC_PLAYERTYPE_INVALID)
	{
		Panel *pButtonReset = GetButton(m_iButtonReset);
		pButtonReset->SetEnabled(true);
	}

	m_pTicker->SetTicking(false);
}

//=========================================================
//=========================================================
void CPlayerLogin::OnCommand(const char *command)
{
	 if(!stricmp(command, "Login"))
	 {
		// send message
		char szUsername[256];
		char szPassword[256];

		m_pUsername->GetText(szUsername, sizeof(szUsername));
		m_pPassword->GetText(szPassword, sizeof(szPassword));

		char szPlayerLogin[256];
		Q_snprintf(szPlayerLogin, sizeof(szPlayerLogin), "playerlogin %s %s",
			szUsername, szPassword);

		engine->ClientCmd(szPlayerLogin);

		// store info
		cl_stats_username.SetValue(szUsername);

		if(!m_pSavePassword->IsSelected())
			szPassword[0] = '\0';

		cl_stats_password.SetValue(szPassword);

		cl_stats_savepass.SetValue(m_pSavePassword->IsSelected());

		// hide buttons
		Panel *pButtonLogin = GetButton(m_iButtonLogin);
		Panel *pButtonClose = GetButton(m_iButtonClose);
		pButtonLogin->SetEnabled(false);
		pButtonClose->SetEnabled(false);

		// show mask
		m_pBackground->SetMaskVisible(true);
		m_pUsername->SetEnabled(false);
		m_pPassword->SetEnabled(false);
		m_pSavePassword->SetVisible(false);

		// make the ticker show
		m_pTicker->SetTicking(true);

		return;
	 }
	 else if(!stricmp(command, "Close"))
	 {
#ifdef STATS_PROTECTION
		 if(!m_bLoginSuccess)
			 return;
#endif
		 ShowPanel(false);
		 engine->ClientCmd("changeteam");
	 }
	 else if(!stricmp(command, "Reset"))
	 {
		cl_stats_username.SetValue("");
		cl_stats_password.SetValue("");
		cl_stats_savepass.SetValue("0");

		Reset();
	 }

	BaseClass::OnCommand(command);
}

//=========================================================
//=========================================================
void CPlayerLogin::UpdateCanLogin(void)
{
	bool bEnabled = false;

	if(!m_pUsername->ShowTitle() && !m_pPassword->ShowTitle() &&
		m_pUsername->GetTextLength() != 0 && m_pPassword->GetTextLength() != 0)
		bEnabled = true;

	Panel *pButtonLogin = GetButton(m_iButtonLogin);
	pButtonLogin->SetEnabled(bEnabled);
}

//=========================================================
//=========================================================
void CPlayerLogin::ShowPanel(bool bShow)
{
	if(IsVisible() == bShow)
		return;

	Assert(g_pGameRules);

	if(bShow)
	{
		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}
}
