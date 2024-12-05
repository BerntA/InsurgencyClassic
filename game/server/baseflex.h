//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASEFLEX_H
#define BASEFLEX_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseAnimatingOverlay.h"
#include "utlvector.h"
#include "utlrbtree.h"

struct flexsettinghdr_t;
struct flexsetting_t;

//-----------------------------------------------------------------------------
// Purpose: Animated characters who have vertex flex capability (e.g., facial expressions)
//-----------------------------------------------------------------------------
class CBaseFlex : public CBaseAnimatingOverlay
{
	DECLARE_CLASS(CBaseFlex, CBaseAnimatingOverlay);
public:
	DECLARE_SERVERCLASS();
	DECLARE_PREDICTABLE();

	CBaseFlex(void);
	~CBaseFlex(void);

	void SentenceStop(void) { EmitSound("AI_BaseNPC.SentenceStop"); }
};

EXTERN_SEND_TABLE(DT_BaseFlex);

#endif // BASEFLEX_H