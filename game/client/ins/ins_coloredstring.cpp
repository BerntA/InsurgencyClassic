//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"

#include <vgui_controls/richtext.h>

#include "ins_utils.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
int UTIL_TeamColorLookup( int iTeamID )
{
	switch( iTeamID )
	{
	case TEAM_UNASSIGNED:
		return CLOOKUP_TEAM_UNASSIGNED;

	case TEAM_ONE:
		return CLOOKUP_TEAM_ONE;

	case TEAM_TWO:
		return CLOOKUP_TEAM_TWO;

	case TEAM_SPECTATOR:
		return CLOOKUP_TEAM_SPECTATOR;
	}

	return CLOOKUP_INVALID;
}

//=========================================================
//=========================================================
void CColorLookup::FindColor( int iID, Color &StringColor ) const
{
	int iTeamID = INVALID_TEAM;

	switch( iID )
	{
		case CLOOKUP_TEAM_UNASSIGNED:
		{
			iTeamID = TEAM_UNASSIGNED;
			break;
		}

		case CLOOKUP_TEAM_ONE:
		{
			iTeamID = TEAM_ONE;
			break;
		}

		case CLOOKUP_TEAM_TWO:
		{
			iTeamID = TEAM_TWO;
			break;
		}

		case CLOOKUP_TEAM_SPECTATOR:
		{
			iTeamID = TEAM_SPECTATOR;
			break;
		}
	}

	if( iTeamID == INVALID_TEAM )
		return;

	const Color &TeamColor = INSRules( )->TeamColor( iTeamID );
	StringColor = TeamColor;
}

//=========================================================
//=========================================================
CMarkerColor::CMarkerColor( )
{
	m_iLength = 0;
	m_iDataType = CDATATYPE_LOOKUP;

	m_Data.m_iLookupID = CLOOKUP_INVALID;
}

//=========================================================
//=========================================================
void CMarkerColor::Init( int iLength, int iLookupID )
{
	m_iLength = iLength;
	m_iDataType = CDATATYPE_LOOKUP;

	m_Data.m_iLookupID = iLookupID;
}

//=========================================================
//=========================================================
void CMarkerColor::Init( int iLength, const Color &StringColor )
{
	m_iLength = iLength;
	m_iDataType = CDATATYPE_UNIQUE;

	m_Data.m_Color.r = StringColor[ 0 ];
	m_Data.m_Color.g = StringColor[ 1 ];
	m_Data.m_Color.b = StringColor[ 2 ];
}

//=========================================================
//=========================================================
void CMarkerColor::Parse( Color &StringColor, const CColorLookup &ColorLookup ) const
{
	switch( m_iDataType )
	{
		case CDATATYPE_LOOKUP:
		{
			ColorLookup.FindColor( m_Data.m_iLookupID, StringColor );

			break;
		}

		case CDATATYPE_UNIQUE:
		{
			StringColor[ 0 ] = m_Data.m_Color.r;
			StringColor[ 1 ] = m_Data.m_Color.g;
			StringColor[ 2 ] = m_Data.m_Color.b;

			break;
		}
	}
}

//=========================================================
//=========================================================
void CColoredString::Add( const char *pszMessage )
{
	Add( pszMessage, CLOOKUP_DEFAULT );
}

//=========================================================
//=========================================================
void CColoredString::Add( const char *pszMessage, int iLookupID )
{
	int iLength = AddString( pszMessage );

	if( iLength == 0 )
		return;

	CMarkerColor MarkerColor;
	MarkerColor.Init( iLength, iLookupID );

	m_ColorMarkers.AddToTail( MarkerColor );
}

//=========================================================
//=========================================================
void CColoredString::Add( const char *pszMessage, const Color &StringColor )
{
	int iLength = AddString( pszMessage );

	if( iLength == 0 )
		return;

	int iMarkerID = m_ColorMarkers.AddToTail( );

	if( !m_ColorMarkers.IsValidIndex( iMarkerID ) )
		return;

	CMarkerColor &MarkerColor = m_ColorMarkers[ iMarkerID ];
	MarkerColor.Init( iLength, StringColor );
}

//=========================================================
//=========================================================
void CColoredString::Draw( vgui::RichText *pRichText, const CColorLookup &LookupColors )
{
	char szBuffer[ MAX_COLOREDSTRING_LENGTH ];
	int iOffset = 0;

	for( int i = 0; i < m_ColorMarkers.Count( ); i++ )
	{
		const CMarkerColor &MarkerColor = m_ColorMarkers[ i ];

		szBuffer[ 0 ] = '\0';
		Q_strncpy( szBuffer, m_szString + iOffset, MarkerColor.m_iLength + 1 );

		Color StringColor;
		MarkerColor.Parse( StringColor, LookupColors );
		StringColor[ 3 ] = 255;

		pRichText->InsertColorChange( StringColor );

		pRichText->InsertString( szBuffer );
		iOffset += MarkerColor.m_iLength;
	}
}