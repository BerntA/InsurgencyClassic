//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_MOTD_H
#define VGUI_MOTD_H
#ifdef _WIN32
#pragma once
#endif

#include "htmlwindow.h"

#include <vgui_controls/editablepanel.h>

namespace vgui
{
	class IScheme;
	class ImagePanel;
};

enum MOTDType_t
{
	MOTDTYPE_NONE = 0,
	MOTDTYPE_URL,
	MOTDTYPE_MSG,
	MOTDTYPE_HTML
};

extern const char *g_pszHostname;

// ------------------------------------------------------------------------------------ //
// Message of the Day - Helper Class
// ------------------------------------------------------------------------------------ //
class CMOTDFile
{
public:
	CMOTDFile(const char *pszMOTDMsg);
	CMOTDFile(const char *pszMOTD, int iLength);
	~CMOTDFile();

	static void Load(void);

	int GetType(void) { return m_iType; }
	const char *GetBuffer(void) const { return m_szBuffer; }

private:
	void Setup(void);

private:
	int m_iType;
	char m_szBuffer[MOTD_LENGTH];
};

extern CMOTDFile *g_pMOTDFile;

// ------------------------------------------------------------------------------------ //
// Message of the Day Container
// ------------------------------------------------------------------------------------ //
class CMOTDWindow;

class CMOTD : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CMOTD, vgui::EditablePanel);

public:
	CMOTD(vgui::Panel *pParent, const char *pszName);

	vgui::ImagePanel *GetDefaultMOTD(void) const { return m_pDefaultMOTD; }

protected:
	virtual bool UseTraverseSizing(void) { return true; }

private:
	CMOTDWindow *m_pMOTDWindow;
	vgui::ImagePanel *m_pWelcome;

	vgui::ImagePanel *m_pDefaultMOTD;
};


#endif // VGUI_MOTD_H
