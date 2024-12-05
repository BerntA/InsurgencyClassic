//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef PAINHELPERSHARED_H
#define PAINHELPERSHARED_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
enum PainTypes_t
{
	PAINTYPE_RESET = -1,		// force reset
	PAINTYPE_PMINOR = 0,		// minor pain
	PAINTYPE_PMAJOR,			// major pain
	PAINTYPE_CMINOR,			// minor concussion
	PAINTYPE_CMAJOR,			// major concussion
	PAINTYPE_CLIGHT,			// light concussion
	PAINTYPE_DEATH,				// death
	PAINTYPE_BWHIZ,				// bulletwhiz
	MAX_PAINTYPES
};

#endif // PAINHELPERSHARED_H
