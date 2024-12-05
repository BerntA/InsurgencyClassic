//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_MAPINFO_H
#define VGUI_MAPINFO_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/editablepanel.h>

namespace vgui
{
	class RichText;
	class ListBox;
	class ImageCycler;
}

#define TABNAME_MAPINFO "Map Information"

// ------------------------------------------------------------------------------------ //
// Mission Overview
// ------------------------------------------------------------------------------------ //
class CMapInfo : public vgui::EditablePanel
{
private:
	DECLARE_CLASS_SIMPLE(CMapInfo, vgui::EditablePanel);

public:
	CMapInfo(vgui::Panel *pParent);

	void Reset(void);

protected:
	virtual void OnVisibilityChange(int iOldVisible);

private:	
	vgui::ListBox *m_pOverview;
	vgui::RichText *m_pDescription;
	vgui::ImageCycler *m_pPreviewImages;

	bool m_bSetupPanel;
};

#endif // VGUI_MAPINFO_H
