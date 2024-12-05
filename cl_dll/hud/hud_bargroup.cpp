//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud_bargroup.h"
#include "hud_macros.h"
#include "ins_player_shared.h"
#include "c_playerresource.h"
#include "vgui_entitypanel.h"
#include "iclientmode.h"
#include "vgui/ilocalize.h"
#include "ins_gamerules.h"
#include "c_ins_obj.h"
#include "gamemovement.h"
#include "basic_colors.h"
#include "hlscolor.h"
#include "weapon_ballistic_base.h"
#include "basic_colors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define HUDBAR_BG_PATH "barbg"
#define HUDBAR_FG_PATH "barfg"
#define HUDBAR_BORDER_PATH "barborder"

#define HUDBAR_START_YPOS 6
#define HUDBAR_XPOS_GAP 1
#define HUDBAR_YPOS_GAP 3

#define HUDBAR_WIDE 92
#define HUDBAR_TALL 10
#define HUDBAR_TEX_WIDE 128
#define HUDBAR_TEX_TALL 16
#define HBICON_SIZE 10
#define HBICON_TEX_SIZE 16

#define HBE_TRANS_TIME 0.5f
#define HBE_HIDE_TIME 0.5f
#define HBE_INVALID_ID -1

//=========================================================
//=========================================================
typedef CHudBarElement *(*fpBarElementCreate)(void);

CUtlVector<fpBarElementCreate> g_ElementHelpers;

class CBarElementHelper
{
public:
	CBarElementHelper(fpBarElementCreate BarElementCreate)
	{
		g_ElementHelpers.AddToTail(BarElementCreate);
	}
};

#define DEFINE_BARELEMENT(name) \
	static CHudBarElement *CreateBarElement__##name##()	{  \
		 return new name(); \
	} \
	static CBarElementHelper g_BarElementHelper__##name##(CreateBarElement__##name##);


//=========================================================
//=========================================================
enum BarElementUpdateType_t
{
	BARUPDATE_VALID = 0,
	BARUPDATE_HIDE,
	BARUPDATE_FORCEHIDE
};

class CHudBarElement
{
	friend CHudBarGroup;

public:
	//=========================================================
	//=========================================================
	typedef CHudBarElement BaseClass;

	CHudBarElement()
	{
		m_iID = HBE_INVALID_ID;
		m_iIconTexID = 0;
		m_flFraction = 0.0f;
		m_Color = GetDefaultColor();
		m_bVisible = false;
		m_flHideTime = 0.0f;
	}

	//=========================================================
	//=========================================================
	virtual void Init(void)
	{
		Reset();
	}

	//=========================================================
	//=========================================================
	virtual bool Setup(void)
	{
		m_bVisible = true;

		if(IsWaitingForHide())
		{
			m_flHideTime = 0.0f;
			return false;
		}

		m_iID = HBE_INVALID_ID;
		m_iTargetID = 0;
		m_flStartTime = gpGlobals->curtime;

		return true;
	}

	virtual void Reset(void) = 0;

	//=========================================================
	//=========================================================
	void StartHide(void)
	{
		m_flHideTime = gpGlobals->curtime + HBE_HIDE_TIME;
	}

	void FinishHide(void)
	{
		m_bVisible = false;
		m_flHideTime = 0.0f;

		Reset();
	}

	//=========================================================
	//=========================================================
	virtual int Update(void) = 0;
	virtual void UpdateFraction(void) = 0;

	//=========================================================
	//=========================================================
	int GetID(void) const { return m_iID; }
	bool IsVisible(void) const { return m_bVisible; }
	bool IsWaitingForHide(void) const { return m_flHideTime != 0.0f; }
	bool IsHidden(void) const { return (IsWaitingForHide() && gpGlobals->curtime >= m_flHideTime); }
	float GetHideTime(void) const { return m_flHideTime; }
	float GetStartTime(void) const { return m_flStartTime; }
	int GetIconTexID(void) const { return m_iIconTexID; }
	virtual const Color &GetColor(void) const { return m_Color; }
	virtual float GetFraction(void) const { return m_flFraction; }
	int GetTargetID(void) const { return m_iTargetID; }
	
	const Color &GetDefaultColor(void)
	{
		static Color DefaultColor = COLOR_WHITE;
		return DefaultColor;
	}

	//=========================================================
	//=========================================================
	void ReachedTargetID(void)
	{
		m_iID = m_iTargetID;
		m_flStartTime = 0.0f;
	}

	void RaiseTargetID(void)
	{
		m_iTargetID++;
		m_flStartTime = gpGlobals->curtime;
	}

	void LowerTargetID(void)
	{
		m_iTargetID--;
		m_flStartTime = gpGlobals->curtime;
	}

protected:
	int m_iIconTexID;

