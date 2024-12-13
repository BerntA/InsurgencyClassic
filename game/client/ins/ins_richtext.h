//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef INS_RICHTEXT_H
#define INS_RICHTEXT_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/richtext.h"

//=========================================================
//=========================================================
class CINSRichText : public vgui::RichText
{
	DECLARE_CLASS_SIMPLE( CINSRichText, vgui::RichText );

public:
	CINSRichText( Panel *pParent, const char *pszPanelName );

	void ClearText( void );

	bool HasMultipleLines( void );

	virtual int CalcTextWidth( void );
};

#endif // INS_RICHTEXT_H