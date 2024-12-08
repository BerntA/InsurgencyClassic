//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef CLIPDEF_H
#define CLIPDEF_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_defines.h"

//=========================================================
//=========================================================
enum ClipTypes_t
{
	INVALID_CLIP = -1,
	CLIP_M16 = 0,
	CLIP_M9,
	CLIP_M14,
	CLIP_M249,
	CLIP_AK47,
	CLIP_AKS74U,
	CLIP_RPK,
	CLIP_UZI,
	CLIP_MAKAROV,
	CLIP_FNFAL,
	CLIP_SKS,
	CLIP_L42A1,
	CLIP_M1014,
	CLIP_TOZ,
	MAX_CLIPS
};

DECLARE_STRING_LOOKUP_CONSTANTS( int, cliptypeids )
#define LookupClipTypeID( k, j ) STRING_LOOKUP( cliptypeids, k, j )

//=========================================================
//=========================================================
struct Clip_t
{
	int m_iWeaponType;
	int m_iBulletType;
	int m_iBullets;
};

#define MAX_CLIPSIZE 250
#define MAX_CLIPSIZE_BITS 8

//=========================================================
//=========================================================
class CClipDef : public CAutoGameSystem
{
public:
	static CClipDef *GetClipDef( void );

private:
	CClipDef( );

	bool Init( void );
	bool Load( void );

public:
	Clip_t m_ClipData[ MAX_CLIPS ];

private:
	static CClipDef m_ClipDef;
};

extern CClipDef *GetClipDef( void );

#endif // CLIPDEF_H
