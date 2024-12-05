//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "c_playerresource.h"
#include "vgui_entitypanel.h"
#include "iclientmode.h"
#include "vgui/ilocalize.h"
#include "ins_gamerules.h"
#include "c_ins_obj.h"
#include "ins_player_shared.h"
#include "ins_squad_shared.h"
#include "basic_colors.h"

#include <vgui_controls/panel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
enum OrderBarSegments_t
{
	ORDERBAR_LEFT = 0,
	ORDERBAR_BG,
	ORDERBAR_RIGHT,
	ORDERBAR_COUNT
};

enum OrderBarTexSegments_t
{
	ORDERBARTEX_LEFT = 0,
	ORDERBARTEX_BG,
	ORDERBARTEX_RIGHT,
	ORDERBARTEX_RIGHT_TIMER,
	ORDERBARTEX_COUNT
};

struct OrderBarTex_t
{
	int m_iTexID;
	int m_iXPos, m_iWidth;

	int m_iTexWide, m_iTexTall;
	TexCoord_t m_TexCoords;
};

#define MAX_ORDER_STRING_LENGTH 256

//=========================================================
//=========================================================
class COrderBar : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(COrderBar, Panel);

public:
	COrderBar(const char *pElementName);

	void Init(void);
	void LevelInit(void);

private:
	void ApplySchemeSettings(vgui::IScheme *pScheme);

	bool ShouldDraw(void);
	void Paint(void);

	void FireGameEvent(IGameEvent *pEvent);

	void Reposition(void);

	void UpdateOrders(C_INSSquad *pSquad, bool bUpdated);
	void UpdateOrders(const CObjOrders &UpdateOrders);

	OrderBarTex_t &GetOrderBarTex(int iSegmentID);

private:
	const CObjOrders *m_pLastOrders;

	OrderBarTex_t m_OrderBarTex[ORDERBARTEX_COUNT];

	wchar_t m_wszOrderString[MAX_ORDER_STRING_LENGTH];
	int m_iOrderStringLength;
	vgui::HFont	m_hOrderFont;
	int m_iTextXPos, m_iTextYPos;

	vgui::HFont	m_hTimerFont;
	int m_iTimerXPos, m_iTimerYPos, m_iTimerXPosGap;
	float m_flFlashTime;
	bool m_bDraw;

	float m_flEndTime;
};

//=========================================================
//=========================================================
const char *g_pszOrderBarSegmentNames[ORDERBARTEX_COUNT] = {
	"orderbar1",	// ORDERBARTEX_LEFT
	"orderbar2",	// ORDERBARTEX_BG
	"orderbar3",	// ORDERBARTEX_RIGHT
	"orderbar4"		// ORDERBARTEX_RIGHT_TIMER
};

int g_iOrderBarSegmentWide[ORDERBARTEX_COUNT] = {
	60,				// ORDERBARTEX_LEFT
	512,			// ORDERBARTEX_BG
	18,				// ORDERBARTEX_RIGHT
	98				// ORDERBARTEX_RIGHT_TIMER
};

#define ORDERBAR_NORDERS "Unassigned Orders"
#define ORDERBAR_TEXT_YPOS 1
#define ORDERBAR_BGTEXT_PADDING 0.10f
#define ORDERBAR_HEIGHT 64
#define ORDERBAR_SCALE 2.75f

#define ORDERBAR_TIMERTEXT_YPOS		5
#define ORDERBAR_TIMERTEXT_XPOSGAP	5
#define ORDERBAR_TIMER_FLASH		0.5f
#define ORDERBAR_TIMER_RED			5

//=========================================================
//=========================================================
DECLARE_HUDELEMENT(COrderBar);

//=========================================================
//=========================================================
COrderBar::COrderBar(const char *pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "OrderBar")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetPaintBackgroundEnabled(false);

	SetHiddenBits(HIDEHUD_SPECTATOR);

	gameeventmanager->AddListener(this, "squad_order", false);
	gameeventmanager->AddListener(this, "round_reset", false);
	gameeventmanager->AddListener(this, "round_end", false);
}

//=========================================================
//=========================================================
void COrderBar::Init(void)
{
	char szTexPath[256];

	for(int i = 0; i < ORDERBARTEX_COUNT; i++)
	{
		OrderBarTex_t &OrderBarTex = m_OrderBarTex[i];

		Q_snprintf(szTexPath, sizeof(szTexPath), "sprites/hud/orderbar/%s", g_pszOrderBarSegmentNames[i]);

		OrderBarTex.m_iTexID = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(OrderBarTex.m_iTexID, szTexPath, false, false);
	}
}

//=========================================================
//=========================================================
void COrderBar::LevelInit(void)
{
	m_pLastOrders = NULL;

	m_flEndTime = 0.0f;

	m_flFlashTime = 0.0f;
	m_bDraw = false;
}

