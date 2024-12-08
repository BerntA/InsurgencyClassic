//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "vgui_controls/vgui_helper.h"
#include <vgui_controls/RichText.h>
#include <vgui_controls/imagecycler.h>
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool CheckVGUIMaterialExists(const char* pszPath, bool bVGUI)
{
	char szFullPath[MAX_PATH];
	const char* pszPathPrefix = (bVGUI ? "materials/vgui" : "materials");

	Q_snprintf(szFullPath, sizeof(szFullPath), "%s/%s.vtf", pszPathPrefix, pszPath);
	if (!g_pFullFileSystem->FileExists(szFullPath, "GAME"))
		return false;

	Q_snprintf(szFullPath, sizeof(szFullPath), "%s/%s.vmt", pszPathPrefix, pszPath);
	if (!g_pFullFileSystem->FileExists(szFullPath, "GAME"))
		return false;

	return true;
}

bool CheckCycleMaterialsExists(const char* pszPath, CUtlVector<int>* pValidImages)
{
	char szCyclePath[256];
	bool bFound = false;

	for (int i = 0; i < MAX_CYCLE_IMAGES; i++)
	{
		FormCyclePath(pszPath, i, szCyclePath, sizeof(szCyclePath));

		if (CheckVGUIMaterialExists(szCyclePath))
		{
			bFound = true;

			if (pValidImages)
				pValidImages->AddToTail(i);
		}
	}

	return bFound;
}

void FormCyclePath(const char* pszPath, int iNum, char* pszBuffer, int iLength)
{
	Q_snprintf(pszBuffer, iLength, "%s%02i", pszPath, iNum);
}

void FormCyclePath(int iNum, char* pszPath, int iLength)
{
	char szPath[256];
	FormCyclePath(pszPath, iNum, szPath, sizeof(szPath));
	Q_strncpy(pszPath, szPath, iLength);
}