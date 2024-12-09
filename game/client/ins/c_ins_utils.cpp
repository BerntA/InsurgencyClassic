//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_utils.h"

#include <vgui/isurface.h>
#include <vgui_controls/panel.h>

#include "keyvalues.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
const char *g_pszPhoneticAlphabet[ ALPHABET_SIZE ] = { 
	{ "Alpha" }, { "Bravo" }, { "Charlie" }, { "Delta" }, { "Echo" },
	{ "Foxtrot" }, { "Golf" }, { "Hotel" }, { "India" }, { "Juliet" },
	{ "Kilo" },	{ "Lima" },	{ "Mike" }, { "November" }, { "Oscar" },
    { "Papa" }, { "Quebec" }, { "Romeo" }, { "Sierra" }, { "Tango" },
	{ "Uniform" }, { "Victor" }, { "Whisky" }, { "X-Ray" }, { "Yankee" },
	{ "Zulu" }
};

//=========================================================
//=========================================================
void UTIL_ReadKeyValues( bf_read &msg, KeyValues *pData )
{
	int iDataType;
	char szKey[ 64 ];

	do
	{
		iDataType = msg.ReadByte( );

		msg.ReadString( szKey, sizeof( szKey ) );

		if( iDataType == KeyValues::TYPE_INT )
		{
			pData->SetInt( szKey, msg.ReadByte( ) );
		}
		else
		{
			char szStringData[ 64 ];

			pData->SetString( szKey, szStringData );
		}

	} while( msg.ReadByte( ) != 0 );
}

//=========================================================
//=========================================================
void UTIL_RemoveCommandFlags( const ConCommandBase *pCommand, int iFlags )
{
	char *pFlagData = ( char* )pCommand + 20;
	*( int* )pFlagData &= ~iFlags;
}

//=========================================================
//=========================================================
const char *g_pszOrientationNames[ ORIENTATION_COUNT ] = {
	"North",			// ORIENTATION_NORTH
	"North-East",		// ORIENTATION_NORTHEAST
	"West",				// ORIENTATION_EAST
	"South-East",		// ORIENTATION_SOUTHEAST
	"South",			// ORIENTATION_SOUTH
	"South-West",		// ORIENTATION_SOUTHWEST
	"West",				// ORIENTATION_WEST
	"North-West"		// ORIENTATION_NORTHWEST
};

//=========================================================
//=========================================================
const char *g_pszFireModeNames[ FIREMODE_COUNT ] = {
	"Semi",			// FIREMODE_SEMI
	"Burst",		// FIREMODE_3RNDBURST
	"Automatic"		// FIREMODE_FULLAUTO
};

//=========================================================
//=========================================================
#define MAPNAME_BLOAT_PREFIX_SIZE 5 // "maps\"
#define MAPNAME_BLOAT_SUFFIX_SIZE 3 // ".bsp"

void UTIL_ParseMapName( char *pszBuffer, int iLength )
{
	const char *pszMapName = engine->GetLevelName( );
	Q_strncpy( pszBuffer, pszMapName + MAPNAME_BLOAT_PREFIX_SIZE, Q_strlen( pszMapName ) - ( MAPNAME_BLOAT_PREFIX_SIZE + MAPNAME_BLOAT_SUFFIX_SIZE ) );
}

//=========================================================
//=========================================================
int UTIL_ComputeStringWidth( vgui::HFont &hFont, TextStream_t &TextStream )
{
	int iPixels = 0;

	for( int i = 0; i < TextStream.Count( ); i++ )
		iPixels += vgui::surface( )->GetCharacterWidth( hFont, TextStream[ i ] );

	return iPixels;
}