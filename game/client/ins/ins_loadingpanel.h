//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef INS_LOADINGPANEL_H
#define INS_LOADINGPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/ipanel.h>
#include <vgui/ischeme.h>
#include "ins_panel.h"
#include "gameuipanel.h"
#include "insvgui_utils.h"
#include <string>
#include <map>
using namespace std;

//=========================================================
//=========================================================
class INSLoadingDialog : public CINSPanel
{
	DECLARE_CLASS_SIMPLE( INSLoadingDialog, CINSPanel );

public:
	INSLoadingDialog();
	~INSLoadingDialog();

	void MOTDInit( void );

	void LoadingUpdate( void );
	void LoadedUpdate( void );

	const CLoadElements &GetModifiedDimensions( void ) const;

	void OnCommand( const char *pszCommand );

	void PerformLayout();

	void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	void Paint( void );
	void PaintBackground();

	void Setup( void );
	void SetupContents();
	void TranslateLoadDimensions( void );

private:
	CLoadElements m_ModifiedElements;

	float m_flFadeTime;
	bool m_bFading;

	typedef map< int, string > TipList;

	TipList m_tips;

	vgui::Panel* m_pMapImage;
	vgui::Label* m_pLoadingStatus;
	vgui::Label* m_pServerInfo;
	vgui::Label* m_pMapDesc;
	vgui::Label* m_pGameType;
	vgui::Label* m_pTipText;
	vgui::Label* m_pGameTypeName;

	bool m_bInit; // hack

	int m_iMapImage;
	char m_szMapImage[MAX_PATH];
};


#endif // INS_LOADINGPANEL_H