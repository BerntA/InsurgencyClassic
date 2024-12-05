//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef LISTBOX_H
#define LISTBOX_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/richtext.h>

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: Insurgency ListBox
//-----------------------------------------------------------------------------
class ListBox : public RichText
{
	DECLARE_CLASS_SIMPLE( ListBox, RichText );

public:
	ListBox(Panel *parent, const char *panelName)
		: RichText(parent, panelName) { }

	void InsertLine(const char *pszName, const char *pszValue, bool bIndent = false);
	void InsertLine(void);

protected:
	virtual void ApplySchemeSettings(IScheme *pScheme);

private:
	Color _nameColor;
	Color _valueColor;
};

} // namespace vgui


#endif // LISTBOX_H