	Color m_Color;
	float m_flFraction;

private:
	int m_iID;
	int m_iTargetID;

	bool m_bVisible;
	float m_flHideTime;

	float m_flStartTime;
};

//=========================================================
//=========================================================
class CSprintElement : public CHudBarElement
{
public:
	virtual void Init(void)
	{
		BaseClass::Init();

		m_iIconTexID = CHudBarGroup::LoadBarImage("icon_sprint");
	}

	virtual int Update(void)
	{
		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();
		Assert(pPlayer);

		return ((pPlayer->GetCurrentStamina() == MAX_STAMINA) ? BARUPDATE_HIDE : BARUPDATE_VALID);
	}

	void Reset(void) { }

	virtual void UpdateFraction(void)
	{
		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();
		Assert(pPlayer);

		float flStamina = (float)pPlayer->GetCurrentStamina();

		m_flFraction = flStamina / MAX_STAMINA;

		if(flStamina >= STAMINA_NEEDED_SPRINT)
		{
			m_Color = GetDefaultColor();
		}
		else
		{
			m_Color = Color(255, 0, 0);
			m_Color[1] = m_Color[2] = 255.0f * (flStamina / STAMINA_NEEDED_SPRINT);
		}
	}
};

DEFINE_BARELEMENT(CSprintElement);

//=========================================================
//=========================================================
#define WAIT_AFTER_RELOAD 2.0f

class CAmmoElement : public CHudBarElement
{
private:
	float m_flCheckClipTime;
	C_WeaponBallisticBase *m_pCurrentWeapon;

public:
	virtual bool Setup(void)
	{
		bool bSuccess = BaseClass::Setup();

		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		Assert(pPlayer);

		m_pCurrentWeapon = (C_WeaponBallisticBase*)pPlayer->GetActiveWeapon();
		m_flCheckClipTime = m_pCurrentWeapon->m_flNextPrimaryAttack + WAIT_AFTER_RELOAD;

		return bSuccess;
	}

	void Reset(void)
	{
		m_pCurrentWeapon = NULL;
		m_flCheckClipTime = 0.0f;
	}

	virtual void Init(void)
	{
		CHudBarElement::Init();

		m_Color = COLOR_WHITE;
		m_iIconTexID = CHudBarGroup::LoadBarImage("icon_ammo");
	}

	virtual int Update(void)
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		Assert(pPlayer);

		C_BaseCombatWeapon *pActiveWeapon = pPlayer->GetActiveWeapon();

		if(!pActiveWeapon)
			return BARUPDATE_FORCEHIDE;

		if(m_pCurrentWeapon)
		{
			if(m_pCurrentWeapon != pActiveWeapon)
				return BARUPDATE_FORCEHIDE;

			if(gpGlobals->curtime >= m_flCheckClipTime && !m_pCurrentWeapon->IsReloading())
				return BARUPDATE_HIDE;

			return BARUPDATE_VALID;
		}

		C_WeaponBallisticBase *pBallisiticWeapon = dynamic_cast<C_WeaponBallisticBase*>(pActiveWeapon);

		if(!pBallisiticWeapon)
			return BARUPDATE_HIDE;

		if(pBallisiticWeapon->IsReloading())
			return BARUPDATE_VALID;

		return BARUPDATE_HIDE;

		//return (pBallisiticWeapon->IsReloading() ? BARUPDATE_VALID : BARUPDATE_HIDE);
	}

	virtual void UpdateFraction(void)
	{
		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();
		Assert(pPlayer);

		if(!m_pCurrentWeapon)
			return;
	
		// the "5" should equal the max clips for each weapon, how do you decide that!?
		m_flFraction = (float)pPlayer->GetClipCount(m_pCurrentWeapon) / MAX_PCLIPS;

		if(m_flFraction != 0.0f)
			m_Color = GetDefaultColor();
		else
			m_Color = COLOR_RED;
	}
};

DEFINE_BARELEMENT(CAmmoElement);

//=========================================================
//=========================================================
DECLARE_HUDELEMENT(CHudBarGroup);

CHudBarGroup::CHudBarGroup(const char *pszElementName)
	: CHudElement(pszElementName), BaseClass(NULL, "HudBarGroup")
{
	// set parent
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	// setup
	SetPaintBackgroundEnabled(false);

	SetHiddenBits(HIDEHUD_SPECTATOR);
}

//=========================================================
//=========================================================
CHudBarGroup::~CHudBarGroup()
{
	m_Elements.PurgeAndDeleteElements();
}

