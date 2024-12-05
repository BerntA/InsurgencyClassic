//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "hud_boxed.h"
#include "basic_colors.h"

#include <vgui_controls/editablepanel.h>

#include <vgui/ischeme.h>
#include <vgui/isurface.h>
#include <vgui/isystem.h>
#include "vgui/ilocalize.h"

#include "ins_squad_shared.h"
#include "c_ins_obj.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define BG_START_X			3
#define BG_START_Y			2

#define OBJ_GAP				2
#define OBJ_LENGTH			4
#define MAX_OBJDISPLAY		5
#define MAX_OBJSAND			2
#define OBJ_ALPHA			192

enum {
	SIZE_NSELECTED = 0,
	SIZE_SELECTED,
	SIZE_TYPES
};

//=========================================================
//=========================================================
class CHudObjOverview : public CHUDBoxed
{
	DECLARE_CLASS_SIMPLE(CHudObjOverview, CHUDBoxed);

public:
	CHudObjOverview(const char *pszName);

private:
	void ApplySchemeSettings(vgui::IScheme *pScheme);

	void LevelInit(void);
	void PerformLayout(void);
	void CalculateSizes(void);

	void FireGameEvent(IGameEvent *pEvent);

	void UpdateOrders(C_INSSquad *pSquad, bool bUpdated);

	bool ShouldDraw(void);
	void Paint(void);

	int FindPosition(int iObjID);
	void CreateObjString(C_INSObjective *pObj, int iSizeType, wchar_t *pszBuffer);

private:
	HFont m_hFont;

	int m_iBGStartX, m_iBGStartY;

	int m_iCurrentObj;

	int m_iSizes[MAX_OBJECTIVES+1][SIZE_TYPES];
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT(CHudObjOverview);

//=========================================================
//=========================================================
CHudObjOverview::CHudObjOverview(const char *pszName) :
	CHUDBoxed(pszName, "HudObjOverview")
{
	gameeventmanager->AddListener(this, "squad_order", false);

	SetEdgeBit(BOXEDEDGES_BOTTOMRIGHT);
}

//=========================================================
//=========================================================
void CHudObjOverview::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont("DefaultBig", true);

	m_iBGStartX = scheme()->GetProportionalScaledValue(BG_START_X);
	m_iBGStartY = scheme()->GetProportionalScaledValue(BG_START_Y);
}

//=========================================================
//=========================================================
void CHudObjOverview::LevelInit(void)
{
	InvalidateLayout(false, true);
}

//=========================================================
//=========================================================
void CHudObjOverview::PerformLayout(void)
{
	CalculateSizes();
}

//=========================================================
//=========================================================
void CHudObjOverview::CalculateSizes(void)
{
	memset(&m_iSizes, 0, sizeof(m_iSizes));

	int iGap = scheme()->GetProportionalScaledValue(OBJ_GAP);
	wchar_t szObj[OBJ_LENGTH];

	const CUtlVector< C_INSObjective* > &Objectives = C_INSObjective::GetObjectiveList( );

	for(int i = 0; i < Objectives.Count(); i++)
	{
		C_INSObjective *pObjective = Objectives[i];

		if(!pObjective)
			return;

		for(int j = 0; j < SIZE_TYPES; j++)
		{
			CreateObjString(pObjective, j, szObj);

			m_iSizes[i][j] = UTIL_ComputeStringWidth(m_hFont, szObj) + iGap;
		}
	}
}

//=========================================================
//=========================================================
void CHudObjOverview::FireGameEvent(IGameEvent *pEvent)
{
	EHANDLE Squad = pEvent->GetInt("squad");
	C_BaseEntity *pSquad = (C_BaseEntity*)Squad;

	if(pSquad)
		UpdateOrders((C_INSSquad*)pSquad, pEvent->GetBool("update"));
}

//=========================================================
//=========================================================
void CHudObjOverview::UpdateOrders(C_INSSquad *pSquad, bool bUpdated)
{
	const CObjOrders &Orders = pSquad->GetObjOrders();

	if(Orders.HasOrders())
		m_iCurrentObj = Orders.m_pObjective->GetOrderID();
	else
		m_iCurrentObj = INVALID_OBJECTIVE;		

	if(!m_hFont)
		InvalidateLayout(true);
}

//=========================================================
//=========================================================
bool CHudObjOverview::ShouldDraw(void)
{
	if(!CHudElement::ShouldDraw())
		return false;

	if(!g_bInStatus)
		return false;

	if(m_iCurrentObj == INVALID_OBJECTIVE)
		return false;

	/*C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();

	if(!pPlayer)
		return false;

	C_PlayTeam *pTeam = pPlayer->GetPlayTeam();

	// make sure they're on a playteam
	if(!pTeam)
		return false;

	// make sure the game isn't restarting
	if(INSRules()->IsStatusRunning() && INSRules()->GetRunningMode()->GetStatus(GAMERUNNING_RESTARTING))
		return false;

	// make sure there are enough reinforcements
	if(pPlayer->IsAlive() && !pPlayer->IsObserver() && pTeam->GetReinforcementsLeft() > RoundFloatToInt((pTeam->GetMaxWaves() * SHOW_DEPLOYMENTS)))
		return false;*/

	return true;
}

//=========================================================
//=========================================================
void CHudObjOverview::Paint(void)
{
	surface()->DrawSetTextFont(m_hFont);

	int iStartObj = m_iCurrentObj - MAX_OBJSAND;

	const CUtlVector< C_INSObjective* > &Objectives = C_INSObjective::GetObjectiveList( );

	if(Objectives.Count() >= MAX_OBJDISPLAY && (iStartObj + MAX_OBJDISPLAY) >= Objectives.Count())
		iStartObj = Objectives.Count() - MAX_OBJDISPLAY;
	else if(iStartObj < 0)
		iStartObj = 0;

	wchar_t szObj[OBJ_LENGTH];

	int iXPos = 0;

	for(int i = 0; i < MAX_OBJDISPLAY; i++)
	{
		int iObjID = iStartObj + i + 1;

		C_INSObjective *pObjective = Objectives[iObjID];

		if(!pObjective)
			break;

		int iSizeType = ((iObjID == m_iCurrentObj) ? SIZE_SELECTED : SIZE_NSELECTED);
		CreateObjString(pObjective, iSizeType, szObj);

		Color ObjColor;

		if(pObjective->GetCapturedTeam() == TEAM_NEUTRAL)
			ObjColor = COLOR_GREY;
		else
			ObjColor = INSRules()->TeamColor(pObjective->GetCapturedTeam());

		ObjColor[3] = OBJ_ALPHA;

		surface()->DrawSetTextColor(ObjColor);
		surface()->DrawSetTextPos(m_iBGStartX + iXPos, m_iBGStartY);
		surface()->DrawPrintText(szObj, OBJ_LENGTH - 1);

		iXPos += m_iSizes[iObjID][iSizeType];
	}
}

//=========================================================
//=========================================================
void CHudObjOverview::CreateObjString(C_INSObjective *pObj, int iSizeType, wchar_t *pszBuffer)
{
	if(iSizeType == SIZE_SELECTED)
		_snwprintf(pszBuffer, sizeof(wchar_t) * OBJ_LENGTH, L"[%c]", pObj->GetPhonetischLetter());
	else
		_snwprintf(pszBuffer, sizeof(wchar_t) * OBJ_LENGTH, L"(%c)", pObj->GetPhonetischLetter());
}