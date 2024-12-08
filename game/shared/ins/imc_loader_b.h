//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef IMC_LOADER_B_H
#define IMC_LOADER_B_H

// NOTE: this is code from 2004

//=========================================================
// Headers
//=========================================================
#define IMC_SIGNATURE 0x000422EF	// Initial Signature
#define IMC_VERSION 10	// File Version

#define IMC_OFFICAL 0x0000C40A	// Offical Number

//=========================================================
// Value Limits (IMC Specific)
//=========================================================

#define PROFILE_MAGICNUMBER 0x0000F7C1	// Check for a Consistent File

// the former concerns the maths required to caculate the set-bits
// and the latter shows an example of what it looks like
#define PROFILE_TEAM_REINFORCEMENTS		0	// Flags for Overloading Specific Team Data
#define PROFILE_TEAM_AIRSUPPORT			1
#define PROFILE_TEAM_TEAMTWO			2

#define PROFILE_OBJ_MAGICNUMER 0x0073F5F0

// PROFILE_TEAMONE_REINFORCEMENTS	(1<<TEAM_OVERLOAD_REINFORCEMENTS)
// PROFILE_TEAMONE_AIRSUPPORT		(1<<TEAM_OVERLOAD_AIRSUPPORT)
// PROFILE_TEAMTWO_REINFORCEMENTS	(1<<PROFILE_TEAM_TEAMTWO + PROFILE_TEAM_REINFORCEMENTS)
// PROFILE_TEAMTWO_AIRSUPPORT		(1<<PROFILE_TEAM_TEAMTWO + PROFILE_TEAM_AIRSUPPORT)

//=========================================================
//=========================================================
struct SIMCHeader
{
	unsigned int m_iSignature;
	char m_iVersion;
	unsigned int m_iOffical;
};

//=========================================================
//=========================================================
struct SIMCConfig
{
	unsigned short iMapName; // Operation Name
	unsigned int iTimeStamp; // Date of Operation
	unsigned short iMapOverview; // Map Overview

	char iNumProfiles; // Number of Profiles
	char iNumObjectives; // Number of Objectives
};

// -- char[iMapName]
// -- char[iMapOverview]

//=========================================================
//=========================================================
struct SIMCGlobalObj // written iNumObjectives
{
	char iName; // Objective Name
	char iPhonetic; // Phonectic Name
	char iColor[3]; // Objective Colour
};

// -- char[iName]

//=========================================================
//=========================================================
struct SIMCTeam // written twice (two teams)
{
	char iTeam; // Team
	char iSkin; // Skin for Team (NOTE: Not Value Checked) NOTE: Redundant

	signed char iNumWaves; // Amount of Reinforcement Waves
	short iTimeWave; // Amount of Time (in Seconds) Between Reinforcement Waves

	//signed char iMaxAirStrikes; // Amount of Air-Strikes
	//short iTimeAirStrikes; // Time (in Seconds) between Possible Air-Strikes

	signed char iSquads[MAX_SQUADS]; // Squads
};

// -- char[iSquads[SQUAD_ONE]]
// -- char iSquadOnePositions[MAX_SQUAD_SLOTS] (when cSquads[SQUAD_ONE] not 0)
// -- char[iSquads[SQUAD_TWO]]
// -- char iSquadTwoPositions[MAX_SQUAD_SLOTS] (when cSquads[SQUAD_TWO] not 0)
// -- char[iSquads[SQUAD_THREE]]
// -- char iSquadThreePositions[MAX_SQUAD_SLOTS] (when cSquads[SQUAD_THREE] not 0)
// -- char[iSquads[SQUAD_FOUR]]
// -- char iSquadFourPositions[MAX_SQUAD_SLOTS] (when cSquads[SQUAD_FOUR] not 0)

//=========================================================
//=========================================================
struct SIMCProfile // written iNumProfiles
{
	unsigned short iMagicNumber; // Magic Number (PROFILE_MAGICNUMBER)
	char iName; // Profile Name
	char iGameType; // Game Type
	unsigned short iRoundLength; // Max Round Time (in Seconds)
	char iNumObjectives; // Number of Enabled Objectives
	char iFlags; // Team Overloading
};

// -- char[iName]

// (when iFlags & TEAM_OVERLOAD_ONE_REINFORCEMENTS)
// -- char iMaxWaves; // Amount of Reinforcement Waves
// -- char iTimeWave; // Amount of Time (in Minutes) Between Reinforcement Waves

// -- SIMCProfilePart (when iFlags & TEAM_OVERLOAD_TWO_REINFORCEMENTS)
// -- char iMaxAirStrikes; // Amount of Air-Strikes
// -- char iTimeAirStrikes; // Time (in Minutes) between Possible Air-Strikes

//=========================================================
//=========================================================
struct SIMCProfileObj
{
	unsigned int iMagicNumber; // Magic Number (PROFILE_OBJ_MAGICNUMBER)
	char iID; // ID of Objective
	char iFlags; // General Flags (Mixed Spawns, Starting Spawn, Hidden and Starting Team)
	char iReqPercent; // Required Percent of Team before a Capture
	char iCapTime; // Capture Time in Seconds
};

#endif // IMC_LOADER_B_H