//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef INS_VGUI_H
#define INS_VGUI_H
#ifdef _WIN32
#pragma once
#endif

#include "baseviewport.h"
#include "squad_data.h"
#include "commander_shared.h"

//=========================================================
//=========================================================
class IViewPortPanel;
class CTeamLookup;
class C_PlayTeam;
class ICustomizeHelper;

//=========================================================
//=========================================================
enum VGUITeamPath_t
{
	VGUI_TEAMPATH_CLASS = 0,
	VGUI_TEAMPATH_ENDGAME,
	VGUI_TEAMPATH_RANKICONS,
	VGUI_TEAMPATH_COUNT
};

//=========================================================
//=========================================================
class CINSViewportHelper
{
	typedef IViewPortPanel *( *CreatePanel_t )( CBaseViewport *pViewport );

public:
	static CINSViewportHelper *m_sHelpers;

	static void CreateAllElements( CBaseViewport *pViewport );

public:
	CINSViewportHelper( CreatePanel_t pfnCreate );

	CINSViewportHelper *GetNext( void ) { return m_pNext; }

private:
	CINSViewportHelper *m_pNext;
	CreatePanel_t m_pfnCreate;
};

//=========================================================
//=========================================================
#define CREATE_INSVIEWPORT_PANEL( panel ) \
	IViewPortPanel *CreatePanel__##panel( CBaseViewport *pViewport ) { \
		return new panel( pViewport ); } \
	CINSViewportHelper g_Helper__##panel( CreatePanel__##panel );

//=========================================================
//=========================================================
#define INSVGUI_FINAL_CHANGETEAM
//#define INSVGUI_FINAL_SQUADSELECTION
//#define INSVGUI_FINAL_GAMEINFO
//#define INSVGUI_FINAL_DEATHINFO
//#define INSVGUI_FINAL_COMMANDER
//#define INSVGUI_FINAL_ENDGAME
//#define INSVGUI_FINAL_CUSTOMIZEGUI
//#define INSVGUI_FINAL_SCOREBOARD

//=========================================================
//=========================================================
enum INSMenus_t
{
	INSMENU_INVALID = 0,

#ifndef INSVGUI_FINAL_CHANGETEAM

	INSMENU_TEAM,

#endif

#ifndef INSVGUI_FINAL_SQUADSELECTION

	INSMENU_SQUADSELECTION,

#endif

	INSMENU_COUNT
};

//=========================================================
//=========================================================
class IINSMenu
{
public:
	virtual const char *GetTitle( void ) const = 0;
	virtual void Setup( void ) = 0;
	virtual bool Action( int iItemID ) = 0;
	virtual void Closed( void ) { }

	virtual bool IgnoreResetHUD( void ) const { return false; }
};

//=========================================================
//=========================================================
class IINSMenuManager
{
public:
	virtual void ResetMenu( void ) = 0;
	virtual void ResetItems( void ) = 0;

	virtual void ShowMenu( int iID ) = 0;
	virtual void UpdateMenu( void ) = 0;
	virtual void CloseMenu( void ) = 0;

	bool HasActiveMenu( void ) const;
	virtual int GetActiveMenu( void ) const = 0;

	virtual int AddSection( int iSectionID, const char *pszName ) = 0;
	virtual int AddItem( int iSectionID, const char *pszName, const char *pszExtText, bool bHasAction ) = 0;
	virtual int GetSection( int iItemID ) = 0;
};

//=========================================================
//=========================================================
typedef IINSMenu *( *MenuCreatorHelper_t )( void );

class CINSMenuHelper
{
public:
	CINSMenuHelper( int iID, MenuCreatorHelper_t Helper );

public:
	CINSMenuHelper *m_pNext;

	int m_iID;
	MenuCreatorHelper_t m_Helper;
};

#define DECLARE_INSMENU( id, classname ) \
	IINSMenu *CreateINSMenu__##id( void ) { \
	return new classname; } \
	CINSMenuHelper g_INSMenuHelper__##id( id, CreateINSMenu__##id );

//=========================================================
//=========================================================
class IINSSquadSelection
{
public:
	IINSSquadSelection( );

	virtual void TeamUpdate( int iTeamID ) = 0;
	virtual void SquadUpdate( void ) = 0;
};

//=========================================================
//=========================================================

// PNOTE: deathinfo being full could change, so it's stored
// as bool inside the struct to be safe
struct DeathInfoData_t
{
	int m_iType;
	int m_iAttackerID;

	bool m_bDeathInfoFull;

	int m_iDistance;
	int m_iInflictorType;
	int m_iInflictorID;

	int m_iDamageType;
};

class IINSDeathInfo
{
public:
	IINSDeathInfo( );

	virtual void ShowDeathInfo( void ) = 0;
	virtual void UpdateDeathInfo( void ) = 0;
};

//=========================================================
//=========================================================
class CINSVGUIHelper : public IGameEventListener2
{
public:
	~CINSVGUIHelper( );

	static CINSVGUIHelper *GetINSVGUIHelper( void );

	void Init( void );

	// Registering
	void RegisterMenuManager( IINSMenuManager *pMenuManager );
	void RegisterSquadSelection( IINSSquadSelection *pSquadSelection );
	void RegisterDeathInfo( IINSDeathInfo *pDeathInfo );

	// Generic Helpers
	static bool IsGameReady( void );
	static const char *GetTeamName( int iTeamID );
	static bool IsGameRunning( void );

	// Showing and Hiding VGUIs
	bool CanShowTeamPanel( void );
	bool CanShowSquadPanel( void );
	bool ShowDeathPanel( void );

	// Server Messages
	static void JoinFull( int iTeamSelectID, EncodedSquadData_t &EncodedSquadData );
	static void JoinTeam( int iTeamSelectID );
	static void JoinSquad( EncodedSquadData_t &EncodedSquadData, bool bWhenDie );

	// Menu Manager
	IINSMenuManager *GetMenuManager( void ) const;
	IINSMenu *GetMenu( int iID ) const;

	// Death Info
	void DeathInfoMsg( bf_read &Msg );
	bool IsValidDeathData( void ) const { return m_bValidDeathData; }
	const DeathInfoData_t &GetDeathData( void ) const { return m_DeathInfo; }
	void FinishDeathInfo( void );

	// Customization
	ICustomizeHelper *GetCustomizeHelper( void );

	// Misc
	void MassHide( void );

	bool CreateTeamPath( CTeamLookup *pTeam, int iType, char *pszBuffer, int iLength );
	bool CreateTeamPath( C_PlayTeam *pTeam, int iType, char *pszBuffer, int iLength );

private:
	void FireGameEvent( IGameEvent *pEvent );

private:
	CINSVGUIHelper( );

private:
	IINSMenuManager *m_pMenuManager;
	CUtlMap< int, IINSMenu* > m_Menus;

	IINSSquadSelection *m_pSquadSelection;

	IINSDeathInfo *m_pDeathInfo;
	DeathInfoData_t m_DeathInfo;
	bool m_bValidDeathData;
};

//=========================================================
//=========================================================
inline CINSVGUIHelper *GetINSVGUIHelper( void )
{
	return CINSVGUIHelper::GetINSVGUIHelper( );
}

#endif // INS_VGUI_H
