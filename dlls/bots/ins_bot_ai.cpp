//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of bot AI
//		Run() is called once per frame for each bot in the server, and builds a
//		CUserCmd defining bot movement which is then passed to CmdExecute().
//
//====================================================================================//

#include "ins_bot.h"

/* TODO

*/

CINSBot::CINSBot() {

	// bot starts dead
	bIsAlive = false;

	// initialize AI variables
	memset(&m_CmdLookAndMove, 0, sizeof(m_CmdLookAndMove));
	m_CmdLookAndMove.vSpringK.x = 50.0; // pitch
	m_CmdLookAndMove.vSpringK.y = 100.0; // yaw
	m_CmdLookAndMove.vDamper.x = 200.0; // pitch
	m_CmdLookAndMove.vDamper.y = 800.0; // yaw
	m_CmdLookAndMove.fWallDist = 25.0;

	memset(&m_GetAMoveOn, 0, sizeof(m_GetAMoveOn));

	memset(&m_HeHasABiggerGun, 0, sizeof(m_HeHasABiggerGun));

	memset(&m_ILoveSquares, 0, sizeof(m_ILoveSquares));

	memset(&m_Traverse, 0, sizeof(m_Traverse));

}

bool CINSBot::TraceClear(const Vector &v1, const Vector &v2, float z) {
	trace_t tr;
	UTIL_TraceLine(v1 + Vector(0, 0, z), v2 + Vector(0, 0, z), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr);
	return !tr.DidHit();
}

// Top-level AI processing
void CINSBot::Run() {

	// easy access to time values
	m_Now = gpGlobals->curtime;
	m_Dt = gpGlobals->frametime;

	// set current position and view angles
	m_vPos = GetAbsOrigin();
	QAngle aLook = GetLocalAngles();
	m_vLook.Init(aLook.x, aLook.y);

	/*
		AI stuff goes here
		Set m_vMoveTo and m_vLookAt
	*/

	//GetAMoveOn();
	ILoveSquares();

	if (m_Traverse.pPath && m_Traverse.pPath->IsPath())
		m_vLookAt = m_Traverse.pPath->GetNextPoint();
	else
		HeHasABiggerGun();

	// debugging lines (target move, target look, actual look)
	Vector vLookDir(cos(m_vLook.y), sin(m_vLook.y), 0);
	NDebugOverlay::Line(m_vPos, m_vPos + vLookDir * 50, 0, 0, 255, false, 0.1);
	NDebugOverlay::Line(m_vPos + Vector(0, 0, 5), m_vMoveTo + Vector(0, 0, 5), 255, 0, 0, false, 0.1);
	NDebugOverlay::Line(m_vPos, m_vLookAt, 255, 100, 100, false, 0.1);

	// Generate and execute the command
	CUserCmd cmd;

	// makes the bot jump off the path to test unstucking algorithms
	if (rndi(0, 500) == 0)
		cmd.buttons |= IN_JUMP;

	CmdLookAndMove(cmd);
	CmdExecute(cmd);

}

void CINSBot::Death() {

	if (m_Traverse.pPath && m_Traverse.pPath->IsPath())
		m_Traverse.pPath->KillPath();

}

void CINSBot::ILoveSquares() {
	struct ILoveSquares_data *my = &m_ILoveSquares;

	if (!my->pPath) my->pPath = new BotPath;

	// build new path if there is no path
	if (!my->pPath->IsPath()) {

		// find a random area that can be walked to
		do {
			CNavArea *nArea = NULL;
			do {
				nArea = TheNavMesh->GetNavAreaByID(rndi(0, TheNavMesh->GetNavAreaCount() - 1));
			} while (!nArea);

			// try to build the path
			my->pPath->MakePath(m_vPos, nArea->GetCenter());

		// keep trying until a path can be made
		} while (!my->pPath->IsPath());

		TraverseSetPath(my->pPath);
	}

	// walk this path
	Traverse();

}

void CINSBot::TraverseSetPath(BotPath *p) {
	struct Traverse_data *my = &m_Traverse;

	if (!p || !p->IsPath()) return;

	my->pPath = p;

	my->tTimer = 0;
	my->iStuck = 0;
	my->bNewTarget = true;
	my->vPrevPos.Init();

}

