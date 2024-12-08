//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "mapname_utils.h"
#include <vgui_controls/vgui_helper.h>
#include "imc_config.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
bool GetMapImagePath(const char *pszType, const char *pszName, bool bCycler, char *pszBuffer, int iLength, CUtlVector<int> *pValidImages)
{
	FormMapImagePath(pszType, pszName, true, pszBuffer, iLength);

	// check that its a valid path
	if(!bCycler)
	{
		if(CheckVGUIMaterialExists(pszBuffer))
			return true;
	}
	else
	{
		if(CheckCycleMaterialsExists(pszBuffer, pValidImages))
			return true;
	}

	// ... otherwise use an invalid one
	FormMapImagePath(pszType, pszName, false, pszBuffer, iLength);

	if(bCycler)
		CheckCycleMaterialsExists(pszBuffer, pValidImages);

	return false;
}

//=========================================================
//=========================================================
void FormMapImagePath(const char *pszType, const char *pszName, bool bValid, char *pszBuffer, int iLength)
{
	if(bValid)
	{
		char szMapName[MAX_MAPNAME_LENGTH];
		GetVGUIMapName(szMapName);
		
		Q_snprintf(pszBuffer, iLength, "maps/%s/%s/%s", szMapName, pszType, pszName);
	}
	else
	{
		Q_snprintf(pszBuffer, iLength, "resources/invalidmap/%s/%s", pszType, pszName);
	}
}

//=========================================================
//=========================================================
void GetVGUIMapName(char *pszBuffer)
{
	const char *pszVGUIMapName = NULL;

	if(IMCConfig())
	{
		pszVGUIMapName = IMCConfig()->GetVGUIMapName();

		if(*pszVGUIMapName)
		{
			Q_strncpy(pszBuffer, pszVGUIMapName, MAX_MAPNAME_LENGTH);
			return;
		}
	}

	ConvertClientMapNameToServerMapName(pszBuffer, engine->GetLevelName());
}
