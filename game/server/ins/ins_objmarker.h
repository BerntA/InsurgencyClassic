//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_OBJMARKER_H
#define INS_OBJMARKER_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"

// ------------------------------------------------------------------------------------ //
// Insurgency Objective
// ------------------------------------------------------------------------------------ //
class CINSObjMarker : public CBaseEntity
{
public:
	DECLARE_CLASS(CINSObjMarker, CBaseEntity);
	DECLARE_DATADESC();

	int UpdateTransmitState() {	return SetTransmitState(FL_EDICT_DONTSEND);	}

	int GetParentObj(void) const { return m_iParentObj; }

private:
	int m_iParentObj;
};

#endif // INS_OBJMARKER_H