//=========================================================
//=========================================================
void CHudBarGroup::Init(void)
{
	// setup elements
	for(int i = 0; i < g_ElementHelpers.Count(); i++)
		m_Elements.AddToTail(g_ElementHelpers[i]());

	// setup gfx
	m_iElementBGTex = LoadBarImage(HUDBAR_BG_PATH);
	m_iElementFGTex = LoadBarImage(HUDBAR_FG_PATH);
	m_iElementBorderTex = LoadBarImage(HUDBAR_BORDER_PATH);

    CreateTexCoord(ElementBGTexCoord,
		0, 0,
		HUDBAR_WIDE, HUDBAR_TALL,
		HUDBAR_TEX_WIDE, HUDBAR_TEX_TALL);

	CreateTexCoord(ElementIconTexCoord,
		0, 0,
		HBICON_SIZE, HBICON_SIZE,
		HBICON_TEX_SIZE, HBICON_TEX_SIZE);

	for(int i = 0; i < m_Elements.Count(); i++)
		m_Elements[i]->Init();
}

//=========================================================
//=========================================================
void CHudBarGroup::ShowBar(int iID)
{
	if(!m_Elements[iID]->Setup())
		return;

	ModifyElements(RaiseElement, iID);
}

//=========================================================
//=========================================================
void CHudBarGroup::StartHideBar(int iID)
{
	m_Elements[iID]->StartHide();
}

//=========================================================
//=========================================================
void CHudBarGroup::ModifyElements(ModifyElement_t ModifyElement, int iID)
{
	for(int i = 0; i < m_Elements.Count(); i++)
	{
		CHudBarElement *pElement = m_Elements[i];

		if(i == iID || !pElement->IsVisible())
			continue;

		(*this.*ModifyElement)(iID, pElement);
	}
}

//=========================================================
//=========================================================
void CHudBarGroup::RaiseElement(int iID, CHudBarElement *pElement)
{
	pElement->RaiseTargetID();
}

//=========================================================
//=========================================================
void CHudBarGroup::LowerElement(int iID, CHudBarElement *pElement)
{
	CHudBarElement *pTargetElement = m_Elements[iID];

	if(pElement->GetID() <= pTargetElement->GetID())
		return;

	pElement->LowerTargetID();
}

//=========================================================
//=========================================================
void CHudBarGroup::Think(void)
{
	if(!C_BasePlayer::GetLocalPlayer())
		return;

	for(int i = 0; i < m_Elements.Count(); i++)
	{
		CHudBarElement *pElement = m_Elements[i];

		if(pElement->IsHidden())
		{
			ModifyElements(LowerElement, i);
			pElement->FinishHide();
			continue;
		}

		int iUpdateResult = pElement->Update();

		if(pElement->IsVisible())
		{
			if(iUpdateResult != BARUPDATE_VALID && !pElement->IsWaitingForHide())
				StartHideBar(i);

			if(pElement->IsWaitingForHide() && iUpdateResult == BARUPDATE_VALID)
				ShowBar(i);
		}
		else
		{
			if(iUpdateResult == BARUPDATE_VALID)
				ShowBar(i);
		}

		if(iUpdateResult != BARUPDATE_FORCEHIDE)
			pElement->UpdateFraction();
	}
}

//=========================================================
//=========================================================
void CHudBarGroup::Paint(void)
{
	for(int i = 0; i < m_Elements.Count(); i++)
	{
		CHudBarElement *pElement = m_Elements[i];

		if(!pElement->IsVisible())
			continue;

		int iYPos, iAlpha;
		GetElementData(pElement, iYPos, iAlpha);

		DrawElement(m_Elements[i], iYPos, iAlpha);
	}
}

//=========================================================
//=========================================================
void CHudBarGroup::GetElementData(CHudBarElement *pElement, int &iYPos, int &iAlpha)
{
	int iID = pElement->GetID();
	int iTargetID = pElement->GetTargetID();

	int iMyYPos = GetElementYPos(iID);

	bool bWaitingForHide = pElement->IsWaitingForHide();

	// no need for caculations
	if(iID == iTargetID && !bWaitingForHide)
	{
		iAlpha = 255;
		iYPos = iMyYPos;
		return;
	}

	// work out fractions
	float flEndTransTime = gpGlobals->curtime - pElement->GetStartTime();
	float flTransFraction = 0.0f;

	int iTargetYPos = GetElementYPos(iTargetID);

	int iYPosMod = iMyYPos - iTargetYPos;

	if(flEndTransTime >= HBE_TRANS_TIME)
	{
		pElement->ReachedTargetID();
		flTransFraction = 1.0f;
	}
	else if(flEndTransTime != 0.0f)
	{
		flTransFraction = (flEndTransTime / HBE_TRANS_TIME);
	}

	flTransFraction = clamp(flTransFraction, 0.0f, 1.0f);

	// get ypos
	if(iID == iTargetID)
		iYPos = iMyYPos;
	else
		iYPos = iMyYPos - (iYPosMod * flTransFraction);

	// get alpha
	if(iID == HBE_INVALID_ID)
	{
		iAlpha = (int)(255.0f * flTransFraction);
	}
	else if(pElement->IsWaitingForHide())
	{
		iAlpha = (int)(255.0f * ((pElement->GetHideTime() - gpGlobals->curtime) / HBE_HIDE_TIME));
	}
	else
	{
		iAlpha = 255;
	}

	iAlpha = clamp(iAlpha, 0, 255);
}

