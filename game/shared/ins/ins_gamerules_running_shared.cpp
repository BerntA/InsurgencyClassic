//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
DECLARE_MODEMANAGER( GRMODE_RUNNING, CRunningMode );

//=========================================================
//=========================================================
bool CRunningMode::IsWarmingup( void ) const
{
	return GetStatus( GAMERUNNING_WARMINGUP );
}

//=========================================================
//=========================================================
bool CRunningMode::IsRestarting( void ) const
{
	return GetStatus( GAMERUNNING_RESTARTING );
}

//=========================================================
//=========================================================
bool CRunningMode::IsEnding( void ) const
{
	return GetStatus( GAMERUNNING_ENDGAME );
}

//=========================================================
//=========================================================
bool CRunningMode::IsWaitingForPlayers( void ) const
{
	return GetStatus( GAMERUNNING_PLAYERWAIT );
}

//=========================================================
//=========================================================
bool CRunningMode::IsFrozen( void ) const
{
	return GetStatus( GAMERUNNING_FREEZE );
}

//=========================================================
//=========================================================
bool CRunningMode::IsScoringAllowed( void ) const
{
	return ( !GetStatus( GAMERUNNING_PLAYERWAIT ) );
}