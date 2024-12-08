//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "hint_helper.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
namespace HintData {

//=========================================================
//=========================================================
class CSniper : public IHint
{
	const char *Text( void ) const { return "Press £ to use Scope"; }
	const char *Binding( void ) const { return "special2"; }
};

DECLARE_HINT( HINT_SNIPER, CSniper );

//=========================================================
//=========================================================
class CIronsights : public IHint
{
	const char *Text( void ) const { return "Press £ to use Ironsights"; }
	const char *Binding( void ) const { return "special2"; }
};

DECLARE_HINT( HINT_IRONSIGHTS, CIronsights );

//=========================================================
//=========================================================
class CGrenadeThrow : public IHint
{
	const char *Text( void ) const { return "Press £ for a Shorter Throw"; }
	const char *Binding( void ) const { return "special1"; }
};

DECLARE_HINT( HINT_GRENADE, CGrenadeThrow );

//=========================================================
//=========================================================
class CCaptureCapturing : public IHint
{
	const char *Text( void ) const { return "You are Capturing an Objective"; }
	const char *TextAdditional( void ) const { return "Capture all of the Objectives to win the Round"; }
};

DECLARE_HINT( HINT_CAPTURE_CAPTURING, CCaptureCapturing );

//=========================================================
//=========================================================
class CCaptureDefending : public IHint
{
	const char *Text( void ) const { return "You are Defending your Objective"; }
};

DECLARE_HINT( HINT_CAPTURE_DEFENDING, CCaptureDefending );

//=========================================================
//=========================================================
class CCaptureNEP : public IHint
{
	const char *Text( void ) const { return "Not Enough Players to Start Capturing"; }
};

DECLARE_HINT( HINT_CAPTURE_NEP, CCaptureNEP );

//=========================================================
//=========================================================
class CCaptureBlock : public IHint
{
	const char *Text( void ) const { return "Enemy Players are Blocking you from Capturing"; }
};

DECLARE_HINT( HINT_CAPTURE_BLOCK, CCaptureNEP );

//=========================================================
//=========================================================
class CCaptureCaptured : public IHint
{
	const char *Text( void ) const { return "You have Captured an Objective"; }
	const char *TextAdditional( void ) const { return "You have also Gained Morale Points"; }
};

DECLARE_HINT( HINT_CAPTURE_CAPTURED, CCaptureCaptured );

//=========================================================
//=========================================================
class CCaptureCannot : public IHint
{
	const char *Text( void ) const { return "This Objective cannot be Captured at this Time"; }
};

DECLARE_HINT( HINT_CAPTURE_CANNOT, CCaptureCannot );

//=========================================================
//=========================================================
class CCommanderMissing : public IHint
{
	const char *Text( void ) const { return "Your Squad does not have a Commander"; }
};

DECLARE_HINT( HINT_NOCOMMANDER, CCommanderMissing );

//=========================================================
//=========================================================
class CCommanderInitialOrders : public IHint
{
	const char *Text( void ) const { return "Remember to Assign an Order!"; }
};

DECLARE_HINT( HINT_INITIALORDERS, CCommanderInitialOrders );

//=========================================================
//=========================================================
class CNoKillConfirmation : public IHint
{
	const char *Text( void ) const { return "Kills are not Confirmed"; }
	const char *TextAdditional( void ) const { return "They will be Confirmed when the Round is Over"; }
};

DECLARE_HINT( HINT_NKILLCONF, CNoKillConfirmation );

//=========================================================
//=========================================================
class CFriendlyHit : public IHint
{
	const char *Text( void ) const { return "Do not Injure your own Team"; }
	const char *TextAdditional( void ) const { return "You may be Punished"; }
};

DECLARE_HINT( HINT_FHIT, CFriendlyHit );

//=========================================================
//=========================================================
class CTeamKillPunish : public IHint
{
	const char *Text( void ) const { return "You will miss a Reinforcement for TK"; }
};

DECLARE_HINT( HINT_TKPUNISH, CTeamKillPunish );

//=========================================================
//=========================================================
class CSpawnProtection : public IHint
{
	const char *Text( void ) const { return "You are in Spawn Protection"; }
};

DECLARE_HINT( HINT_SPAWNPROTECTION, CSpawnProtection );

//=========================================================
//=========================================================
class CSniperZone : public IHint
{
	const char *Text( void ) const { return "You are in a Sniper Zone"; }
	const char *TextAdditional( void ) const { return "Leave the Area Quickly"; }
};

DECLARE_HINT( HINT_SNIPERZONE, CSniperZone );

//=========================================================
//=========================================================
class CMineField : public IHint
{
	const char *Text( void ) const { return "You are in a Minefield"; }
	const char *TextAdditional( void ) const { return "Leave the Area Carefully"; }
};

DECLARE_HINT( HINT_MINEFIELD, CMineField );

//=========================================================
//=========================================================
class CStanceAlternative : public IHint
{
	const char *Text( void ) const { return "An Alternative Stance Mode is Available"; }
	const char *TextAdditional( void ) const { return "See Advanced Options"; }
};

DECLARE_HINT( HINT_STANCEALT, CStanceAlternative );

};