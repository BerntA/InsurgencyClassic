//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#if !defined( USERCMD_H )
#define USERCMD_H
#ifdef _WIN32
#pragma once
#endif

#include "vector.h"
#include "utlvector.h"
#include "imovehelper.h"

class bf_read;
class bf_write;

class CUserCmd
{
public:
	CUserCmd()
	{
		Reset();
	}

	virtual ~CUserCmd() { };

	void Reset()
	{
		command_number = 0;
		tick_count = 0;
		viewangles.Init();
		forwardmove = 0.0f;
		sidemove = 0.0f;
		upmove = 0.0f;
		buttons = 0;
		weaponselect = 0;
		random_seed = 0;
		mousedx = 0;
		mousedy = 0;

		// Pongles [

		vmuzzle.Init( );
		amuzzle.Init( );

		trackir = false;
		headangles.Init( );
		lean = 0.0f;

		// Pongles ]

		hasbeenpredicted = false;
	}

	CUserCmd& operator =( const CUserCmd& src )
	{
		if ( this == &src )
			return *this;

		command_number		= src.command_number;
		tick_count			= src.tick_count;
		viewangles			= src.viewangles;
		forwardmove			= src.forwardmove;
		sidemove			= src.sidemove;
		upmove				= src.upmove;
		buttons				= src.buttons;
		weaponselect		= src.weaponselect;
		random_seed			= src.random_seed;
		mousedx				= src.mousedx;
		mousedy				= src.mousedy;

		// Pongles [

		amuzzle				= src.amuzzle;
		vmuzzle				= src.vmuzzle;

		trackir				= src.trackir;
		headangles			= src.headangles;
		lean				= src.lean;

		// Pongles ]

		hasbeenpredicted	= src.hasbeenpredicted;

		return *this;
	}

	// For matching server and client commands for debugging
	int		command_number;
	
	// the tick the client created this command
	int		tick_count;
	
	// Player instantaneous view angles.
	QAngle	viewangles;  

	// Intended velocities
	//	forward velocity.
	float	forwardmove;   
	//  sideways velocity.
	float	sidemove;      
	//  upward velocity.
	float	upmove;         
	// Attack button states
	int		buttons;		
	// Current weapon id
	int		weaponselect;	

	int		random_seed;	// For shared random functions

	short	mousedx;		// mouse accum in x from create move
	short	mousedy;		// mouse accum in y from create move

	// Pongles [

	// muzzle stuff
	QAngle amuzzle;
	Vector vmuzzle;

	// trackir stuff
	bool	trackir;
	QAngle	headangles;
	float	lean;

	// Pongles ]

	// Client only, tracks whether we've predicted this command at least once
	bool	hasbeenpredicted;
};

void ReadUsercmd( bf_read *buf, CUserCmd *move, CUserCmd *from );
void WriteUsercmd( bf_write *buf, CUserCmd *to, CUserCmd *from );

#endif // USERCMD_H
