//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "mapover.h"
#include "mapoverview.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
CMapOver::CMapOver(Panel *pParent)
	: EditablePanel(pParent, PANEL_MAPOVER)
{
	SetScheme("ClientScheme");

	m_pMapOverview = new CMapOverview(this);

	LoadControlSettings("Resource/UI/Frames/Panels/MapOverview.res");
}

//=========================================================
//=========================================================
void CMapOver::OnResetData(void)
{
	m_pMapOverview->Reset();
}