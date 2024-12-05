//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Custom bot AI for Insurgency using Source nav_mesh
//
//=============================================================================//

#ifndef INS_BOT_H
#define INS_BOT_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "player.h"
#include "ins_player.h"
#include "in_buttons.h"
#include "movehelper_server.h"
#include "gameinterface.h"
#include "ins_gamerules.h"
#include "ins_stats_shared.h"
#include "nav_mesh.h"

#ifdef _DEBUG
	#define BOTS_ENABLED "1"
#else
	#define BOTS_ENABLED "1" // enabled by default (for now)
#endif

// these are going to get very useful
#define MsgV(name, vec) (Msg("%s: %.3f %.3f %.3f\n", name, vec.x, vec.y, vec.z))
#define MsgV2(name, vec) (Msg("%s: %.3f %.3f\n", name, vec.x, vec.y))
#define rndf(lo, hi) (random->RandomFloat(lo, hi))
#define rndi(lo, hi) (random->RandomInt(lo, hi))
#define asin(x) (asin(x)*57.295779513)
#define acos(x) (acos(x)*57.295779513)
#define sin(x) (sin(x/57.295779513))
#define cos(x) (cos(x/57.295779513))
#define wrap(x) (x = ((x > 180)?(x - 360):( (x < -180)?(x + 360):(x))))

// parameters
#define STOP_DIST 50.0 // stop moving when m_vMoveTo is closer than this

// List of live players in the server
extern CUtlVector<CINSPlayer *> PlayerList;

class BotPath {
public:
	BotPath();
	
	bool MakePath(const Vector &vStart, const Vector &vEnd); // build path between two points
	void KillPath(); // destroy the path

	bool IsPath(); // returns true if the path is ready

	Vector GetPoint(); // return the current point
	Vector GetNextPoint(); // look ahead to next point

	bool Next(); // make the next point current; return false if already at end
	bool Prev(); // make the previous point current; return false if already at start

	void SetCurrentIndex(int i); // make this point index current
	int GetCurrentIndex(); // return the current point index

	Vector GetPointByIndex(int i); // return the point at the given index
	int GetClosestIndex(const Vector &v); // return the closest point index

	void InsertPoint(const Vector &v); // insert a new point before the current and make it current

	void Debug(); // display the path using debug overlay lines
	
private:

	bool m_bIsPath;

	typedef struct PathPoint_s {
		Vector vPoint;
		CUtlVector<HidingSpot *> hSpots;
	} PathPoint;

	CUtlVector<PathPoint *> m_pPath;

	int iCurrentPoint;

};

// Bot class
class CINSBot : public CINSPlayer {
	DECLARE_CLASS(CINSBot, CINSPlayer);
public:
	CINSBot(); // Constructor

	void Run(); // Once-per-frame bot calculations (called by Bot_Update)
	void Death(); // Reset AI when bot is killed (called by Bot_Update)

	bool Debug(); // Override AI with debugging commands if necessary
	void CmdExecute( CUserCmd &cmd ); // Send a user command to the engine
	
	//void PhysicsSimulate(); // Override: fix hitbox problem
	int GetDeathMenuType(); // Override: bots don't get a menu

	char m_Name[64]; // the bot's name in the server
	bool bIsAlive; // bot alive status during previous frame

private:

	float m_Now; // the time right now
	float m_Dt; // the timestep

	Vector m_vPos; // current player origin (read-only)
	Vector2D m_vLook; // current view angles (x = pitch, y = yaw) (read-only)
	
	Vector m_vMoveTo; // desired point to move towards
	Vector m_vLookAt; // desired point to look at

	/* AI */

	void ILoveSquares(); // 'Traverse' to a random NavArea
	struct ILoveSquares_data {
		BotPath *pPath;
	} m_ILoveSquares;

	void GetAMoveOn(); // 'm_vMoveTo' a random spot at random times
	struct GetAMoveOn_data {
		float tTimer;
	} m_GetAMoveOn;

	void HeHasABiggerGun(); // 'm_vLookAt' a random player at random times
	struct HeHasABiggerGun_data {
		float tTimer;
		CINSPlayer *p_Target; // the player to look at
	} m_HeHasABiggerGun;

	/* Common AI */

	// return true if nothing blocks at the given height (default halfheight)
	bool TraceClear(const Vector &v1, const Vector &v2, float z = 36);

	void TraverseSetPath( BotPath *p ); // set current path
	void Traverse(); // move along to the current path
	struct Traverse_data {
		// public
		bool bSuccess; // after KillPath: true = complete, false = lost

		// private
		BotPath *pPath;
		float tTimer;
		Vector vPrevPos;
		int iStuck;
		bool bNewTarget;
	} m_Traverse;

	/* Command */

	void CmdLookAndMove( CUserCmd &cmd ); // apply m_vMoveTo and m_vLookAt
	struct CmdLookAndMove_data {
		// public
		Vector2D vSpringK; // spring constant for view angle
		Vector2D vDamper; // damping constant for view angle
		float fWallDist; // use trace-controlled forced movements within this threshold

		// to be set by Traverse
		bool bStuck;

		// private
		float tLookTimer;
		float tStopTimer;
		Vector2D vTempTarget;
		Vector2D vLookVelocity;
		bool bFollowingTarget;
		int iPrevStopLevel;
		float tForceTimer;
		int iForceWalk;
		int iForceStrafe;
	} m_CmdLookAndMove;

	/* Debug */

	bool Debug_Mimic();
	bool Debug_Frozen();
};

extern void Bot_Initialize( void );

#endif // INS_BOT_H
