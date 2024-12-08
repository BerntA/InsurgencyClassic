//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSTEXTWINDOW_H
#define CSTEXTWINDOW_H
#ifdef _WIN32
#pragma once
#endif

#include "vguitextwindow.h"

//-----------------------------------------------------------------------------
// Purpose: displays the MOTD
//-----------------------------------------------------------------------------

class CHL2MPTextWindow : public CTextWindow
{
private:
	DECLARE_CLASS_SIMPLE( CHL2MPTextWindow, CTextWindow );

public:
	CHL2MPTextWindow(IViewPort *pViewPort);
	virtual ~CHL2MPTextWindow();

	virtual void Update();
	virtual void SetVisible(bool state);
	virtual void ShowPanel( bool bShow );
	virtual void OnKeyCodePressed(vgui::KeyCode code);

	virtual void PaintBackground();
	virtual void PerformLayout();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

#endif // CSTEXTWINDOW_H