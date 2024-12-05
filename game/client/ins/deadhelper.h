//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef DEADPANELHELPER_H
#define DEADPANELHELPER_H
#ifdef _WIN32
#pragma once
#endif

#include "insvgui_utils.h"

//=========================================================
//=========================================================
namespace vgui {
	class EditablePanel;
};

class KeyValues;

//=========================================================
//=========================================================
class CDeadHUDHelper
{
public:
	CDeadHUDHelper( vgui::EditablePanel *pParent );

protected:
	void DeadInit( KeyValues *pResourceData );
	void DeadUpdate( void );

	virtual void OnDeadSize( void ) { }
	
private:
	vgui::EditablePanel *m_pParent;

	bool m_bIsDead;

	int m_iDeadXPos, m_iDeadYPos;
	int m_iRestoreXPos, m_iRestoreYPos;
};

#endif // DEADPANELHELPER_H
