//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <keyvalues.h>
#include <filesystem.h>
#include <cl_dll/iviewport.h>
#include "htmlwindow.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
CHTMLWindow::CHTMLWindow(Panel *pParent, const char *pszPanelName)
	: Panel(pParent, pszPanelName)
{
	SetScheme("ClientScheme");

	m_pMessage = new HTML(this, "Message");
	m_pMessage->SetScrollbarsEnabled(true);
	m_pMessage->SetContextMenuEnabled(true);

	m_bSizeParent = false;
}

//=========================================================
//=========================================================
void CHTMLWindow::PerformLayout(void)
{
	BaseClass::PerformLayout();

	int iWide, iTall;

	if(m_bSizeParent)
	{
		SetPos(0, 0);
		GetParent()->GetSize(iWide, iTall);
	}
	else
	{
		GetSize(iWide, iTall);
	}

	m_pMessage->SetPos(0, 0);

	SetSize(iWide, iTall);
	m_pMessage->SetSize(iWide, iTall);
}

//=========================================================
//=========================================================
void CHTMLWindow::LoadFile(const char *pszRelative)
{
	if(!vgui::filesystem()->FileExists(pszRelative, "GAME"))
		return;

	char szFullPath[_MAX_PATH];
	vgui::filesystem()->GetLocalPath(pszRelative, szFullPath, sizeof(szFullPath));

	char szLocalURL[_MAX_PATH + 7];
	Q_strcpy(szLocalURL, "file://");
	Q_strncat(szLocalURL, szFullPath, sizeof(szFullPath), COPY_ALL_CHARACTERS);

	LoadURL(szLocalURL);
}

//=========================================================
//=========================================================
void CHTMLWindow::LoadURL(const char *pszURL)
{
	m_pMessage->OpenURL(pszURL);
}
