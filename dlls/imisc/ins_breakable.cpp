//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "func_break.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define SF_INSBREAK_NOBULLETS				0x01000
#define SF_INSBREAK_ENGINEER				0x02000

//=========================================================
//=========================================================
class CINSBreakable : public CBreakable
{
	DECLARE_CLASS(CINSBreakable, CBreakable);

public:
	int OnTakeDamage(const CTakeDamageInfo &info);
};

//=========================================================
//=========================================================
int CINSBreakable::OnTakeDamage(const CTakeDamageInfo &info)
{
	if((FBitSet(m_spawnflags, SF_INSBREAK_NOBULLETS) && info.GetDamageType() & DMG_BULLET) ||
		(FBitSet(m_spawnflags, SF_INSBREAK_ENGINEER) && info.GetDamageType() & DMG_ENGINEER))
		return 0;

	return BaseClass::OnTakeDamage(info);
}
