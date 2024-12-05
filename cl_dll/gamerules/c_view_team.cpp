//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#include "cbase.h"
#include "view_team_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#if defined(CViewTeam)
#undef CViewTeam	
#endif

//=========================================================
//=========================================================
IMPLEMENT_CLIENTCLASS_DT( C_ViewTeam, DT_ViewTeam, CViewTeam )

END_RECV_TABLE( )
