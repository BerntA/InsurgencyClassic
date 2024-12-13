//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef INS_OPTIONFIELD_H
#define INS_OPTIONFIELD_H
#ifdef _WIN32
#pragma once
#endif

#include <convar.h>
#include <vgui_controls/frame.h>
#include <vgui_controls/checkbutton.h>
#include <vgui_controls/combobox.h>

//=========================================================
//=========================================================
class IINSOptionField
{
public:
	IINSOptionField( ConVar *pOption );

	virtual void UpdateOption( void ) = 0;
	virtual void UpdateControl( void ) = 0;

protected:
	ConVar *m_pOption;
};

//=========================================================
//=========================================================
class CINSOptionFrame : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CINSOptionFrame, vgui::Frame );

public:
	CINSOptionFrame( const char *pszName );
	~CINSOptionFrame( );

	virtual void Activate( void );

	void RegisterOption( ConVar *pOption, vgui::CheckButton *pControl );
	void RegisterOption( ConVar *pOption, vgui::ComboBox *pControl );

protected:
	void OnCommand( const char *pszCommand );

private:
	CUtlVector< IINSOptionField* > m_Options;
};

#endif // INS_OPTIONFIELD_H

