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
#include "motd.h"
#include <vgui_controls/imagepanel.h>
#include "gamevars_shared.h"
#include "networkstringtable_clientdll.h"
#include "gameuipanel.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define TEMP_MOTD_FILE	"~motdtemp.html"

//=========================================================
//=========================================================
CMOTDFile *g_pMOTDFile = NULL;

//=========================================================
//=========================================================
CMOTDFile::CMOTDFile(const char *pszMOTDMsg)
{
	Setup();

	Q_strncpy(m_szBuffer, pszMOTDMsg, MAX_MOTDMSG_LENGTH);
	m_iType = MOTDTYPE_MSG;
}

//=========================================================
//=========================================================
CMOTDFile::CMOTDFile(const char *pszMOTD, int iLength)
{
	Setup();

	// ensure the buffer has been filled
	if (!pszMOTD || !pszMOTD[0])
		return;

	if(Q_strcmp("http://", pszMOTD) == 0)
	{
		m_iType = MOTDTYPE_MSG;
		Q_strncpy(m_szBuffer, pszMOTD, MOTD_LENGTH);

		m_iType = MOTDTYPE_URL;
	}
	else
	{
		// open a temp file for writing
		FileHandle_t hFile = vgui::filesystem()->Open(TEMP_MOTD_FILE, "wb", "GAME");

		if(hFile == FILESYSTEM_INVALID_HANDLE)
			return;

		// write the received MOTD
		vgui::filesystem()->Write(pszMOTD, iLength, hFile);
		vgui::filesystem()->Close(hFile);

		// set the type
		m_iType = MOTDTYPE_HTML;
	}
}

//=========================================================
//=========================================================
CMOTDFile::~CMOTDFile()
{
	Assert(g_pMOTDFile == this);
	g_pMOTDFile = NULL;

	if(GetType() == MOTDTYPE_HTML)
		vgui::filesystem()->RemoveFile(TEMP_MOTD_FILE, "GAME");
}

//=========================================================
//=========================================================
const char *g_pszHostname = NULL;

void CMOTDFile::Load(void)
{
	int iMOTDIndex = g_pStringTableInfoPanel->FindStringIndex(MOTD_STRING);
	const char *pszMOTDMsg = motdmsg.GetString();

	if(pszMOTDMsg[0] != '\0')
	{
		g_pMOTDFile = new CMOTDFile(pszMOTDMsg);
	}
	else if(iMOTDIndex != ::INVALID_STRING_INDEX)
	{
		int iLength = 0;
		const char *pszMOTD = (const char *)g_pStringTableInfoPanel->GetStringUserData(iMOTDIndex, &iLength);

		g_pMOTDFile = new CMOTDFile(pszMOTD, iLength);
	}

	// now load the hostname
	int iHostnameIndex = g_pStringTableInfoPanel->FindStringIndex(INS_HOSTNAME_STRING);

	if(iHostnameIndex != ::INVALID_STRING_INDEX)
	{
		int iLength = 0;
		g_pszHostname = (const char *)g_pStringTableInfoPanel->GetStringUserData(iHostnameIndex, &iLength);
	}

	// tell the GameUI
	GameUIPanel( )->MOTDInit();
}

//=========================================================
//=========================================================
void CMOTDFile::Setup(void)
{
	// setup the global pointer
	Assert(!g_pMOTDFile);
	g_pMOTDFile = this;

	// init vars
	m_iType = MOTDTYPE_NONE;
	m_szBuffer[0] = '\0';
}

//=========================================================
//=========================================================
#define DEFAULT_MOTD_PATH		"materials/VGUI/resources/motd/default_motd.htm"
#define DEFAULT_MOTD_IMAGE_PATH "resources/motd/default_motd"

class CMOTDWindow : public CHTMLWindow
{
	DECLARE_CLASS_SIMPLE(CMOTDWindow, CHTMLWindow);

public:
	CMOTDWindow(CMOTD *pMOTD)
		: CHTMLWindow(pMOTD)
	{
		m_pMOTD = pMOTD;
	}

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		// set size
		int iParentWide, iParentTall;
		m_pMOTD->GetSize(iParentWide, iParentTall);

		SetSize(iParentWide, iParentTall);

		// set the motd
		ImagePanel *pDefaultMOTD = m_pMOTD->GetDefaultMOTD();

		if(g_pMOTDFile && g_pMOTDFile->GetType() == MOTDTYPE_HTML)
		{
			LoadFile(TEMP_MOTD_FILE);
			pDefaultMOTD->SetVisible(false);
		}
		else
		{
			LoadFile(DEFAULT_MOTD_PATH);

			pDefaultMOTD->SetPos(0, 0);
			pDefaultMOTD->SetImage(DEFAULT_MOTD_IMAGE_PATH);
			pDefaultMOTD->SetVisible(true);
		}
	}

private:
	CMOTD *m_pMOTD;
};

//=========================================================
//=========================================================
CMOTD::CMOTD(vgui::Panel *pParent, const char *pszName)
	: EditablePanel(pParent, pszName)
{
	m_pMOTDWindow = new CMOTDWindow(this);
	m_pWelcome = new ImagePanel(this, "Welcome");

	m_pDefaultMOTD = new ImagePanel(this, NULL);
	m_pDefaultMOTD->SetVisible(false);

	LoadControlSettings("Resource/UI/Frames/Panels/MOTD.res");
}