//=========================================================
//=========================================================
void COrderBar::Reposition(void)
{
	int iXPos = 0;

	for(int i = 0; i < ORDERBAR_COUNT; i++)
	{
		OrderBarTex_t &OrderBarTex = GetOrderBarTex(i);
		OrderBarTex.m_iXPos = iXPos;

		iXPos += OrderBarTex.m_iWidth;
	}

	SetSize(iXPos, GetTall());

	int iScreenWide, iScreenHeight;
	engine->GetScreenSize(iScreenWide, iScreenHeight);

	SetPos((iScreenWide / 2) - (iXPos / 2), 0);
}

//=========================================================
//=========================================================
void COrderBar::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// work out height
	SetSize(GetWide(), scheme()->GetProportionalScaledValue(RoundFloatToInt(ORDERBAR_HEIGHT / ORDERBAR_SCALE)));

	// setup widths
	for(int i = 0; i < ORDERBARTEX_COUNT; i++)
	{
		OrderBarTex_t &OrderBarTex = m_OrderBarTex[i];

		vgui::surface()->DrawGetTextureSize(OrderBarTex.m_iTexID, OrderBarTex.m_iTexWide, OrderBarTex.m_iTexTall);
		OrderBarTex.m_iWidth = RoundFloatToInt(scheme()->GetProportionalScaledValue(g_iOrderBarSegmentWide[i]) / ORDERBAR_SCALE);

		CreateTexCoord(OrderBarTex.m_TexCoords, 0, 0,
			g_iOrderBarSegmentWide[i], OrderBarTex.m_iTexTall,
			OrderBarTex.m_iTexWide, OrderBarTex.m_iTexTall);
	}

	// ... fonts
	m_hOrderFont = pScheme->GetFont("OrderBar", true);

	m_hTimerFont = pScheme->GetFont("CaptureFont");

	// ... and text pos's
	m_iTextXPos = 0;
	m_iTextYPos = scheme()->GetProportionalScaledValue(ORDERBAR_TEXT_YPOS);

	m_iTimerXPos = 0;
	m_iTimerYPos = scheme()->GetProportionalScaledValue(ORDERBAR_TIMERTEXT_YPOS);
	m_iTimerXPosGap = scheme()->GetProportionalScaledValue(ORDERBAR_TIMERTEXT_XPOSGAP);
}

//=========================================================
//=========================================================
#define ORDERBAR_ENDTIME 0.5f

void COrderBar::FireGameEvent(IGameEvent *pEvent)
{
	const char *pszType = pEvent->GetName();

	if(Q_strcmp(pszType, "squad_order") == 0)
	{
		EHANDLE Squad = pEvent->GetInt("squad");
		C_BaseEntity *pSquad = (C_BaseEntity*)Squad;

		if(pSquad)
			UpdateOrders((C_INSSquad*)pSquad, pEvent->GetBool("update"));
	}
	else if(Q_strcmp(pszType, "round_end") == 0)
	{
		m_flEndTime = gpGlobals->curtime + ORDERBAR_ENDTIME;
	}
	else /* Q_strcmp(pszType, "round_reset") == 0) */
	{
		m_flEndTime = 0.0f;
	}
}

//=========================================================
//=========================================================
void COrderBar::UpdateOrders(C_INSSquad *pSquad, bool bUpdated)
{
	if(!pSquad)
		return;

	// invalidate layout when needed
	if(!m_hOrderFont)
		InvalidateLayout(true, true);

	// update orders
	const CObjOrders &Orders = pSquad->GetObjOrders();

	if((!bUpdated && m_pLastOrders != &Orders) || bUpdated || !m_pLastOrders)
		UpdateOrders(Orders);
}

//=========================================================
//=========================================================
// NOTE: One day add code to change the color of the "current ojective" so like ..
//       "Attack Obj. Alpha" (but its slightly green, to match its IMC value)
void COrderBar::UpdateOrders(const CObjOrders &UpdateOrders)
{
	if(!m_hOrderFont)
		return;

	// put together string
	char szOrderString[MAX_ORDER_STRING_LENGTH];

	if(!UpdateOrders.HasOrders())
	{	
		Q_strcpy(szOrderString, ORDERBAR_NORDERS);
	}
	else
	{
		const char *pszOrderType = g_pszOrderTypeObjNames[UpdateOrders.m_iOrderType];
		const char *pszObjPhoneticName = UpdateOrders.m_pObjective->GetPhonetischName();

		Q_snprintf(szOrderString, sizeof(szOrderString), "%s Obj. %s", pszOrderType, pszObjPhoneticName);
	}

	vgui::localize()->ConvertANSIToUnicode(szOrderString, m_wszOrderString, sizeof(m_wszOrderString));
	m_iOrderStringLength = Q_strlen(szOrderString);

	// work out length of string to position text
	int iTotalStringWidth = UTIL_ComputeStringWidth(m_hOrderFont, szOrderString);
	int iPadding = iTotalStringWidth * ORDERBAR_BGTEXT_PADDING;

	m_iTextXPos = iPadding + m_OrderBarTex[ORDERBAR_LEFT].m_iWidth;
	iTotalStringWidth += iPadding * 2;

	if(iTotalStringWidth == 0)
	{
		AssertMsg(false, "Managed to Somehow obtain a Orderbar Width of 0");
		iTotalStringWidth = 1;
	}

	// reposition and resize the background
	OrderBarTex_t &OrderBarTex = m_OrderBarTex[ORDERBARTEX_BG];
	OrderBarTex.m_iWidth = iTotalStringWidth;

    CreateTexCoord(OrderBarTex.m_TexCoords,
		0, 0,
		g_iOrderBarSegmentWide[ORDERBARTEX_BG] * ((float)iTotalStringWidth / g_iOrderBarSegmentWide[ORDERBARTEX_BG]), ORDERBAR_HEIGHT,
		OrderBarTex.m_iTexWide, OrderBarTex.m_iTexTall);

	// now reposition the parent
	Reposition();

	// reposition tex
	m_iTimerXPos = OrderBarTex.m_iXPos + OrderBarTex.m_iWidth + m_iTimerXPosGap;

	// remember the last orders
	m_pLastOrders = &UpdateOrders;
}

