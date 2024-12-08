//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_INSFRAME_H
#define VGUI_INSFRAME_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/FocusNavGroup.h>

namespace vgui
{
	class INSFrameHeader;
	class INSFrameClientBackground;

	//-----------------------------------------------------------------------------
	// Purpose: Insurgency Windowed frame
	//-----------------------------------------------------------------------------
	class INSFrame : public EditablePanel
	{
		DECLARE_CLASS_SIMPLE(INSFrame, EditablePanel);

	public:
		INSFrame(Panel* pParent, const char* pszPanelName);
		virtual ~INSFrame();

		void AddHeader(INSFrameHeader* pTitle);

		int AddButton(const char* pszTitle, const char* pszCommand, bool bDefault = false);
		Button* GetButton(int iID);

		virtual void EnableQuickClose(bool bState);

		virtual void Activate(void);
		MESSAGE_FUNC(Close, "Close");

		Panel* GetClientArea(void) const;

	protected:
		virtual void OnMousePressed(MouseCode Code);
		virtual void OnKeyCodePressed(KeyCode Code);
		virtual void OnKeyCodeTyped(KeyCode Code);
		virtual void OnKeyTyped(wchar_t UniChar);
		virtual void OnKeyCodeReleased(KeyCode code);
		virtual void OnKeyFocusTicked();

		virtual void LoadControlSettings(const char* dialogResourceName, const char* pathID = NULL);
		virtual void ApplySettings(KeyValues* inResourceData);
		virtual const char* GetDescription(void);

		virtual bool GetDefaultScreenPosition(int& x, int& y, int& wide, int& tall);

		virtual void PerformLayout(void);

		virtual void OnClose(void);
		virtual void OnScreenSizeChanged(int iOldWide, int iOldTall);

	private:
		void MoveToCenterOfScreen(void);

		void RecalculateAreas(void);

	private:
		int m_iTitleGap;

		bool m_bQuickClose;

		CUtlVector<INSFrameHeader*> m_Headers;

		INSFrameClientBackground* m_pClientBackground;
		Panel* m_pClientArea;

		CUtlVector<Button*> m_Buttons;
	};

	//-----------------------------------------------------------------------------
	// Purpose: Insurgency Frame Header
	//-----------------------------------------------------------------------------
#define DEFAULT_FRAMEHEADER_HEIGHT 17

	class INSFrameHeader : public EditablePanel
	{
		DECLARE_CLASS_SIMPLE(INSFrameHeader, EditablePanel);

	public:
		INSFrameHeader(INSFrame* pParent, const char* pszName, int iTall = DEFAULT_FRAMEHEADER_HEIGHT);

		virtual void ApplySchemeSettings(IScheme* pScheme);
		virtual void ApplySettings(KeyValues* inResourceData);

		virtual void SetupSize(void);

		void SetOffset(int iOffset);
		int GetOffset(void) const { return m_iOffset; }

	private:
		virtual bool UseDefaultTall(void) const { return true; }

	private:
		int m_iOffset;
	};

	//-----------------------------------------------------------------------------
	// Purpose: Common Insurgency Frame Header
	//-----------------------------------------------------------------------------
#define MAX_TEXTHEADER_LENGTH 512

	class INSTextHeader : public INSFrameHeader
	{
		DECLARE_CLASS_SIMPLE(INSTextHeader, INSFrameHeader);

	public:
		INSTextHeader(INSFrame* pParent, const char* pszName);

		virtual void Reset(void);
		void SetText(const char* pszText);

	protected:
		virtual void ApplySchemeSettings(IScheme* pScheme);

		virtual void Paint(void);

	private:
		HFont m_hFont;
		Color m_TextColor;
		wchar_t m_szText[MAX_TEXTHEADER_LENGTH];
		int m_iTextLength;

		int m_iStringXPos, m_iStringYPos;

		bool m_bNeedsUpdate;
	};

	//-----------------------------------------------------------------------------
	// Purpose: Common Insurgency Frame Header
	//-----------------------------------------------------------------------------
	class INSTopHeader : public INSFrameHeader
	{
		DECLARE_CLASS_SIMPLE(INSTopHeader, INSFrameHeader);

	public:
		INSTopHeader(INSFrame* pParent, const char* pszName);
	};

} // namespace vgui

#endif // VGUI_FRAME_H