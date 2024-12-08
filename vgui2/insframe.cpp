//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "vgui/IInput.h"
#include "vgui/IPanel.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/insframe.h"
#include "vgui_controls/Button.h"

#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SCANLINES_GAP 2
#define DEFAULT_TITLE_GAP 3

using namespace vgui;

//=========================================================
//=========================================================
INSFrameHeader::INSFrameHeader(INSFrame* pParent, const char* pszName, int iTall)
	: EditablePanel(pParent, pszName)
{
	SetProportional(true);

	m_iOffset = 0;
}

//=========================================================
//=========================================================
void INSFrameHeader::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBgColor(pScheme->GetColor("FrameHeader.BgColor", GetBgColor()));

	int iHeaderWidth, iHeaderTall;
	GetSize(iHeaderWidth, iHeaderTall);

	if (UseDefaultTall())
		iHeaderTall = scheme()->GetProportionalScaledValue(DEFAULT_FRAMEHEADER_HEIGHT);

	SetSize(iHeaderWidth, iHeaderTall);

	SetupSize();
}

//=========================================================
//=========================================================
void INSFrameHeader::ApplySettings(KeyValues* inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	const char* pszOffset = inResourceData->GetString("offset", "");

	if (*pszOffset)
		SetOffset(atoi(pszOffset));
}

//=========================================================
//=========================================================
void INSFrameHeader::SetupSize(void)
{
	int iFrameWide, iFrameTall;
	GetParent()->GetSize(iFrameWide, iFrameTall);

	int iHeaderWide, iHeaderTall;
	GetSize(iHeaderWide, iHeaderTall);

	SetSize(iFrameWide, iHeaderTall);
}

//=========================================================
//=========================================================
void INSFrameHeader::SetOffset(int iOffset)
{
	int iHeaderWide, iHeaderTall;
	GetSize(iHeaderWide, iHeaderTall);

	if (iOffset >= iHeaderTall)
		return;

	m_iOffset = scheme()->GetProportionalScaledValue(iOffset);
}

//=========================================================
//=========================================================
INSTextHeader::INSTextHeader(INSFrame* pParent, const char* pszName)
	: INSFrameHeader(pParent, pszName)
{
	m_hFont = vgui::INVALID_FONT;

	Reset();
}

//=========================================================
//=========================================================
void INSTextHeader::Reset(void)
{
	m_iStringXPos, m_iStringYPos = 0;

	m_szText[0] = '\0';
	m_iTextLength = 0;
}

//=========================================================
//=========================================================
void INSTextHeader::SetText(const char* pszText)
{
	// convert text to unicode
	g_pVGuiLocalize->ConvertANSIToUnicode(pszText, m_szText, sizeof(m_szText));
	m_iTextLength = Q_strlen(pszText);

	// setup position
	if (m_hFont == vgui::INVALID_FONT)
		return;

	int iWide, iTall;
	GetSize(iWide, iTall);

	int iTotalStringWidth = 0;

	for (const wchar_t* pch = m_szText; *pch != 0; pch++)
		iTotalStringWidth += surface()->GetCharacterWidth(m_hFont, *pch);

	m_iStringXPos = (iWide / 2) - (iTotalStringWidth / 2);
	m_iStringYPos = (iTall / 2) - (surface()->GetFontTall(m_hFont) / 2);
}

//=========================================================
//=========================================================
void INSTextHeader::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// set text appearance
	m_hFont = pScheme->GetFont("Default");
	m_TextColor = pScheme->GetColor("INSTextHeader.TextColor", GetFgColor());
}

//=========================================================
//=========================================================
void INSTextHeader::Paint(void)
{
	if (m_iTextLength == 0)
		return;

	Assert(m_hFont != vgui::INVALID_FONT);

	surface()->DrawSetTextFont(m_hFont);
	surface()->DrawSetTextPos(m_iStringXPos, m_iStringYPos);
	surface()->DrawSetTextColor(m_TextColor);

	surface()->DrawPrintText(m_szText, m_iTextLength);
}

//=========================================================
//=========================================================
INSTopHeader::INSTopHeader(INSFrame* pParent, const char* pszName)
	: INSFrameHeader(pParent, pszName)
{
	SetPaintBackgroundEnabled(false);
}

//=========================================================
//=========================================================
namespace vgui
{
	class INSFrameClientBackground : public Panel
	{
		DECLARE_CLASS_SIMPLE(INSFrameClientBackground, Panel);

	public:
		INSFrameClientBackground(INSFrame* pParent)
			: Panel(pParent)
		{
			m_iScanLines = 0;
		}

		virtual void ApplySchemeSettings(IScheme* pScheme)
		{
			BaseClass::ApplySchemeSettings(pScheme);

			SetBgColor(pScheme->GetColor("Frame.BgColor", GetBgColor()));
			SetBorder(pScheme->GetBorder("FrameBorder"));

			m_ScanLineColor = pScheme->GetColor("Frame.ScanLinesColor", GetBgColor());
		}

