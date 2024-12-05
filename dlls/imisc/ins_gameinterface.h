//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_GAMEINTERFACE_H
#define INS_GAMEINTERFACE_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#include "utllinkedlist.h"

//=========================================================
//=========================================================

// these are created for map entities in order as the map entities are spawned.
class CMapEntityRef
{
public:
	int m_iEdict;			// which edict slot this entity got
	int	m_iSerialNumber;	// the edict serial number
};

//=========================================================
//=========================================================
extern CUtlLinkedList< CMapEntityRef, unsigned short > g_MapEntityRefs;

#endif // INS_GAMEINTERFACE_H