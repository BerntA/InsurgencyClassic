//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_objmarker.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS(ins_objmarker, CINSObjMarker);

BEGIN_DATADESC( CINSObjMarker )

	DEFINE_KEYFIELD(m_iParentObj, FIELD_INTEGER, "parentobj"),

END_DATADESC()