		virtual void OnSizeChanged(int newWide, int newTall)
		{
			if (newTall == 0)
			{
				m_iScanLines = 0;
				return;
			}

			m_iScanLines = newTall / (SCANLINES_GAP + 1);
		}

		virtual void Paint(void)
		{
			int iXPos, iYPos;
			GetPos(iXPos, iYPos);

			int iWide, iTall;
			GetSize(iWide, iTall);

			for (int i = 0; i < m_iScanLines; i++)
			{
				int iScanLineYPos = i + (SCANLINES_GAP * i);

				surface()->DrawSetColor(m_ScanLineColor);
				surface()->DrawFilledRect(0, iScanLineYPos, iWide, iScanLineYPos + 1);
			}

			BaseClass::Paint();
		}

	private:
		int m_iScanLines;
		Color m_ScanLineColor;
	};

}; // namespace vgui

//=========================================================
//=========================================================
INSFrame::INSFrame(Panel* pParent, const char* pszPanelName) : EditablePanel(pParent, pszPanelName)
{
	// start invisible and ensure the panels independence
	SetVisible(false);
	MakePopup(false);

	SetProportional(true);

	// never draw the background
	SetPaintBackgroundEnabled(false);

	// set a slightly larger min size
	SetMinimumSize(128, 66);

	// create the client area
	m_pClientBackground = new INSFrameClientBackground(this);
	m_pClientBackground->SetVisible(false);

	m_pClientArea = new Panel(m_pClientBackground);
	m_pClientArea->SetPaintBackgroundEnabled(false);
	m_pClientArea->SetVisible(false);

	// by default, you cannot quick close
	m_bQuickClose = false;

	// ensure this has top-level focus
	GetFocusNavGroup().SetFocusTopLevel(true);

	// ensure a scaled title gap and correct areas
	m_iTitleGap = scheme()->GetProportionalScaledValue(DEFAULT_TITLE_GAP);
}
//=========================================================
//=========================================================
INSFrame::~INSFrame()
{
	m_Headers.Purge();
}

//=========================================================
//=========================================================
void INSFrame::AddHeader(INSFrameHeader* pHeader)
{
	// now add the new header into the list (top)
	m_Headers.AddToHead(pHeader);

	pHeader->SetVisible(false);

	// recalculate the areas
	//RecalculateAreas();
}

//=========================================================
//=========================================================
int INSFrame::AddButton(const char* pszTitle, const char* pszCommand, bool bDefault)
{
	Button* pButton = new Button(m_pClientBackground, NULL, pszTitle, this, pszCommand);

	if (bDefault)
		GetFocusNavGroup().SetDefaultButton(pButton);

	int iButtonID = m_Buttons.AddToTail(pButton);
	pButton->SetTabPosition(iButtonID + 1);

	return iButtonID;
}

//=========================================================
//=========================================================
Button* INSFrame::GetButton(int iID)
{
	return m_Buttons[iID];
}

//=========================================================
//=========================================================
void INSFrame::Activate(void)
{
	MoveToFront();
	RequestFocus();
	SetVisible(true);
	SetEnabled(true);

	m_pClientBackground->SetVisible(true);
	m_pClientArea->SetVisible(true);

	for (int i = 0; i < m_Headers.Count(); i++)
		m_Headers[i]->SetVisible(true);
}

//=========================================================
//=========================================================
void INSFrame::EnableQuickClose(bool state)
{
	m_bQuickClose = state;
}

//=========================================================
//=========================================================
void INSFrame::Close()
{
	OnClose();
}

//=========================================================
//=========================================================
void INSFrame::OnMousePressed(MouseCode Code)
{
	// if a child doesn't have focus, get it for ourselves
	VPANEL Focus = input()->GetFocus();

	if (!Focus || !ipanel()->HasParent(Focus, GetVPanel()))
		RequestFocus();

	BaseClass::OnMousePressed(Code);
}

//=========================================================
//=========================================================
void INSFrame::OnKeyCodePressed(KeyCode Code)
{
	if (Code == KEY_ESCAPE/* && surface()->SupportsFeature(ISurface::ESCAPE_KEY)*/)
	{
		if (m_bQuickClose)
			OnClose();
	}
}

//=========================================================
//=========================================================
void INSFrame::OnKeyCodeTyped(KeyCode Code)
{
}

//=========================================================
//=========================================================
void INSFrame::OnKeyTyped(wchar_t UniChar)
{
}

//=========================================================
//=========================================================
void INSFrame::OnKeyCodeReleased(KeyCode code)
{
}

//=========================================================
//=========================================================
void INSFrame::OnKeyFocusTicked()
{
}