//=========================================================
//=========================================================
int CHudBarGroup::GetElementYPos(int iID)
{
	int iYPos = GetTall();

	if(iID == HBE_INVALID_ID)
		return iYPos;

	return (iYPos - (m_iStartYPos + ((m_iYPosGap + m_iElementTall) * (iID + 1))));
}

//=========================================================
//=========================================================
void CHudBarGroup::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_iElementWide = scheme()->GetProportionalScaledValue(HUDBAR_WIDE);
	m_iElementTall = scheme()->GetProportionalScaledValue(HUDBAR_TALL);

	m_iStartYPos = scheme()->GetProportionalScaledValue(HUDBAR_START_YPOS);
	m_iXPosGap = scheme()->GetProportionalScaledValue(HUDBAR_XPOS_GAP);
	m_iYPosGap = scheme()->GetProportionalScaledValue(HUDBAR_YPOS_GAP);

	m_iIconSize = scheme()->GetProportionalScaledValue(HBICON_SIZE);
}

//=========================================================
//=========================================================
void CHudBarGroup::DrawElement(CHudBarElement *pElement, int iYPos, int iAlpha)
{
	Color ElementColor = pElement->GetColor();
	ElementColor[3] = iAlpha;

	int iGlobalAlpha = pElement->IsWaitingForHide() ? iAlpha : 255;

	// draw icon
	vgui::surface()->DrawSetTexture(pElement->GetIconTexID());
	vgui::surface()->DrawSetColor(Color(255, 255, 255, iGlobalAlpha));
	vgui::surface()->DrawTexturedSubRect(0, iYPos, m_iIconSize, iYPos + m_iIconSize,
		ElementIconTexCoord[0], ElementIconTexCoord[1], ElementIconTexCoord[2], ElementIconTexCoord[3]);

	// draw bars
	int iBarXPos = m_iIconSize + m_iXPosGap;

	// ... draw background
	vgui::surface()->DrawSetTexture(m_iElementBGTex);
	vgui::surface()->DrawSetColor(ElementColor);
	vgui::surface()->DrawTexturedSubRect(iBarXPos, iYPos, iBarXPos + m_iElementWide, iYPos + m_iElementTall,
		ElementBGTexCoord[0], ElementBGTexCoord[1], ElementBGTexCoord[2], ElementBGTexCoord[3]);

	// ... draw foreground
	TexCoord_t FGTexCoord;

	int iFGWide = RoundFloatToInt(HUDBAR_WIDE * pElement->GetFraction());

    CreateTexCoord(FGTexCoord,
		0, 0,
		iFGWide, HUDBAR_TALL,
		HUDBAR_TEX_WIDE, HUDBAR_TEX_TALL);

	vgui::surface()->DrawSetTexture(m_iElementFGTex);
	vgui::surface()->DrawSetColor(ElementColor);
	vgui::surface()->DrawTexturedSubRect(iBarXPos, iYPos, iBarXPos + scheme()->GetProportionalScaledValue(iFGWide),
		iYPos + m_iElementTall,
		FGTexCoord[0], FGTexCoord[1], FGTexCoord[2], FGTexCoord[3]);

	// ... draw border
	vgui::surface()->DrawSetTexture(m_iElementBorderTex);
	vgui::surface()->DrawSetColor(Color(255, 255, 255, iGlobalAlpha));
	vgui::surface()->DrawTexturedSubRect(iBarXPos, iYPos, iBarXPos + m_iElementWide, iYPos + m_iElementTall,
		ElementBGTexCoord[0], ElementBGTexCoord[1], ElementBGTexCoord[2], ElementBGTexCoord[3]);
}

//=========================================================
//=========================================================
int CHudBarGroup::LoadBarImage(const char *pszSuffix)
{
	char szPath[256];
	Q_snprintf(szPath, sizeof(szPath), "sprites/HUD/bargroup/%s", pszSuffix);

	int iTexID = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(iTexID, szPath, false, false);

	return iTexID;
}