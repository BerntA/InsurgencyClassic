//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of BotPath for a path between two points
//
//====================================================================================//

#include "ins_bot.h"
#include "nav_pathfind.h"

/* TODO

*/

BotPath::BotPath() {
	m_bIsPath = false;
}

void BotPath::KillPath() {
	m_bIsPath = false;
}

bool BotPath::IsPath() {
	return m_bIsPath;
}

bool BotPath::Next() {
	if (iCurrentPoint >= m_pPath.Count() - 1)
		return false;

	iCurrentPoint++;
	return true;
}

bool BotPath::Prev() {
	if (iCurrentPoint <= 0)
		return false;

	iCurrentPoint--;
	return true;
}

void BotPath::SetCurrentIndex(int i) {
	if (i >= 0 && i < m_pPath.Count())
		iCurrentPoint = i;
}

int BotPath::GetCurrentIndex() {
	return iCurrentPoint;
}

Vector BotPath::GetPointByIndex(int i) {
	return m_pPath.Element(i)->vPoint;
}

int BotPath::GetClosestIndex(const Vector &v) {

	int iClosest = 0;
	float fDistSq = m_pPath.Element(0)->vPoint.DistToSqr(v);

	for (int i=1; i<m_pPath.Count(); i++) {

		float fDistSqTest = m_pPath.Element(i)->vPoint.DistToSqr(v);
		if (fDistSqTest < fDistSq) {
			fDistSq = fDistSqTest;
			iClosest = i;
		}
	}

	return iClosest;
}

Vector BotPath::GetPoint() {
	return m_pPath.Element(iCurrentPoint)->vPoint;
}

Vector BotPath::GetNextPoint() {
	int i = iCurrentPoint + 1;

	if (i >= m_pPath.Count())
		i--;

	return m_pPath.Element(i)->vPoint;
}

void BotPath::InsertPoint(const Vector &v) {

	PathPoint *pt = new PathPoint;
	pt->vPoint = v;
	m_pPath.InsertBefore(iCurrentPoint, pt);

}

// cost functor for MakePath
float CostWalkOnly(CNavArea *area, CNavArea *fromArea, const CNavLadder *ladder) {
	
	// first area in path, no cost
	if (fromArea == NULL)
		return 0;

	// i hate ladders
	if (ladder)
		return -1;

	// i hate crouching
	if (area->GetAttributes() & NAV_MESH_CROUCH)
		return -1;

	// i hate jumping too
	if (area->GetAttributes() & NAV_MESH_JUMP)
		return -1;

	// i just want to walk in a straight line
	return fromArea->GetCostSoFar() + (area->GetCenter() - fromArea->GetCenter()).Length();
}

bool BotPath::MakePath(const Vector &vStart, const Vector &vEnd) {

	// find start and end areas
	CNavArea *aStart = TheNavMesh->GetNearestNavArea(vStart, false, 500);
	CNavArea *aEnd = TheNavMesh->GetNavArea(vEnd);

	// build the area path between these two areas
	m_bIsPath = NavAreaBuildPath(aStart, aEnd, NULL, CostWalkOnly);
	if (!m_bIsPath) return false;

    m_pPath.RemoveAll();

	PathPoint *pt;

	// loops through segments in reverse order, setting PathPoint for each
	for ( CNavArea *aCurrent = aEnd; aCurrent != aStart; aCurrent = aCurrent->GetParent() ) {

		// Add center of this area
		pt = new PathPoint;
		pt->vPoint = aCurrent->GetCenter();
		m_pPath.AddToHead(pt);

		// Add portal between this area and parent area
		if (aCurrent->GetParent() && aCurrent->GetParentHow() < NUM_DIRECTIONS) {
			NavDirType dir = OppositeDirection((NavDirType)aCurrent->GetParentHow());
			float halfWidth;
			pt = new PathPoint;
			aCurrent->ComputePortal(aCurrent->GetParent(), dir, &pt->vPoint, &halfWidth);
			pt->vPoint.z = ( aCurrent->GetCenter().z + aCurrent->GetParent()->GetCenter().z ) * 0.5;
			m_pPath.AddToHead(pt);
		}

	}

	// connect start and end points to path
	pt = new PathPoint;
	pt->vPoint = vEnd;
	m_pPath.AddToTail(pt);

	pt = new PathPoint;
	pt->vPoint = vStart;
	m_pPath.AddToHead(pt);

	iCurrentPoint = 0;

	return true;
}

void BotPath::Debug() {

	for (int i=1; i<m_pPath.Count(); i++) {
		NDebugOverlay::Line(m_pPath.Element(i-1)->vPoint + Vector(0, 0, 3), m_pPath.Element(i-1)->vPoint + Vector(0, 0, 8), 0, 255, 0, false, 0.1);
		NDebugOverlay::Line(m_pPath.Element(i-1)->vPoint + Vector(0, 0, 3), m_pPath.Element(i)->vPoint + Vector(0, 0, 5), 0, 255, 0, false, 0.1);
		NDebugOverlay::Line(m_pPath.Element(i-1)->vPoint + Vector(0, 0, 5), m_pPath.Element(i)->vPoint + Vector(0, 0, 5), 0, 255, 0, false, 0.1);
		NDebugOverlay::Line(m_pPath.Element(i-1)->vPoint + Vector(0, 0, 8), m_pPath.Element(i)->vPoint + Vector(0, 0, 5), 0, 255, 0, false, 0.1);
	}

}

