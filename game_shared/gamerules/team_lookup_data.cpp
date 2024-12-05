//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Lookup Tables for Team Configuration.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "team_lookup.h"
#include "weapon_defines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CTeamLookup *g_pTeamLookup[ MAX_LOOKUP_TEAMS ];

//=========================================================
//=========================================================
START_TEAM_DEFINE( TEAM_USMC )

	DEFINE_TYPE( TEAMTYPE_CONVENTIONAL )
	DEFINE_NAME( "U.S. Marines" )
	DEFINE_ADDITIONALNAME( "USMC" )
	DEFINE_FULLNAME( "United States Marine Corps" )
	DEFINE_FILENAME( "usmc" )
	DEFINE_MODEL( "us_marine" )
	DEFINE_DEFAULTWEAPON( WEAPON_KABAR )

	DEFINE_RANK_PRIVATE( "Private", "PVT" )
	DEFINE_RANK_LCORPORAL( "Pvt 1st Class", "PFC" )
	DEFINE_RANK_CORPORAL( "Corporal", "CPL" )
	DEFINE_RANK_SERGEANT( "Sergeant", "SGT" )
	DEFINE_RANK_LIEUTENANT( "Lieutenant", "LT" )

END_TEAM_DEFINE( )

START_PLAYERCLASS_LOOKUP( TEAM_USMC )

	DEFINE_PLAYERCLASS( "commander", CLASSTYPE_COMMANDER )
	DEFINE_PLAYERCLASS( "corpsman", CLASSTYPE_MEDIC )
	DEFINE_PLAYERCLASS( "marksman", CLASSTYPE_NORMAL )
	DEFINE_PLAYERCLASS( "rifleman", CLASSTYPE_NORMAL )
	DEFINE_PLAYERCLASS( "support", CLASSTYPE_NORMAL )
	DEFINE_PLAYERCLASS( "engineer", CLASSTYPE_NORMAL )
	DEFINE_PLAYERCLASS( "grenadier", CLASSTYPE_NORMAL )

END_PLAYERCLASS_LOOKUP( )

//=========================================================
//=========================================================
START_TEAM_DEFINE( TEAM_IRAQI )

	DEFINE_TYPE( TEAMTYPE_UNCONVENTIONAL )
	DEFINE_NAME( "Iraqi Insurgents" )
	DEFINE_ADDITIONALNAME( "Insurgents" )
	DEFINE_FULLNAME( "Iraqi Insurgents" )
	DEFINE_FILENAME( "iraqi" )
	DEFINE_MODEL( "iraqi_insurgent" )
	DEFINE_DEFAULTWEAPON( WEAPON_BAYONET )
	DEFINE_FOREIGNLANGUAGE( )

	DEFINE_RANK_PRIVATE( "Recruit", "Rc" )
	DEFINE_RANK_LCORPORAL( "Experienced", "Exp" )
	DEFINE_RANK_CORPORAL( "Special", "Spl" )
	DEFINE_RANK_SERGEANT( "Veteran", "Vet" )
	DEFINE_RANK_LIEUTENANT( "Commander", "Cmd" )

END_TEAM_DEFINE( )

START_PLAYERCLASS_LOOKUP( TEAM_IRAQI )

	DEFINE_PLAYERCLASS( "area", CLASSTYPE_COMMANDER )
	DEFINE_PLAYERCLASS( "rifleman", CLASSTYPE_NORMAL )
	DEFINE_PLAYERCLASS( "rpggunner", CLASSTYPE_NORMAL )
	DEFINE_PLAYERCLASS( "sapper", CLASSTYPE_NORMAL )
	DEFINE_PLAYERCLASS( "support", CLASSTYPE_NORMAL )
	DEFINE_PLAYERCLASS( "medic", CLASSTYPE_MEDIC )
	DEFINE_PLAYERCLASS( "sniper", CLASSTYPE_NORMAL )

END_PLAYERCLASS_LOOKUP( )

//=========================================================
//=========================================================

// TODO: really need an animator to rerig the combine model :P

/*START_TEAM_DEFINE( TEAM_COMBINE )

	DEFINE_TYPE( TEAMTYPE_UNCONVENTIONAL )
	DEFINE_NAME( "Combine" )
	DEFINE_ADDITIONALNAME( "Combine" )
	DEFINE_FULLNAME( "Combine Soliders" )
	DEFINE_FILENAME( "combine" )
	DEFINE_MODEL( "combine" )
	DEFINE_DEFAULTWEAPON( WEAPON_BAYONET )

	DEFINE_RANK_PRIVATE( "Grunt", "Gr" )
	DEFINE_RANK_LPRIVATE( "Soldier", "Sdr" )
	DEFINE_RANK_CORPORAL( "Superior", "Sur" )
	DEFINE_RANK_SERGEANT( "Warlord", "Wr" )
	DEFINE_RANK_LIEUTENANT( "Emperor", "EM" )

END_TEAM_DEFINE( )

START_PLAYERCLASS_LOOKUP( TEAM_COMBINE )

	DEFINE_PLAYERCLASS( "emperor", CLASSTYPE_COMMANDER )
	DEFINE_PLAYERCLASS( "warlord", CLASSTYPE_NORMAL )
	DEFINE_PLAYERCLASS( "soldier", CLASSTYPE_NORMAL )

END_PLAYERCLASS_LOOKUP( )*/