//=========================================================
//=========================================================
bool COrderBar::ShouldDraw(void)
{
	if(!CHudElement::ShouldDraw())
		return false;

	return (m_pLastOrders != NULL);
}

//=========================================================
//=========================================================
void COrderBar::Paint(void)
{
	C_INSPlayer *pLocalPlayer = C_INSPlayer::GetLocalPlayer();

	if(!pLocalPlayer || !pLocalPlayer->OnPlayTeam() || !pLocalPlayer->IsValidSquad())
		return;

	int iYOffset = 0;
	int iTall = GetTall();

	if(m_flEndTime != 0.0f)
		iYOffset = clamp(RoundFloatToInt(iTall * (1.0f - ((m_flEndTime - gpGlobals->curtime) / ORDERBAR_ENDTIME))), 0.0f, iTall);

	for(int i = 0; i < ORDERBAR_COUNT; i++)
	{
		OrderBarTex_t &OrderBarTex = GetOrderBarTex(i);

		vgui::surface()->DrawSetTexture(OrderBarTex.m_iTexID);
		vgui::surface()->DrawSetColor(Color(255, 255, 255, 255));
		vgui::surface()->DrawTexturedSubRect(OrderBarTex.m_iXPos, -iYOffset, OrderBarTex.m_iXPos + OrderBarTex.m_iWidth, iTall - iYOffset,
			OrderBarTex.m_TexCoords[0], OrderBarTex.m_TexCoords[1], OrderBarTex.m_TexCoords[2], OrderBarTex.m_TexCoords[3]);
	}

	surface()->DrawSetTextColor(Color(200, 200, 200, 255));
	surface()->DrawSetTextFont(m_hOrderFont);
	surface()->DrawSetTextPos(m_iTextXPos, m_iTextYPos - iYOffset);
	surface()->DrawPrintText(m_wszOrderString, m_iOrderStringLength);

	C_INSRules *pINSRules = INSRules();

	if(!pINSRules->IsModeRunning())
		return;

	C_RunningMode *pRunningMode = pINSRules->RunningMode();

	if(!pRunningMode || pRunningMode->GetRoundLength() == ROUNDTIMER_INVALID)
		return;

	Color TimerColor = COLOR_WHITE;

	char szRoundTime[128];

	if(pRunningMode->IsRoundExtended())
	{
		TimerColor = COLOR_RED;

		Q_strncpy(szRoundTime, "00:00", sizeof(szRoundTime));

		// TODO: put this in think
		if(m_flFlashTime >= gpGlobals->curtime)
		{
			if(!m_bDraw)
				return;
		}
		else
		{
			m_flFlashTime = gpGlobals->curtime + 0.35f;
			m_bDraw = !m_bDraw;
		}
	}
	else
	{
		int iRoundTime = pRunningMode->GetRoundLength() - int(gpGlobals->curtime - pRunningMode->GetRoundStartTime());

		if(iRoundTime <= ORDERBAR_TIMER_RED)
			TimerColor = COLOR_RED;

		Q_snprintf(szRoundTime, sizeof(szRoundTime), "%02d:%02d", (iRoundTime / 60), (iRoundTime % 60));
	}

	wchar_t szText[128];
	localize()->ConvertANSIToUnicode(szRoundTime, szText, sizeof(szText));

	surface()->DrawSetTextColor(TimerColor);
	surface()->DrawSetTextFont(m_hTimerFont);
	surface()->DrawSetTextPos(m_iTimerXPos, m_iTimerYPos - iYOffset);
	surface()->DrawPrintText(szText, Q_strlen(szRoundTime), FONT_DRAW_NONADDITIVE);
}

//=========================================================
//=========================================================
OrderBarTex_t &COrderBar::GetOrderBarTex(int iSegmentID)
{
	int iTexSegmentID = iSegmentID;
	
	if(iSegmentID == ORDERBAR_RIGHT)
	{
		C_INSRules *pINSRules = INSRules();

		if(pINSRules->IsModeRunning())
		{
			C_RunningMode *pRunningMode = pINSRules->RunningMode();

			if(pRunningMode && pRunningMode->GetRoundLength() != -1)
				iTexSegmentID = ORDERBARTEX_RIGHT_TIMER;
		}
	}

	return m_OrderBarTex[iTexSegmentID];
}