//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GAMEUIPANEL_H
#define GAMEUIPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/vgui.h>
#include <vgui_controls/panel.h>

//=========================================================
//=========================================================
class INSLoadingDialog;

//=========================================================
//=========================================================
class CLoadElementBase
{
public:
	virtual void Load( vgui::VPANEL Panel ) = 0;
	virtual void Apply( vgui::VPANEL Panel ) const = 0;
};

class CLoadElementNothing : public CLoadElementBase
{
public:
	virtual void Load( vgui::VPANEL Panel ) {};
	virtual void Apply( vgui::VPANEL Panel ) const {};
};

class CLoadElementDimensions : public CLoadElementBase
{
public:
	CLoadElementDimensions( );

	void Init( int iXPos, int iYPos, int iWide, int iTall );

	void Load( vgui::VPANEL Panel );
	void Apply( vgui::VPANEL Panel ) const;

public:
	int m_iWide, m_iTall;
	int m_iXPos, m_iYPos;
};

enum LoadElementTypes_t
{
	LOADELEMENT_PARENT = 0,
	LOADELEMENT_PROGRESS,
	LOADELEMENT_CANCEL,
	LOADELEMENT_INFO,
	LOADELEMENT_COUNT
};

class CLoadElements
{
public:
	CLoadElementBase &Element( int iID );

public:
	CLoadElementDimensions m_Parent, m_Progress;
	CLoadElementNothing m_Cancel, m_Info;
};

class CLoadDialog
{
public:
	CLoadDialog( );

	bool IsValid( void ) const;

	void Init( vgui::VPANEL Parent );

	void Reset( void );
	void ResetElements( void );

	bool IsModified( void ) const { return m_bModified; }
	void Apply( CLoadElements &Elements );

	const CLoadElements &DefaultElements( void ) const { return m_DefaultElements; }
	vgui::VPANEL GetPanel( int iID ) const;

private:
	void ResetPanels( void );

	void SaveElements( void );

	void Apply( CLoadElements &Elements, bool bDefault );
	

private:
	bool m_bSavedElements;
	bool m_bModified;

	vgui::VPANEL m_Panels[ LOADELEMENT_COUNT ];
	CLoadElements m_DefaultElements;
};

//=========================================================
//=========================================================
class CGameUIPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CGameUIPanel, vgui::Panel );

public:
	CGameUIPanel( vgui::VPANEL parent );

	CLoadDialog &GetLoadDialog( void ) { return m_LoadDialog; }

	void DeleteLoadPanel();

	void WaitingJoinGame( void ) { m_bWaitingJoinGame = true; m_bWaitingShowPanel = true; };

	void MOTDInit( void );
	void JoinServer( void );

private:
	void OnThink( void );
	void OnTick( void );
	void OnCommand( const char *pszCommand );

	void LoadingDialogThink( void );

	void ResetButtons( void );
	void HackButton( Panel *pButton, vgui::VPANEL Parent, const char *pszChildName );
	void FindOptions( void );
	vgui::VPANEL FindOption( vgui::VPANEL Parent, const char *pszUniqueElement );

private:
	vgui::VPANEL m_BaseGameUIPanel;
	int m_iLastUIChildCount;

	bool m_bFoundOptions;
	vgui::VPANEL m_OptionsDialog;
	vgui::VPANEL m_OptionsMultiplayer, m_OptionsMouse, m_OptionsAudio;
	vgui::Button *m_pStats, *m_pINS, *m_pTrackIR, *m_pMusic;

	CLoadDialog m_LoadDialog;
	INSLoadingDialog *m_pLoadPanel;

	bool m_bWaitingJoinGame;
	bool m_bWaitingShowPanel;
};

//=========================================================
//=========================================================
extern CGameUIPanel *g_pGameUIPanel;
extern CGameUIPanel *GameUIPanel( void );

#endif // GAMEUIPANEL_H