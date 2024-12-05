//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_COMMS_H
#define HUD_COMMS_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/panel.h>

#include "inshud.h"

//=========================================================
//=========================================================
class CHUDComms;

//=========================================================
//=========================================================
#define COMMS_OPTION_LENGTH 32

struct CommsTex_t
{
	CommsTex_t( );

	int m_iSelectedID, m_iUnselectedID;
};

struct CommsOption_t
{
	wchar_t m_wszName[ COMMS_OPTION_LENGTH ];
	int m_iNameLength;
};

struct CommsGroup_t
{
	void AddOption( const char *pszName, bool bAddQuotes );

	int m_iGroupID;
	CommsTex_t m_Icon;
	CUtlVector< CommsOption_t > m_Options;
};

//=========================================================
//=========================================================
enum ComGroups_t
{
	COMGROUP_REINFORCE = 0,
	COMGROUP_OBJORDER,
	COMGROUP_UNITORDER,
	COMGROUP_PORDER,
	COMGROUP_POREPONSE,
	COMGROUP_STATUS,
	COMGROUP_NEEDHELP,
	COMGROUP_COUNT
};

//=========================================================
//=========================================================
class ICommsGroup
{
public:
	void Init( CHUDComms *pParent );
	virtual void SetupIcons( void ) = 0;

	virtual void Reset( void ) { }

	virtual bool IsVisible( void ) const { return true; }
	virtual void SetupGroup( CommsGroup_t &Group ) = 0;

	virtual void Selected( int iID ) = 0;

protected:
	void LoadIcon( const char *pszName, CommsTex_t &IconTex );

private:
	CHUDComms *m_pParent;
};

//=========================================================
//=========================================================
class CHUDComms : public CHudElement, public vgui::Panel, public IINSControlListener, public IINSPlayerDeath
{
	DECLARE_CLASS_SIMPLE( CHUDComms, vgui::Panel );

public:
	CHUDComms( const char *pszElementName );
	~CHUDComms( );

	void Toggle( void );
	void Activate( void );
	void Close( void );

	bool HasValidSelection( void ) const;

	void LoadTex( const char *pszName, CommsTex_t &CommsTex );

private:
	void Init( void );
	void LevelInit( void );
	void Reset( void );

	void ApplySchemeSettings( vgui::IScheme *pScheme );

	void Rebuild( void );

	bool ShouldDraw( void );
	void Paint( void );

	void DrawGroup( const CommsTex_t &Icon, const CommsOption_t &Option, bool bSelected, int iYPos );
	void DrawOption( const CommsOption_t &Option, bool bSelected, int iYPos );
	void DrawOptionText( const CommsOption_t &Option, bool bSelected, int iYPos );
	void Draw( const CommsTex_t &Tex, bool bSelected, int iYPos );

	bool IsControlActive( void );
	void DoControlClose( void );
	void Selection( void );
	void Scroll( int iType );

	void PlayerDeath( void );

	void LoadTex( const char *pszName, CommsTex_t &CommsTex, bool bSelection );

	void FindPath( const char *pszName, bool bSelected, char *pszBuffer, int iLength );

private:
	ICommsGroup *m_pGroups[ COMGROUP_COUNT ];
	bool m_bIconsLoaded;

	CommsTex_t m_BGHeader, m_BGSub;
	int m_iBGWide, m_iBGTall, m_iBGYGapSection, m_iBGYGapOption;

	vgui::HFont m_Font;
	int m_iTextXPos, m_iTextYPos;

	int m_iStartYPos;

	CUtlVector< CommsGroup_t > m_Groups;
	int m_iOptionCount;

	int m_iSelectedID;
};

//=========================================================
//=========================================================
typedef ICommsGroup *( *CommsGroupCreator_t )( void );

extern CommsGroupCreator_t g_CommsGroupHelpers[ COMGROUP_COUNT ];

class CCommsGroupHelper
{
public:
	CCommsGroupHelper( int iID, CommsGroupCreator_t CommsGroupCreator )
	{
		g_CommsGroupHelpers[ iID ] = CommsGroupCreator;
	}
};

#define DECLARE_COMGROUP( id, commsheader ) \
	ICommsGroup *CreateCommsGroup__##id( void ) { \
		return new commsheader; } \
	CCommsGroupHelper g_CommsGroupHelper__##id( id, CreateCommsGroup__##id );

#endif // HUD_COMMS_H