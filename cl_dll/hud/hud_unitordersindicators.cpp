//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "iclientmode.h"
#include "view_scene.h"

#include <vgui/ischeme.h>
#include <vgui/isurface.h>
#include <vgui/isystem.h>
#include <vgui_controls/panel.h>

#include "hud_macros.h"
#include "ins_gamerules.h"
#include "ins_player_shared.h"
#include "c_ins_obj.h"
#include "vgui/ilocalize.h"
#include "hud_stringfont.h"

#include "ins_squad_shared.h"

#include "basic_colors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define UOINDI_SIZE 48
#define UOINDI_MAXLIMIT 10
#define UOINDI_MINLIMIT 125
#define UOINDI_MINSCALE 0.01f
#define UOINDI_MAXDISTANCE 95

//=========================================================
//=========================================================
class CUnitOrdersIndicator : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE(CUnitOrdersIndicator, Panel);

public:
	CUnitOrdersIndicator(const char *pElementName);

private:
	void Init(void);
	void ApplySchemeSettings(IScheme *pScheme);

	void Paint(void);

private:
	int m_iIndiSize;

	int m_iOrderTypeTexID[ORDERTYPE_UNIT_COUNT];
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT(CUnitOrdersIndicator);

CUnitOrdersIndicator::CUnitOrdersIndicator(const char *pElementName) 
	: CHudElement(pElementName), vgui::Panel(NULL, "UnitOrdersIndicator") 
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetPaintBackgroundEnabled(false);
	SetProportional(true);
	SetPos(0, 0);
	SetSize(640, 480);

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	SetScheme(scheme);
}

//=========================================================
//=========================================================
void CUnitOrdersIndicator::Init(void)
{
	char szIndicatorPath[128];

	for(int i = 0; i < ORDERTYPE_UNIT_COUNT; i++)
	{
		if(i == ORDERTYPE_UNIT_NONE)
			continue;

		Q_snprintf(szIndicatorPath, sizeof(szIndicatorPath), "sprites/hud/unitordersindi/%s", g_pszUnitTypeObjNames[i]);

		m_iOrderTypeTexID[i] = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_iOrderTypeTexID[i], szIndicatorPath, false, false);
	}
}

//=========================================================
//=========================================================
void CUnitOrdersIndicator::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_iIndiSize = scheme()->GetProportionalScaledValue(UOINDI_SIZE);
}

//=========================================================
//=========================================================
extern bool CaculateIndicatorScaleAlpha(float &flScale, int &iSize, int &iDistance, int &iAlpha, int &iXPos, int &iYPos, 
								 const Vector &vecOrigin,
								 float flMaxLimit, float flMinLimit, float flMinScale);

void CUnitOrdersIndicator::Paint(void)
{
	if(!g_pGameRules)
		return;

	C_INSPlayer *pLocalPlayer = C_INSPlayer::GetLocalPlayer();

	if(!pLocalPlayer || pLocalPlayer->IsPlayerDead() || pLocalPlayer->GetFlags() & FL_IRONSIGHTS)
		return;

	C_INSSquad *pSquad = pLocalPlayer->GetSquad();

	if(!pSquad)
		return;

	CUnitOrders *pUnitOrders = &pSquad->GetUnitOrders();

	int iOrderTexID = m_iOrderTypeTexID[pUnitOrders->m_iOrderType];

	float flScale;
	int iSize, iDistance, iAlpha, iXPos, iYPos;
	iSize = m_iIndiSize;

	if(!CaculateIndicatorScaleAlpha(flScale, iSize, iDistance, iAlpha, iXPos, iYPos,
		pUnitOrders->m_vecPosition, UOINDI_MAXLIMIT, UOINDI_MINLIMIT, UOINDI_MINSCALE))
		return;

	if(iDistance > UOINDI_MAXDISTANCE)
		return;

	surface()->DrawSetTexture(iOrderTexID);
	surface()->DrawSetColor(Color(255, 255, 255, iAlpha));
	surface()->DrawTexturedRect(iXPos, iYPos, iXPos + iSize, iYPos + iSize);
}