//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef VGUI_CUSTOMIZE_H
#define VGUI_CUSTOMIZE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/insframe.h>
#include <cl_dll/iviewport.h>
#include <vgui/ischeme.h>

#include <vgui_controls/button.h>
#include <vgui_controls/combobox.h>
#include <vgui_controls/richtext.h>
#include <vgui_controls/label.h>

typedef struct modelcustomization_s modelcustomization_t;

class CCustomizeGUI : public vgui::INSFrame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CCustomizeGUI, vgui::INSFrame);
	class CModelPanel;
	class EventComboBox;
	friend class CModelPanel;

public:
	CCustomizeGUI(IViewPort *pViewPort);

	virtual const char *GetName(void) { return PANEL_CUSTOMIZEGUI; }
	virtual void SetData(KeyValues *data) { };
	virtual void Update() { }
	virtual bool NeedsUpdate() { return false; };
	virtual bool HasInputElements(void) { return true; }
	virtual void Reset();
	virtual void ShowPanel(bool bShow);
	
	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); }

protected:
	virtual void PerformLayout(void);
	virtual void OnCommand(const char *pszCommand);
	virtual void RequestFocus(int direction = 0);

	void Setup(void);
	void SetupModel(void);

private:
	MESSAGE_FUNC_PTR( OnMenuItemSelected, "MenuItemSelected", panel );
	void PaintModel(CModelPanel* m_pModelPanel);
	void LocalSetBodygroup(int iGroup);

	// Pongles [
	bool SendToServer(void);
	// Pongles ]

private:
	const model_t		*m_pModel;
	studiohdr_t			*m_pStuHdr;
	modelcustomization_t*m_pMdlCust;
	int					m_nBody;
	int					m_nSkin;
	bool				m_bResetModel;
	//HACK HACK. when this is set, dont change the text in the
	//description panel until a submodels is chosen once.
	//This ensures that the hint is visible after popping up
	//the customization screen :)
	bool				m_bOverrideText;

	CModelPanel			*m_pModelPanel;
	EventComboBox		*m_pModels;
	EventComboBox		*m_pSkins;
	EventComboBox		*m_pBodygroups;
	EventComboBox		*m_pSubmodels;
	vgui::RichText		*m_pDescription;
	vgui::Button		*m_apButtons[3];
	vgui::Label			*m_pMessage;
};

#endif // VGUI_CUSTOMIZE_H
