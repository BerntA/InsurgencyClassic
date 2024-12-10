//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//=============================================================================//

#ifndef INS_COLOREDSTRING_H
#define INS_COLOREDSTRING_H

#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
namespace vgui
{
	class RichText;
};

//=========================================================
//=========================================================
enum LookupColors_t
{
	CLOOKUP_INVALID = -1,

	CLOOKUP_DEFAULT = 0,

	CLOOKUP_TEAM_UNASSIGNED,
	CLOOKUP_TEAM_ONE,
	CLOOKUP_TEAM_TWO,
	CLOOKUP_TEAM_SPECTATOR,

	CLOOKUP_SERVER,
	CLOOKUP_THIRDPERSON
};

//=========================================================
//=========================================================
extern int UTIL_TeamColorLookup( int iTeamID );

//=========================================================
//=========================================================
#define MAX_COLOREDSTRING_LENGTH 256

class IColoredString
{
public:
	IColoredString( );

	virtual void Add( const char *pszMessage ) = 0;
	virtual void Add( const char *pszMessage, int iLookupID ) = 0;
	virtual void Add( const char *pszMessage, const Color &StringColor ) = 0;

	const char *Get( void ) const { return m_szString; }

protected:
	virtual void Reset( void );

	int AddString( const char *pszMessage );

protected:
	char m_szString[ MAX_COLOREDSTRING_LENGTH ];
};

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

//=========================================================
//=========================================================
class CColorLookup
{
public:
	virtual void FindColor( int iID, Color &StringColor ) const;
};

//=========================================================
//=========================================================
class CMarkerColor
{
public:
	CMarkerColor( );

	void Init( int iLength, int iLookupID );
	void Init( int iLength, const Color &StringColor );

	void Parse( Color &StringColor, const CColorLookup &ColorLookup ) const;

public:
	int m_iLength;

	enum {
		CDATATYPE_LOOKUP = 0,
		CDATATYPE_UNIQUE
	};

	int m_iDataType;

	union {
		int m_iLookupID;
		color32 m_Color;	// PNOTE: 'Color' has copy constructors
	} m_Data;
};

//=========================================================
//=========================================================
class CColoredString : public IColoredString
{
public:
	void Add( const char *pszMessage );
	void Add( const char *pszMessage, int iLookupID );
	void Add( const char *pszMessage, const Color &StringColor );

	void Draw( vgui::RichText *pRichText, const CColorLookup &ColorLookup );

private:
	CUtlVector< CMarkerColor > m_ColorMarkers;
};

//=========================================================
//=========================================================
#else

//=========================================================
//=========================================================
class CColoredString : public IColoredString
{
public:
	void Add( const char *pszMessage );
	inline void Add( const char *pszMessage, int iLookupID );
	inline void Add( const char *pszMessage, const Color &StringColor );
};

void CColoredString::Add( const char *pszMessage, int iColorLookupID )
{
	Add( pszMessage );
}

void CColoredString::Add( const char *pszMessage, const Color &StringColor )
{
	Add( pszMessage );
}

#endif

#endif // INS_COLOREDSTRING_H