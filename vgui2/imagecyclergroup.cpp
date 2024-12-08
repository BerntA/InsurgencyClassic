//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <vgui_controls/imagecyclergroup.h>
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//=========================================================
//=========================================================
#define INVALID_GROUP -1

Color g_NoGroupColor = Color(0, 0, 0, 255);

//=========================================================
//=========================================================
ImageCyclerGroup::ImageCyclerGroup(Panel* pParent, const char* pszName)
	: Panel(pParent, pszName)
{
	SetScheme("ClientScheme");

	SetBgColor(g_NoGroupColor);

	Reset();
	ResetImages();
}

//=========================================================
//=========================================================
void ImageCyclerGroup::Reset(void)
{
	m_bEmbedded = false;
	m_bFadeImages = false;
	m_flHoldTime = DEFAULT_HOLD_TIME;
	m_flFadeTime = DEFAULT_FADE_TIME;
}

//=========================================================
//=========================================================
void ImageCyclerGroup::ResetImages(void)
{
	m_ImageCyclers.PurgeAndDeleteElements();

	m_iActiveGroup = INVALID_GROUP;
}

//=========================================================
//=========================================================
void ImageCyclerGroup::AddGroup(const char* pszPath)
{
	// add cycler
	ImageCycler* pImageCycler = new ImageCycler(this, NULL);

	// resize cycler
	int iWide, iTall;
	GetSize(iWide, iTall);
	ResizeCycler(pImageCycler, iWide, iTall);

	// setup images
	pImageCycler->SetVisible(false);
	pImageCycler->SetUseEmbedded(m_bEmbedded);
	pImageCycler->SetFadeImages(m_bFadeImages);
	pImageCycler->SetHoldTime(m_flHoldTime);
	pImageCycler->SetFadeTime(m_flFadeTime);

	pImageCycler->AddImages(pszPath);

	// add cycler
	m_ImageCyclers.AddToTail(pImageCycler);
}

//=========================================================
//=========================================================
void ImageCyclerGroup::SetActiveGroup(int iIndex)
{
	// hide old cycler
	ImageCycler* pActiveCycler;

	if (m_iActiveGroup != INVALID_GROUP)
	{
		pActiveCycler = m_ImageCyclers[m_iActiveGroup];
		pActiveCycler->ResetActiveImage();
		pActiveCycler->SetVisible(false);
	}

	// setup new cycler
	pActiveCycler = m_ImageCyclers[iIndex];

	if (pActiveCycler->GetImageCount() == 0)
		return;

	pActiveCycler->ResetActiveImage();
	pActiveCycler->SetVisible(true);
	pActiveCycler->SetupImages();
	pActiveCycler->MoveToFront();

	// update active group
	m_iActiveGroup = iIndex;
}

//=========================================================
//=========================================================
void ImageCyclerGroup::ApplySettings(KeyValues* inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	m_bEmbedded = inResourceData->GetInt("embedded", 0);

	m_bFadeImages = (inResourceData->GetInt("fadeimages", 0) ? true : false);
	m_flHoldTime = inResourceData->GetFloat("holdtime", DEFAULT_HOLD_TIME);
	m_flFadeTime = inResourceData->GetFloat("fadetime", DEFAULT_FADE_TIME);
}

//=========================================================
//=========================================================
void ImageCyclerGroup::OnSizeChanged(int iWide, int iTall)
{
	BaseClass::OnSizeChanged(iWide, iTall);

	for (int i = 0; i < m_ImageCyclers.Count(); i++)
		ResizeCycler(m_ImageCyclers[i], iWide, iTall);
}

//=========================================================
//=========================================================
void ImageCyclerGroup::ResizeCycler(ImageCycler* pImageCycler, int iWide, int iTall)
{
	pImageCycler->SetSize(iWide, iTall);
}