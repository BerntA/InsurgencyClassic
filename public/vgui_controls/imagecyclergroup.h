//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef IMAGECYCLERGROUP_H
#define IMAGECYCLERGROUP_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/imagecycler.h>

namespace vgui
{

	// ------------------------------------------------------------------------------------ //
	// Cycles Images in a Specified Group
	// ------------------------------------------------------------------------------------ //
	class ImageCyclerGroup : public Panel
	{
		DECLARE_CLASS_SIMPLE(ImageCyclerGroup, Panel);

	public:
		ImageCyclerGroup(Panel* pParent, const char* pszName);

		void Reset(void);
		void ResetImages(void);

		void AddGroup(const char* pszPath);
		void SetActiveGroup(int iIndex);

	protected:
		virtual void ApplySettings(KeyValues* inResourceData);
		virtual void OnSizeChanged(int iWide, int iTall);

	private:
		void ResizeCycler(ImageCycler* pImageCycler, int iWide, int iTall);

	private:
		bool m_bEmbedded;
		bool m_bFadeImages;
		float m_flHoldTime, m_flFadeTime;

		CUtlVector<ImageCycler*> m_ImageCyclers;
		//ImageCycler *m_pBlank;

		int m_iActiveGroup;
	};

} // namespace vgui

#endif // IMAGECYCLERGROUP_H