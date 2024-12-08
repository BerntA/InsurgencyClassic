//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_HELPER_H
#define VGUI_HELPER_H

#include "utlvector.h"

extern bool CheckVGUIMaterialExists(const char* pszPath, bool bVGUI = true);
extern bool CheckCycleMaterialsExists(const char* pszPath, CUtlVector<int>* pValidImages = NULL);
extern void FormCyclePath(const char* pszPath, int iNum, char* pszBuffer, int iLength);
extern void FormCyclePath(int iNum, char* pszPath, int iLength);

#endif // VGUI_HELPER_H