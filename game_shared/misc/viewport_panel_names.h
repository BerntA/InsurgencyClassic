//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef VIEWPORT_PANEL_NAMES_H
#define VIEWPORT_PANEL_NAMES_H
#ifdef _WIN32
#pragma once
#endif


// default panel name definitions
#define PANEL_ALL			"all"		// all current panels
#define PANEL_ACTIVE		"active"	// current active panel			

// Pongles [

#define PANEL_CHANGETEAM	"changeteam"		// change team
#define PANEL_SQUADSELECT	"squadselect"		// change squad
#define PANEL_GAMEINFO		"gameinfo"			// info about the game and map as well
#define PANEL_DEATHINFO		"deathinfo"			// death info, includes information about who you took damage from etc
//#define PANEL_COMMANDER	"commander"
#define PANEL_ENDGAME		"endgame"			// game info, includes scoreboard, and chat etc
#define PANEL_CUSTOMIZEGUI	"customizegui"		// customize player
#define PANEL_SCOREBOARD	"scoreboard"		// scoreboard

#define PANEL_MAPINFO		"mapinfo"
#define PANEL_MAPOVER		"mapover"
#define PANEL_OVERVIEW		"overview"
#define PANEL_PLAYERORDER	"playerorder"
#define PANEL_TEAM			"teamchoice"
#define PANEL_SQUAD			"squadchoice"
#define PANEL_OBJMAP		"objmap"
#define PANEL_SCOREBOARD	"scoreboard"
#define PANEL_COMMS			"comms"

#ifdef TESTING

#define PANEL_TESTERNOTICE	"testernotice"

#endif

#define PANEL_SPECGUI		"specgui"

// ---

#define PANEL_NAV_PROGRESS	"nav_progress"

// Pongles ]

#endif // VIEWPORT_PANEL_NAMES_H