void CINSBot::Traverse() {
	struct Traverse_data *my = &m_Traverse;
	bool *bStuck = &m_CmdLookAndMove.bStuck;

	if (!my->pPath || !my->pPath->IsPath()) return;

	// get closer to the target when getting unstuck
	float fStopDistSq = STOP_DIST * STOP_DIST;
	if (my->iStuck)
		fStopDistSq *= 0.01;

	// distance to target point
	float fDistSq = my->pPath->GetPoint().AsVector2D().DistToSqr(m_vPos.AsVector2D());

	// if we reach the target point
	while (fDistSq < fStopDistSq) {

		// skip to the next point, or path is finished
		if (!my->pPath->Next()) {
			my->pPath->KillPath();
			my->bSuccess = true;
			return;
		}

		fDistSq = my->pPath->GetPoint().AsVector2D().DistToSqr(m_vPos.AsVector2D());
		my->bNewTarget = true;
	}

	// the target changed
	if (my->bNewTarget) {
		my->tTimer = m_Now + 1.0;
		
		m_vMoveTo = my->pPath->GetPoint();
		my->vPrevPos = m_vPos;
		my->iStuck = 0;

		// no longer stuck
		*bStuck = false;

	// it's been a while since the last check
	} else if (m_Now > my->tTimer) {
		my->tTimer = m_Now + 1.0;

		// are we in the same place as last time?
		if (my->vPrevPos.DistToSqr(m_vPos) < 5*5) {

			my->iStuck++;

			// when stuck, push away from walls
			*bStuck = true;

			// go back to previous point
			if (my->iStuck == 1) {

				my->pPath->Prev();

			// switch to closest point on path
			} else if (my->iStuck == 2) {

				int iClose = my->pPath->GetClosestIndex(m_vPos);
				Vector vClose = my->pPath->GetPointByIndex(iClose);

				// switch only if the closest point is accessible to the player
				if (TraceClear(m_vPos, vClose))
					my->pPath->SetCurrentIndex(iClose);

			// switch to center of current area
			} else if (my->iStuck == 3) {

				CNavArea *aArea = TheNavMesh->GetNearestNavArea(m_vPos, false, 500);

				// add if this new point will take us back to path
				if (TraceClear(aArea->GetCenter(), my->pPath->GetPoint()))
					my->pPath->InsertPoint(aArea->GetCenter());

			} else if (my->iStuck == 5) {

				my->pPath->KillPath();
				my->bSuccess = false;

			}

			m_vMoveTo = my->pPath->GetPoint();
			
		}
		my->vPrevPos = m_vPos;
	}

	my->bNewTarget = false;

	my->pPath->Debug();
}

void CINSBot::GetAMoveOn() {
	struct GetAMoveOn_data *my = &m_GetAMoveOn;

	if (m_Now > my->tTimer) {
		my->tTimer = m_Now + rndf(8, 10);

		Vector vSpot;

		vSpot.x = rndf(-1500, -500);
		vSpot.y = rndf(0, 800);
		vSpot.z = m_vPos.z;
		m_vMoveTo = vSpot;

	}
}

void CINSBot::HeHasABiggerGun() {
	struct HeHasABiggerGun_data *my = &m_HeHasABiggerGun;

	if (m_Now > my->tTimer) {
		my->tTimer = m_Now + rndf(3, 5);

		if (PlayerList.Count() > 1) {
			do {
				int i = rndi(0, PlayerList.Count()-1);
				my->p_Target = PlayerList.Element(i);
			} while ( my->p_Target == this );
		} else my->p_Target = NULL;

	} else if (!PlayerList.HasElement(my->p_Target)) {
		my->p_Target = NULL;
	}

	if (my->p_Target)
		m_vLookAt = my->p_Target->GetAbsOrigin();

}

