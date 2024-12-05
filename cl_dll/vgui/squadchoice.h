//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_SQUADCHOICE_H
#define VGUI_SQUADCHOICE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/editablepanel.h>

namespace vgui
{
	class CPlayerOrder;
};

class CSquadSetup;


// ------------------------------------------------------------------------------------ //
// Squad Choices
// ------------------------------------------------------------------------------------ //
class CSquadChoice : public vgui::EditablePanel
{
private:
	DECLARE_CLASS_SIMPLE(CSquadChoice, vgui::EditablePanel);

public:
	CSquadChoice(vgui::Panel *pParent);

	virtual void RequestFocus(int direction = 0);
	void SetProcessState(bool bState);
	bool ShouldUpdate(void);
	MESSAGE_FUNC(OnResetData, "ResetData");

private:
	CSquadSetup *m_pSquadSetup;

	vgui::CPlayerOrder *m_pPlayerOrder;
};


#endif // VGUI_SQUADCHOICE_H
