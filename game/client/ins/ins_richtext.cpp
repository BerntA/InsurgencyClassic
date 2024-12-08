//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_richtext.h"
#include "ins_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
CINSRichText::CINSRichText( Panel *pParent, const char *pszPanelName )
	: RichText( pParent, pszPanelName )
{
}

//=========================================================
//=========================================================
void CINSRichText::ClearText( void )
{
	SetText( "" );
}

//=========================================================
//=========================================================
bool CINSRichText::HasMultipleLines( void )
{
	return ( GetNumLines( ) > 1 );
}

//=========================================================
//=========================================================
int CINSRichText::CalcTextWidth( void )
{
	// return width of panel when multiple lines
	if( HasMultipleLines( ) )
		return GetWide( );

	return UTIL_ComputeStringWidth( _font, m_TextStream );
}