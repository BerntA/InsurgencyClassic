//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef INSVGUI_UTILS_H
#define INSVGUI_UTILS_H

#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
extern vgui::VPANEL UTIL_FindVChildByName( vgui::VPANEL Parent, const char *pszChildName, bool bRecurseDown );

//=========================================================
//=========================================================
enum AlignmentType_t
{
	ALIGNMENT_XPOS_RIGHT = 1,
	ALIGNMENT_XPOS_CENTER,
	ALIGNMENT_YPOS_BOTTOM,
	ALIGNMENT_YPOS_CENTER
};

struct AlignedPositionData_t
{
	AlignedPositionData_t( );

	int m_iAlignmentTypeID;
	int m_iPosition;
};

struct TeamSelectionData_t
{
	TeamSelectionData_t( void )
	{
		m_iTeamID	= 0;
		m_iSquadID	= 0;
		m_iClassID	= 0;
	}

	int m_iTeamID;
	int m_iSquadID;
	int m_iClassID;
};

extern void UTIL_FindAlignment( const char *pszXPos, const char *pszYPos, AlignedPositionData_t &XData, AlignedPositionData_t &YData );

extern int UTIL_TranslateAlignment( AlignedPositionData_t &Data );
extern int UTIL_TranslateAlignment( AlignedPositionData_t &Data, int iParentPosition );
extern int UTIL_TranslateAlignment( AlignedPositionData_t &Data, bool bProportional, int iParentPosition );

#endif // INSVGUI_UTILS_H