//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef INS_VIEWPORT_H
#define INS_VIEWPORT_H

#include "baseviewport.h"

//=========================================================
//=========================================================
class INSViewport : public CBaseViewport, public CAutoGameSystem
{
private:
	DECLARE_CLASS_SIMPLE( INSViewport, CBaseViewport );

public:
	void LevelInitPostEntity( void );

	void CreateDefaultPanels( void );

	void ApplySchemeSettings( vgui::IScheme *pScheme );

	void HideAllPanels( void );

	void OnScreenSizeChanged( int iOldWide, int iOldTall );
};

#endif // INS_VIEWPORT_H
