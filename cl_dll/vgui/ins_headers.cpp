//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "vgui/ivgui.h"
#include "ins_headers.h"
#include "c_playerresource.h"
#include "hud_stringfont.h"
#include "basic_colors.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
class CINSVersion : public Panel
{
	DECLARE_CLASS_SIMPLE(CINSVersion, Panel);

public:
	CINSVersion(Panel *pParent)
		: Panel(pParent, "INSVersion")
	{
		SetPaintBackgroundEnabled(false);
	}

protected:
	virtual void Paint(void)
	{
		g_pHudStringDisplay->DrawString(0, 0,
			INS_SERVER_STRING,
			COLOR_WHITE, 1.0f, STRINGFONT_VERSION);
	}
};

//=========================================================
//=========================================================
CINSHeader::CINSHeader(INSFrame *pParent, const char *pszName)
	: INSFrameHeader(pParent, pszName)
{
	SetPaintBackgroundEnabled(false);

	m_pINSVersion = new CINSVersion(this);

	LoadControlSettings("Resource/UI/Frames/Headers/INS.res");
}

//=========================================================
//=========================================================
CINSHeader::~CINSHeader()
{
	delete m_pINSVersion;
}

//=========================================================
//=========================================================
CServerHeader::CServerHeader(INSFrame *pParent, const char *pszName)
	: INSTextHeader(pParent, pszName)
{
	ivgui()->AddTickSignal(GetVPanel(), 250);

	Reset();
}

//=========================================================
//=========================================================
void CServerHeader::Reset(void)
{
	BaseClass::Reset();

	m_iCurrentPlayers = 0;
}

//=========================================================
//=========================================================
void CServerHeader::PerformLayout(void)
{
	Update();
}

//=========================================================
//=========================================================
void CServerHeader::OnTick(void)
{
	if(GetParent()->IsVisible() && ShouldUpdate())
		Update();
}

//=========================================================
//=========================================================
void CServerHeader::OnVisibilityChange(int iOldVisible)
{
	BaseClass::OnVisibilityChange(iOldVisible);

	Update();
}

//=========================================================
//=========================================================
extern const char *g_pszHostname;

void CServerHeader::Update(void)
{
	int iCurrentPlayers = 0;

	// NOTE: there *must* be a better way of doing this
	for(int i = 1; i <= MAX_PLAYERS; i++)
	{
		if(g_PR->IsConnected(i))
			iCurrentPlayers++;
	}

	char szText[MAX_TEXTHEADER_LENGTH];
	int iMaxClients = gpGlobals->maxClients;

	Q_snprintf(szText, sizeof(szText), "%s -- %i/%i", g_pszHostname, m_iCurrentPlayers, iMaxClients);

	SetText(szText);
}

//=========================================================
//=========================================================
bool CServerHeader::ShouldUpdate(void)
{
	if(!g_PR)
		return false;

	int iCurrentPlayers = 0;

	// NOTE: there *must* be a better way of doing this
	for(int i = 1; i <= MAX_PLAYERS; i++)
	{
		if(g_PR->IsConnected(i))
			iCurrentPlayers++;
	}
	
	if(m_iCurrentPlayers != iCurrentPlayers)
	{
		m_iCurrentPlayers = iCurrentPlayers;
		return true;
	}

	return false;
}
