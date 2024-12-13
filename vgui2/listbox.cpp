//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <vgui/IScheme.h>
#include <vgui_controls/listbox.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

void ListBox::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	_nameColor = GetSchemeColor("ListBox.NameColor", GetFgColor(), pScheme);
	_valueColor = GetSchemeColor("ListBox.ValueColor", GetFgColor(), pScheme);
}

void ListBox::InsertLine(const char* pszName, const char* pszValue, bool bIndent)
{
	if (bIndent)
		InsertString("   ");

	char szNameLine[32];
	sprintf(szNameLine, "* %s: ", pszName);
	InsertColorChange(_nameColor);
	InsertString(szNameLine);

	InsertColorChange(_valueColor);
	InsertString(pszValue);

	InsertString("\n");
}

void ListBox::InsertLine(void)
{
	InsertString("\n");
}