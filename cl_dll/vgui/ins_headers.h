//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_INSHEADERS_H
#define VGUI_INSHEADERS_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/insframe.h>

//-----------------------------------------------------------------------------
// Purpose: Insurgency Logo and Version
//-----------------------------------------------------------------------------
class CINSVersion;

class CINSHeader : public vgui::INSFrameHeader
{
	DECLARE_CLASS_SIMPLE(CINSHeader, INSFrameHeader);

public:
	CINSHeader(vgui::INSFrame *pParent, const char *pszName);
	virtual ~CINSHeader();

private:
	virtual bool UseDefaultTall(void) const { return false; }

private:
	CINSVersion *m_pINSVersion;
};

//-----------------------------------------------------------------------------
// Purpose: Server Details Header
//-----------------------------------------------------------------------------
class CServerHeader : public vgui::INSTextHeader
{
	DECLARE_CLASS_SIMPLE(CServerHeader, INSTextHeader);

public:
	CServerHeader(vgui::INSFrame *pParent, const char *pszName);

	virtual void Reset(void);

protected:
	virtual void PerformLayout(void);
	virtual void OnTick(void);
	virtual void OnVisibilityChange(int iOldVisible);

private:
	void Update(void);
	bool ShouldUpdate(void);

private:
	int m_iCurrentPlayers;
};

//-----------------------------------------------------------------------------
// Purpose: Basic INS Header
//-----------------------------------------------------------------------------
#define CreateBasicINSHeader(name) \
class C##name##Header : public INSTopHeader \
	{ \
	public: \
		DECLARE_CLASS_SIMPLE(C##name##Header, INSTopHeader); \
		C##name##Header(INSFrame *pParent); \
	private: \
		virtual bool UseDefaultTall(void) const { return false; } \
	}; \
	C##name##Header::C##name##Header(INSFrame *pParent) \
	: INSTopHeader(pParent, #name "Header") \
	{ \
	LoadControlSettings("Resource/UI/Frames/Headers/" #name ".res"); \
	}

#endif // VGUI_INSHEADERS_H