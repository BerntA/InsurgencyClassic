//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "animation.h"
#include "baseflex.h"
#include "filesystem.h"
#include "studio.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "tier1/strtools.h"
#include "KeyValues.h"
#include "datacache/imdlcache.h"
#include "tier1/byteswap.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ---------------------------------------------------------------------
//
// CBaseFlex -- physically simulated brush rectangular solid
//
// ---------------------------------------------------------------------

// SendTable stuff.
IMPLEMENT_SERVERCLASS_ST(CBaseFlex, DT_BaseFlex)
SendPropFloat(SENDINFO_VECTORELEM(m_vecViewOffset, 0), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO_VECTORELEM(m_vecViewOffset, 1), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO_VECTORELEM(m_vecViewOffset, 2), 0, SPROP_NOSCALE),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(baseflexer, CBaseFlex); // meaningless independant class!!

CBaseFlex::CBaseFlex(void)
{
}

CBaseFlex::~CBaseFlex(void)
{
}