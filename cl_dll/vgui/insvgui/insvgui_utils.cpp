//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"

#include <vgui/isurface.h>
#include <vgui_controls/panel.h>

#include "insvgui_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
AlignedPositionData_t::AlignedPositionData_t( )
{
	m_iAlignmentTypeID = 0;
	m_iPosition = 0;
}

//=========================================================
//=========================================================
VPANEL UTIL_FindVChildByName( VPANEL Parent, const char *pszChildName, bool bRecurseDown )
{
	VPANEL Child = 0;
	const char *pszCurrentName = NULL;

	int iCount = ipanel( )->GetChildCount( Parent );

	for( int i = 0; i < iCount; i++ )
	{
		Child = ipanel( )->GetChild( Parent, i );
		pszCurrentName = ipanel( )->GetName( Child );

		if( !Child )
			continue;

		if( FStrEq( ipanel( )->GetName( Child ), pszChildName ) )
			return Child;

		if( bRecurseDown )
		{
			Child = UTIL_FindVChildByName( Child, pszChildName, true );

			if( Child != 0 )
				return Child;
		}
	}

	return 0;
}

//=========================================================
//=========================================================
void UTIL_FindAlignment( const char *pszXPos, const char *pszYPos, AlignedPositionData_t &XData, AlignedPositionData_t &YData )
{
	if( pszXPos )
	{
		// look for alignment flags
		if( pszXPos[ 0 ] == 'r' || pszXPos[ 0 ] == 'R' )
		{
			XData.m_iAlignmentTypeID = ALIGNMENT_XPOS_RIGHT;
			pszXPos++;
		}
		else if( pszXPos[ 0 ] == 'c' || pszXPos[ 0 ] == 'C' )
		{
			XData.m_iAlignmentTypeID = ALIGNMENT_XPOS_CENTER;
			pszXPos++;
		}

		// get the value
		XData.m_iPosition = atoi( pszXPos );
	}

	if( pszYPos )
	{
		// look for alignment flags
		if( pszYPos[ 0 ] == 'b' || pszYPos[ 0 ] == 'B' )
		{
			YData.m_iAlignmentTypeID = ALIGNMENT_YPOS_BOTTOM;
			pszYPos++;
		}
		else if( pszYPos[ 0 ] == 'c' || pszYPos[ 0 ] == 'C' )
		{
			YData.m_iAlignmentTypeID = ALIGNMENT_YPOS_CENTER;
			pszYPos++;
		}

		YData.m_iPosition = atoi( pszYPos );
	}
}

//=========================================================
//=========================================================
int UTIL_TranslateAlignment( AlignedPositionData_t &Data, bool bProportional, int *pParent )
{
	int iParentWide, iParentTall;

	if( pParent )
		iParentWide = iParentTall = *pParent;
	else
		surface( )->GetScreenSize( iParentWide, iParentTall );

	int iPosition = Data.m_iPosition;
		
	if( bProportional )
		iPosition = scheme( )->GetProportionalScaledValue( iPosition );

	switch( Data.m_iAlignmentTypeID )
	{
		case ALIGNMENT_XPOS_CENTER:
			return ( iParentWide / 2 ) + iPosition;

		case ALIGNMENT_XPOS_RIGHT:
			return iParentWide - iPosition;

		case ALIGNMENT_YPOS_BOTTOM:
			return iParentTall - iPosition;

		case ALIGNMENT_YPOS_CENTER:
			return ( iParentTall / 2 ) + iPosition;
	}

	return iPosition;
}

//=========================================================
//=========================================================
int UTIL_TranslateAlignment( AlignedPositionData_t &Data )
{
	return UTIL_TranslateAlignment( Data, true, NULL );
}

//=========================================================
//=========================================================
int UTIL_TranslateAlignment( AlignedPositionData_t &Data, int iParentPosition )
{
	return UTIL_TranslateAlignment( Data, true, &iParentPosition );
}

//=========================================================
//=========================================================
int UTIL_TranslateAlignment( AlignedPositionData_t &Data, bool bProportional, int iParentPosition )
{
	return UTIL_TranslateAlignment( Data, bProportional, &iParentPosition );
}