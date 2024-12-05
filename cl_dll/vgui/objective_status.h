//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OBJECTIVE_STATUS_H
#define OBJECTIVE_STATUS_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/panel.h>
#include "vgui_basepanel.h"
#include "vgui_controls/frame.h"
#include <vgui_controls/textentry.h>
#include <vgui_controls/richtext.h>

namespace vgui
{
	class IScheme;
};

#define OBJLINE_FADE_TIME 1.0f
#define OBJLINE_TIME 6.0f

//-----------------------------------------------------------------------------
// Display Line for the Objective Status
//-----------------------------------------------------------------------------
class CObjectiveStatusLine : public vgui::RichText
{
	typedef vgui::RichText BaseClass;

public:
	CObjectiveStatusLine(vgui::Panel *parent, const char *panelName);

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	int GetCount(void) { return m_nCount; }
	float GetStartTime(void) { return m_flStartTime; }

	void SetExpireTime(void);
	bool IsReadyToExpire(void);
	void Expire(void);

	vgui::HFont GetFont() { return m_hFont; }

	Color GetTextColor( void ) { return m_clrText; }
	void SetNameLength( int iLength ) { m_iNameLength = iLength; }
	void SetNameColor( Color cColor ){ m_clrNameColor = cColor; }
		
	void PerformFadeout(void);

private:
	CObjectiveStatusLine(const CObjectiveStatusLine &); // damn those copy constructors!

	int	m_iNameLength;
	vgui::HFont	m_hFont;

	Color m_clrText;
	Color m_clrNameColor;

	float m_flExpireTime;
	
	float m_flStartTime;
	int m_nCount;
};

//-----------------------------------------------------------------------------
// Objective Status 
//-----------------------------------------------------------------------------
#define OBJ_MAX_LINES 6
#define OBJ_FADE_TIME OBJLINE_TIME + 3.0f

class CObjectiveStatus : public CHudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CObjectiveStatus, vgui::EditablePanel);

public:
	CObjectiveStatus(const char *pElementName);

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual void Init(void);
	void LevelInit(const char *newmap) { Clear(); }
	void LevelShutdown(void) { Clear(); }

	virtual void SetVisible(bool state);

	void MsgFunc_ObjMsg(bf_read &msg);
	void Print(class C_INSObjective *obj, const char *message);

	virtual void OnThink(void);

	static int m_nLineCounter;

private:
	CObjectiveStatusLine *FindUnusedLine(void);

	void ExpireOldest(void);

	void Reset(void);
	void Clear(void);

	CObjectiveStatusLine *m_ChatLines[OBJ_MAX_LINES];
	int	m_iFontHeight;

	int m_nVisibleHeight;

	float m_flStartTime;
	float m_flExpireTime;
};

#endif // OBJECTIVE_STATUS_H