/* Given m_vMoveTo and m_vLookAt, the user command's 'forwardmove',
	'sidemove', and 'viewangles' fields are set. The results are not
	exact; the movement commands are designed to appear humanlike
	and therefore not 100% accurate and take time to perform.
*/
void CINSBot::CmdLookAndMove( CUserCmd &cmd ) {
	struct CmdLookAndMove_data *my = &m_CmdLookAndMove;
	Vector2D vLookAtAngle;

	// if temptarget has been set and is still active, use it
	if (!my->bFollowingTarget && m_Now < my->tLookTimer) {
		vLookAtAngle = my->vTempTarget;

	// otherwise use m_vLookAt
	} else {

		// look-at direction vector
		Vector vLookAt = m_vLookAt - m_vPos;
		vLookAt.NormalizeInPlace();

		// get angles from this direction vector
		vLookAtAngle.Init(-asin(vLookAt.z), (vLookAt.y > 0)?(acos(vLookAt.x)):(-acos(vLookAt.x)));
		vLookAtAngle.x = clamp(vLookAtAngle.x, -89, 89);
	}

	// change vLookAtAngle into a delta from m_vLook
	vLookAtAngle -= m_vLook;

	// fix wraparound
	wrap(vLookAtAngle.y);

	// if timer clicks, maybe find a new temptarget
	if (m_Now >= my->tLookTimer) {

		// if we are aiming correctly, start/keep following the target
		if (vLookAtAngle.x < 15 && vLookAtAngle.y > 5) {
			my->bFollowingTarget = true;

		// otherwise, jerk the mouse randomly near the target
		} else {
			my->bFollowingTarget = false;

			// impose maximum mouse sweep
			vLookAtAngle.x = clamp(vLookAtAngle.y, -20, 20);
			vLookAtAngle.y = clamp(vLookAtAngle.y, -50, 50);

			// randomize target on each axis (tend to overshoot)
			vLookAtAngle.x += rndf(-0.4, 0.5) * vLookAtAngle.x;
			vLookAtAngle.y += rndf(-0.4, 0.5) * vLookAtAngle.y;

			// save this target for use until next timer click
			my->vTempTarget = vLookAtAngle + m_vLook;
			my->vTempTarget.x = clamp(my->vTempTarget.x, -89, 89);
			wrap(my->vTempTarget.y);
		}

		// when to recalculate
		my->tLookTimer = m_Now + rndf(0.2, 0.5) + vLookAtAngle.y/200;
	}

	// TODO: randomize generally based on excitedness and tension
	
	// change vLookAtAngle into a force
	vLookAtAngle *= my->vSpringK; // spring force
	vLookAtAngle -= my->vLookVelocity * my->vDamper * m_Dt; // damper force

	// determine and apply the force to the velocity
	my->vLookVelocity.MulAdd(my->vLookVelocity, vLookAtAngle, m_Dt);

	// determine the new view angles from m_vLook and the new velocity
	vLookAtAngle.MulAdd(m_vLook, my->vLookVelocity, m_Dt);
	vLookAtAngle.x = clamp(vLookAtAngle.x, -89, 89);
	wrap(vLookAtAngle.y);

	// move vector (not a unit direction just yet)
	// there is no Z component because the bot can't fly
    Vector vMoveTo3D = m_vMoveTo - m_vPos;
	Vector2D vMoveTo = vMoveTo3D.AsVector2D();

	int iWalk = 0; // backwards, none, or forwards (-1, 0, 1)
	int iStrafe = 0; // left, none, or right (-1, 0, 1)

	// determine distance remaining for adjustment stop levels
	int iStopLevel;
	if (vMoveTo.IsLengthGreaterThan(STOP_DIST)) iStopLevel = 0;
	else if (vMoveTo.IsLengthGreaterThan(STOP_DIST * 0.5)) iStopLevel = 1;
	else if (vMoveTo.IsLengthGreaterThan(STOP_DIST * 0.1)) iStopLevel = 2;
	else iStopLevel = 3;

	// if just moved inwards a level, start waiting
	if (iStopLevel > my->iPrevStopLevel) {
		my->tStopTimer = m_Now + rndf(0.2, 0.5);

	// if not at highest level, and at same level (and not waiting), or moved outwards, process the move
	} else if (iStopLevel < 3 && ((iStopLevel == my->iPrevStopLevel && m_Now > my->tStopTimer) || iStopLevel < my->iPrevStopLevel)) {

		// make sure we stop waiting now
		my->tStopTimer = m_Now - 1;

		// now it becomes a direction vector
		vMoveTo.NormalizeInPlace();

		// directions we will go if we walk or strafe
		Vector2D vWalkDir(cos(vLookAtAngle.y), sin(vLookAtAngle.y));
		Vector2D vStrafeDir(vWalkDir.y, -vWalkDir.x);

		// if forced settings exist, start with them
		if (m_Now <= my->tForceTimer) {
			iWalk = (my->iForceWalk)?(my->iForceWalk):(0);
			iStrafe = (my->iForceStrafe)?(my->iForceStrafe):(0);
		}

		// calculate only what we have to
		if (!iWalk || !iStrafe) {

			// dot product tells us if forwards or backwards will get us there
			float fDot = vMoveTo.Dot(vWalkDir);

			// walk only if there is no forced walk
			if (!iWalk) {
				
				// walk if the target is within forward threshold
				float fWalkAngle = sin(15);
				iWalk = ((fDot > fWalkAngle)?(1):( (fDot < -fWalkAngle)?(-1):( 0 ) ));
			}

			// strafe only if there is no forced strafe
			if (!iStrafe) {       

				// strafe if the target is within strafe threshold
				float fStrafeAngle = cos(15);
				if (fDot < fStrafeAngle && fDot > -fStrafeAngle) {

					// we know we need to strafe; find the direction
					fDot = vMoveTo.Dot(vStrafeDir);
					iStrafe = (fDot > 0)?(1):(-1);

				}
			}
		}

		// if its time to check for forced movements
		if ((iWalk || iStrafe) && (m_Now > my->tForceTimer)) {
			my->tForceTimer = m_Now + rndf(0.2, 0.4);

			my->iForceWalk = 0;
			my->iForceStrafe = 0;

			bool bWalkRight = false;
			bool bWalkLeft = false;
			bool bStrafeUp = false;
			bool bStrafeDown = false;
			Vector vStart, vEnd;
			Vector vPos = m_vPos + Vector(0, 0, 36);
			Vector vWalkDir3(vWalkDir.x, vWalkDir.y, 0);
			Vector vStrafeDir3(vStrafeDir.x, vStrafeDir.y, 0);

			// are we trying to walk forward?
			if (iWalk) {

				// trace from left/right corners of collision box forwards or backwards
				vStart = m_vPos + my->fWallDist * ( (vWalkDir3 * iWalk) + vStrafeDir3 );
				vEnd = vStart + my->fWallDist * (vWalkDir3 * iWalk);
				bWalkRight = TraceClear(vStart, vEnd);

				vStart = m_vPos + my->fWallDist * ( (vWalkDir3 * iWalk) - vStrafeDir3 );
				vEnd = vStart + my->fWallDist * (vWalkDir3 * iWalk);
				bWalkLeft = TraceClear(vStart, vEnd);

				// if both hit, there is definitely a wall to back away from
				if (!bWalkRight && !bWalkLeft) {
					my->iForceWalk = -iWalk;
				}
			}

			// are we trying to strafe?
			if (iStrafe) {

				// trace from forward/backward corners of collision box left or right
				vStart = m_vPos + my->fWallDist * ( (vStrafeDir3 * iStrafe) + vWalkDir3 );
				vEnd = vStart + my->fWallDist * (vStrafeDir3 * iStrafe);
				bStrafeUp = TraceClear(vStart, vEnd);

				vStart = m_vPos + my->fWallDist * ( (vStrafeDir3 * iStrafe) - vWalkDir3 );
				vEnd = vStart + my->fWallDist * (vStrafeDir3 * iStrafe);
				bStrafeDown = TraceClear(vStart, vEnd);

				// we need to strafe away from this wall
				if (!bStrafeUp && !bStrafeDown) {
					my->iForceStrafe = -iStrafe;
				}
			}
			
			// is something partly in front that we can strafe around?
			if (iWalk && !my->iForceStrafe) {
				if (bWalkRight && !bWalkLeft) { // right clear, not left
					my->iForceStrafe = 1;
				} else if (!bWalkRight && bWalkLeft) { // left clear, not right
					my->iForceStrafe = -1;
				}
			}

			// is something partly beside that we can walk around?
			if (iStrafe && !my->iForceWalk) {
				if (bStrafeUp && !bStrafeDown) { // forward clear, not back
					my->iForceWalk = 1;
				} else if (!bStrafeUp && bStrafeDown) { // back clear, not forward
					my->iForceWalk = -1;
				}
			}
		}

		// finally adjust the values
		iWalk = (my->iForceWalk)?(my->iForceWalk):(iWalk);
		iStrafe = (my->iForceStrafe)?(my->iForceStrafe):(iStrafe);

	}

	// save this level for next frame
	my->iPrevStopLevel = iStopLevel;

	// Apply the movement
	cmd.viewangles.Init(vLookAtAngle.x, vLookAtAngle.y, 0);
	cmd.forwardmove = iWalk * 600;
	cmd.sidemove = iStrafe * 600;

}



