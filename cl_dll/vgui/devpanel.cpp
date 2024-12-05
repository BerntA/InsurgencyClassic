//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "devpanel.h"
#include "ins_headers.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
CreateBasicINSHeader(DevPanel);

CDevPanel::CDevPanel(IViewPort *pViewPort)
	: INSFrame(NULL, PANEL_DEVPANEL)
{
}

//=========================================================
//=========================================================
void CDevPanel::ShowPanel(bool bShow)
{
	if(BaseClass::IsVisible() == bShow)
		return;

	if(bShow)
	{
		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false );
		SetMouseInputEnabled(false);
	}
}
