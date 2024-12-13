//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_EMBEDDEDPANEL_H
#define VGUI_EMBEDDEDPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>

namespace vgui
{
	class ImagePanel;

	// ------------------------------------------------------------------------------------ //
	// EmbeddedImage to avoid ^2 Problems
	// ------------------------------------------------------------------------------------ //
	class EmbeddedImage : public EditablePanel
	{
	private:
		DECLARE_CLASS_SIMPLE(EmbeddedImage, EditablePanel);

	public:
		EmbeddedImage(Panel* pParent, const char* pszPanelName);

		void SetImage(const char* pszPath);

	protected:
		virtual void ApplySettings(KeyValues* inResourceData);
		virtual void OnSizeChanged(int iWide, int iTall);

	private:
		int FindPower2Size(int iSize);

	private:
		ImagePanel* m_pImage;
	};
} // namespace vgui

#endif // VGUI_EMBEDDEDPANEL_H