//=========================================================
//=========================================================
void INSFrame::LoadControlSettings(const char* dialogResourceName, const char* pathID)
{
	BaseClass::LoadControlSettings(dialogResourceName, pathID);

	// set the focus on the default control
	Panel* defaultFocus = GetFocusNavGroup().GetDefaultPanel();

	if (defaultFocus)
		defaultFocus->RequestFocus();
}

//=========================================================
//=========================================================
void INSFrame::ApplySettings(KeyValues* inResourceData)
{
	// don't change the frame's visibility, remove that setting from the config data
	inResourceData->SetInt("visible", -1);

	// apply settings
	BaseClass::ApplySettings(inResourceData);

	// ensure the client area is up-to-date
	//RecalculateAreas();
}

//=========================================================
//=========================================================
const char* INSFrame::GetDescription(void)
{
	static char buf[1024];
	_snprintf(buf, sizeof(buf), "%s", BaseClass::GetDescription());
	return buf;
}

//=========================================================
//=========================================================
bool INSFrame::GetDefaultScreenPosition(int& x, int& y, int& wide, int& tall)
{
	return false;
}

//=========================================================
//=========================================================
void INSFrame::PerformLayout()
{
	// redo areas
	RecalculateAreas();

	// also recaculate buttons
	int iButtonCount = m_Buttons.Count();

	// move the buttons to the bottom-right corner
	if (iButtonCount != 0)
	{
		int iWide, iTall;

		m_pClientBackground->GetSize(iWide, iTall);
		m_pClientArea->SetSize(iWide, iTall - 32);

		int iXPos = iWide - 80;
		int iYPos = iTall - 28;

		bool bFirstButton = true;

		for (int i = 0; i < iButtonCount; i++)
		{
			Button* pButton = m_Buttons[i];

			if (pButton->IsVisible())
			{
				int iContentWide, iContentTall;
				pButton->GetContentSize(iContentWide, iContentTall);

				int iButtonWide = max(iContentWide + 5, 72);
				iXPos -= bFirstButton ? 8 : (iButtonWide + 8);
				pButton->SetBounds(iXPos, iYPos, iButtonWide, 24);

				bFirstButton = false;
			}
		}
	}
}

//=========================================================
//=========================================================
void INSFrame::OnClose()
{
	// if we're modal, release that before we hide the window else the wrong window will get focus
	if (input()->GetAppModalSurface() == GetVPanel())
		input()->ReleaseAppModalSurface();

	BaseClass::OnClose();
	SetVisible(false);
}

//=========================================================
//=========================================================
// NOTE: is this redundant?
void INSFrame::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);

	if (IsProportional())
		return;

	// make sure we're completely on screen
	int iNewWide, iNewTall;
	surface()->GetScreenSize(iNewWide, iNewTall);

	int x, y, wide, tall;
	GetBounds(x, y, wide, tall);

	// make sure the bottom-right corner is on the screen first
	if (x + wide > iNewWide)
	{
		x = iNewWide - wide;
	}
	if (y + tall > iNewTall)
	{
		y = iNewTall - tall;
	}

	// make sure the top-left is visible
	x = max(0, x);
	y = max(0, y);

	// apply
	SetPos(x, y);
}

//=========================================================
//=========================================================
Panel* INSFrame::GetClientArea(void) const
{
	return m_pClientArea;
}

//=========================================================
//=========================================================
void INSFrame::MoveToCenterOfScreen()
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}

//=========================================================
//=========================================================
void INSFrame::RecalculateAreas(void)
{
	int iHeaderCount = m_Headers.Count();

	// setup the size
	for (int i = 0; i < iHeaderCount; i++)
		m_Headers[i]->SetupSize();

	int iYOffset = 0;

	// the offset is equal to the size of the headers
	// and the gaps between them
	if (iHeaderCount != 0)
	{
		for (int i = 0; i < iHeaderCount; i++)
		{
			INSFrameHeader* pPanel = m_Headers[i];

			// set the new position
			pPanel->SetPos(0, iYOffset);

			// increase the offset by the panel tall
			int iPanelWide, iPanelTall;
			pPanel->GetSize(iPanelWide, iPanelTall);
			iYOffset += (iPanelTall + m_iTitleGap - pPanel->GetOffset());
		}
	}

	// use offset on the parent window size
	int iXPos, iYPos;
	GetPos(iXPos, iYPos);

	int iWide, iTall;
	GetSize(iWide, iTall);

	int iClientBackgroundYPos = iYOffset;
	int iClientBackgroundTall = iTall - iYOffset;

	m_pClientBackground->SetBounds(0, iClientBackgroundYPos, iWide, iClientBackgroundTall);
	m_pClientArea->SetBounds(0, 0, iWide, iClientBackgroundTall);
}