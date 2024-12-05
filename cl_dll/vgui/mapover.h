//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_MAPOVER_H
#define VGUI_MAPOVER_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/editablepanel.h>

#define TABNAME_MAPOVER "Map Overview"

class CMapOverview;

// ------------------------------------------------------------------------------------ //
// Map Overview
// ------------------------------------------------------------------------------------ //
class CMapOver : public vgui::EditablePanel
{
private:
	DECLARE_CLASS_SIMPLE(CMapOver, vgui::EditablePanel);

public:
	CMapOver(vgui::Panel *pParent);

protected:
	MESSAGE_FUNC( OnResetData, "ResetData" );

private:	
	CMapOverview *m_pMapOverview;
};

#endif // VGUI_MAPOVER_H
