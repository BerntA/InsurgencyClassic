//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef IN_BUTTONS_H
#define IN_BUTTONS_H
#ifdef _WIN32
#pragma once
#endif

#define IN_ATTACK		(1 <<  0)
#define IN_JUMP			(1 <<  1)
#define IN_CROUCH		(1 <<  2)
#define IN_PRONE		(1 <<  3)
#define IN_FORWARD		(1 <<  4)
#define IN_BACK			(1 <<  5)
#define IN_USE			(1 <<  6)
#define IN_LEFT			(1 <<  7)
#define IN_RIGHT		(1 <<  8)
#define IN_MOVELEFT		(1 <<  9)
#define IN_MOVERIGHT	(1 << 10)
#define IN_RELOAD		(1 << 11)
#define IN_FIREMODE		(1 << 12)
#define IN_LEAN_LEFT	(1 << 13)
#define IN_LEAN_RIGHT	(1 << 14)
#define IN_SPRINT		(1 << 15)
#define IN_SHUFFLE		(1 << 16)	// walk key
#define IN_SPECIAL1		(1 << 17)
#define IN_SPECIAL2		(1 << 18)
#define IN_BANDAGE		(1 << 19)
#define IN_CANCEL		(1 << 20)

#endif // IN_BUTTONS_H