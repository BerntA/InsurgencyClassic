//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef C_VIEW_TEAM_H
#define C_VIEW_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_team.h"

//=========================================================
//=========================================================
class C_ViewTeam : public C_Team
{
public:
	DECLARE_CLASS( C_ViewTeam, C_Team );
	DECLARE_CLIENTCLASS( );

public:
	const char *GetName( void ) const;
};

#endif // C_VIEW_TEAM_H
