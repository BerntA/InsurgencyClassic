//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <vgui/isurface.h>
#include <vgui/ischeme.h>
#include <vgui/isystem.h>
#include <vgui/iborder.h>
#include <keyvalues.h>

#include <vgui_controls/animationcontroller.h>
#include <vgui_controls/imagecycler.h>
#include <vgui_controls/imagepanel.h>
#include <vgui_controls/embeddedimage.h>
#include <vgui_controls/vgui_helper.h>

#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//=========================================================
//=========================================================
#define INVALID_IMAGE -1

Color g_DefaultColor(0, 0, 0, 255);
Color g_FadeColor(255, 255, 255, 255);

//=========================================================
//=========================================================
ImageCycler::ImageCycler(Panel *pParent, const char *pszName)
	: Panel(pParent, pszName)
{
	m_bAllocated = false;

	m_pImages = NULL;
	m_pEmbeddedImages = NULL;

	// allocate image list
	SetUseEmbedded(false, true);

	// setup variables
	SetFadeImages(false);
	SetHoldTime(DEFAULT_HOLD_TIME);
	SetFadeTime(DEFAULT_FADE_TIME);

	// reset
	Reset();
}

//=========================================================
//=========================================================
ImageCycler::~ImageCycler()
{
	// destroy images
	DestroyImageList();
}

//=========================================================
//=========================================================
void ImageCycler::Reset(void)
{
	// clear images
	ClearImages();
	m_iActiveImage = INVALID_IMAGE;

	// draw blank
	DrawCycler = &ImageCycler::DrawBlank;

	// reset fading
	ResetFade();

	// don't setup images yet
	m_flSetupImage = 0.0f;
}

//=========================================================
//=========================================================
void ImageCycler::ResetActiveImage(void)
{
	GetAnimationController()->CancelAllAnimations();

	m_iActiveImage = GetLastImageID();
	ResetFade();

	SetupImages(false);
}

//=========================================================
//=========================================================
void ImageCycler::SetUseEmbedded(bool bState, bool bInt)
{
	if(!bInt)
	{
		if(m_bEmbeddedImages == bState)
			return;

		DestroyImageList();
	}

	m_bEmbeddedImages = bState;
	AddImage = (m_bEmbeddedImages ? &ImageCycler::AddEmbeddedImage : &ImageCycler::AddImagePanel);

	AllocateImageList();
}

//=========================================================
//=========================================================
void ImageCycler::SetFadeImages(bool bFadeImages)
{
	m_bFadeImages = bFadeImages;

	if(!bFadeImages)
		ResetFade();
}

//=========================================================
//=========================================================
void ImageCycler::Think(void)
{
	if(m_flSetupImage == 0.0f || m_flSetupImage >= system()->GetCurrentTime())
		return;

	SetupImages(false);
	m_flSetupImage = 0.0f;
}

//=========================================================
//=========================================================
void ImageCycler::AddImages(const char *pszPath)
{
	// collect images
	char szImagePath[_MAX_PATH];

	CheckCycleMaterialsExists(pszPath, &m_ValidImages);

	for(int i = 0; i < m_ValidImages.Count(); i++)
	{
		FormCyclePath(pszPath, m_ValidImages[i], szImagePath, sizeof(szImagePath));
		(*this.*AddImage)(szImagePath);
	}

	// setup images if they're are some
	if(GetImageCount() == 0 || m_iActiveImage != INVALID_IMAGE)
		return;

	DrawCycler = &ImageCycler::DrawActiveImage;
	m_iActiveImage = GetLastImageID();

	SetupImages();
}

//=========================================================
//=========================================================
void ImageCycler::SetupImages(bool bTimed)
{
	if(bTimed)
	{
		m_flSetupImage = system()->GetCurrentTime() + 0.25f;
		return;
	}

	for(int i = 0; i < GetImageCount(); i++)
	{
		Panel *pImage = GetImage(i);
		pImage->SetAlpha(0);
	}

	Panel *pActiveImage = GetImage(m_iActiveImage);
	pActiveImage->SetAlpha(255);
}

//=========================================================
//=========================================================
void ImageCycler::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	// set the border
	const char *pszBorder = inResourceData->GetString("border", "");

	if(*pszBorder)
	{
		IScheme *pScheme = scheme()->GetIScheme(GetScheme());
		SetBorder(pScheme->GetBorder(pszBorder));
	}

	// embedded?
	if(inResourceData->GetInt("embedded", 0))
		SetUseEmbedded(true);

	// fade control
	SetFadeImages(inResourceData->GetInt("fadeimages", 0) ? true : false);
	SetHoldTime(inResourceData->GetFloat("holdtime", DEFAULT_HOLD_TIME));
	SetFadeTime(inResourceData->GetFloat("fadetime", DEFAULT_FADE_TIME));
}

//=========================================================
//=========================================================
void ImageCycler::PaintBackground(void)
{
	(*this.*DrawCycler)();
}

//=========================================================
//=========================================================
void ImageCycler::OnVisibilityChange(int iOldVisible)
{
	if(!IsVisible())
	{
		// force out fading image
		UpdateFadingImage(true);
		return;
	}

	ResetNextFadeTime();
}

//=========================================================
//=========================================================
void ImageCycler::OnSizeChanged(int iWide, int iTall)
{
	BaseClass::OnSizeChanged(iWide, iTall);

	if(!m_bEmbeddedImages)
		return;

	for(int i = 0; i < GetImageCount(); i++)
		GetImage(i)->OnSizeChanged(iWide, iTall);
}

//=========================================================
//=========================================================
void ImageCycler::DrawActiveImage(void)
{
	// draw background when fading
	if(m_iFadingImage != INVALID_IMAGE)
		DrawBlank(g_FadeColor);

	// update fading image
	UpdateFadingImage();
	UpdateActiveImage();
}

//=========================================================
//=========================================================
void ImageCycler::DrawBlank(void)
{
	DrawBlank(g_DefaultColor);
}

//=========================================================
//=========================================================
void ImageCycler::UpdateActiveImage(void)
{
	if(m_flNextFade >= system()->GetCurrentTime() && GetImageCount() >= 1)
		return;

	// fade out active image
	UpdateAlphaImageOut();

	// set the new image
	m_iActiveImage = GetNextImage();
	ResetNextFadeTime();

	// fade in new image
	UpdateAlphaImageIn();
}

//=========================================================
//=========================================================
void ImageCycler::UpdateFadingImage(bool bForceOut)
{
	if(!m_bFadeImages || m_iFadingImage == INVALID_IMAGE)
		return;

	if(!bForceOut && m_flFadeImageTime >= system()->GetCurrentTime())
		return;

	Panel *pFadedImage = GetImage(m_iFadingImage);
	pFadedImage->SetAlpha(0);

	m_iFadingImage = INVALID_IMAGE;
}

//=========================================================
//=========================================================
void ImageCycler::UpdateAlphaImageOut(void)
{
	Panel *pActiveImage = GetImage(m_iActiveImage);

	if(m_bFadeImages && CanFadeImages())
	{
		GetAnimationController()->RunAnimationCommand(pActiveImage, "alpha", 0.0f, 0.0f, m_flFadeTime, AnimationController::INTERPOLATOR_LINEAR);

		m_iFadingImage = m_iActiveImage;
		m_flFadeImageTime = system()->GetCurrentTime() + m_flFadeTime;
	}
	else
	{
		pActiveImage->SetAlpha(25);
	}
}

//=========================================================
//=========================================================
void ImageCycler::UpdateAlphaImageIn(void)
{
	Panel *pActiveImage = GetImage(m_iActiveImage);

	if(m_bFadeImages && CanFadeImages())
		GetAnimationController()->RunAnimationCommand(pActiveImage, "alpha", 255.0f, 0.0f, m_flFadeTime, AnimationController::INTERPOLATOR_LINEAR);
	else
		pActiveImage->SetAlpha(255);
}

//=========================================================
//=========================================================
void ImageCycler::AllocateImageList(void)
{
	Assert(!m_bAllocated);

	/*if(m_bEmbeddedImages)
		m_ImageTypes.m_pEmbeddedImages = new CUtlVector<EmbeddedImage*>;
	else
		m_ImageTypes.m_pImages = new CUtlVector<ImagePanel*>;*/

	// Pon [
	if(m_bEmbeddedImages)
		m_pEmbeddedImages = new CUtlVector<EmbeddedImage*>;
	else
		m_pImages = new CUtlVector<ImagePanel*>;
	// Pon ]

	m_bAllocated = true;
}

//=========================================================
//=========================================================
void ImageCycler::DestroyImageList(void)
{
	Assert(m_bAllocated);

	/*if(m_bEmbeddedImages)
		delete m_ImageTypes.m_pEmbeddedImages;
	else
		delete m_ImageTypes.m_pImages;*/

	if(m_bEmbeddedImages)
		delete m_pEmbeddedImages;
	else
		delete m_pImages;

	m_bAllocated = false;
}

//=========================================================
//=========================================================
void ImageCycler::ClearImages(void)
{
	Assert(m_bAllocated);

	if(!m_bAllocated)
		return;

	/*if(m_bEmbeddedImages)
		m_ImageTypes.m_pEmbeddedImages->PurgeAndDeleteElements();
	else
		m_ImageTypes.m_pImages->PurgeAndDeleteElements();*/

	if(m_bEmbeddedImages)
		m_pEmbeddedImages->PurgeAndDeleteElements();
	else
		m_pImages->PurgeAndDeleteElements();
}

//=========================================================
//=========================================================
void ImageCycler::AddImagePanel(const char *pszPath)
{
	ImagePanel *pImagePanel = new ImagePanel(this, NULL);
	pImagePanel->SetShouldScaleImage(true);

	StartImageAdd(pImagePanel);
	pImagePanel->SetImage(pszPath);

	GetImages().AddToTail(pImagePanel);
}

//=========================================================
//=========================================================
void ImageCycler::AddEmbeddedImage(const char *pszPath)
{
	EmbeddedImage *pEmbeddedImage = new EmbeddedImage(this, NULL);

	StartImageAdd(pEmbeddedImage);
	pEmbeddedImage->SetImage(pszPath);

	GetEmbeddedImages().AddToTail(pEmbeddedImage);
}

//=========================================================
//=========================================================
void ImageCycler::StartImageAdd(Panel *pImage)
{
	int iWide, iTall;
	GetSize(iWide, iTall);

	pImage->SetPos(0, 0);
	pImage->SetSize(iWide, iTall);
}

//=========================================================
//=========================================================
void ImageCycler::DrawBlank(Color &Color)
{
	int iWidth, iTall;
	GetSize(iWidth, iTall);

	surface()->DrawSetColor(Color);
	surface()->DrawFilledRect(0, 0, iWidth, iTall);
}

//=========================================================
//=========================================================
int ImageCycler::GetImageCount(void) const
{
	if(m_bEmbeddedImages)
		return GetEmbeddedImages().Count();
	else
		return GetImages().Count();
}

//=========================================================
//=========================================================
Panel *ImageCycler::GetImage(int iID) const
{
	if(m_bEmbeddedImages)
		return GetEmbeddedImages().Element(iID);
	else
		return GetImages().Element(iID);
}

//=========================================================
//=========================================================
int ImageCycler::GetNextImage(void)
{
	int iNextImage = m_iActiveImage;

	if(iNextImage + 1 == GetImageCount())
		iNextImage = 0;
	else
		iNextImage++;

	return iNextImage;
}

//=========================================================
//=========================================================
void ImageCycler::ResetNextFadeTime(void)
{
	m_flNextFade = system()->GetCurrentTime() + m_flHoldTime;
}

//=========================================================
//=========================================================
CUtlVector<ImagePanel*> &ImageCycler::GetImages(void)
{
	// Pon [
	/*Assert(!m_bEmbeddedImages);
	return *m_ImageTypes.m_pImages;*/
	// Pon ]

	Assert(!m_bEmbeddedImages);
	return *m_pImages;
}

//=========================================================
//=========================================================
CUtlVector<EmbeddedImage*> &ImageCycler::GetEmbeddedImages(void)
{
	// Pon [
	/*Assert(m_bEmbeddedImages);
	return *m_ImageTypes.m_pEmbeddedImages;*/
	// Pon ]

	Assert(m_bEmbeddedImages);
	return *m_pEmbeddedImages;
}

//=========================================================
//=========================================================
const CUtlVector<ImagePanel*> &ImageCycler::GetImages(void) const
{
	// Pon [
	/*Assert(!m_bEmbeddedImages);
	return *m_ImageTypes.m_pImages;*/

	Assert(!m_bEmbeddedImages);
	return *m_pImages;
}

//=========================================================
//=========================================================
const CUtlVector<EmbeddedImage*> &ImageCycler::GetEmbeddedImages(void) const
{
	// Pon [
	/*Assert(m_bEmbeddedImages);
	return *m_ImageTypes.m_pEmbeddedImages;*/
	// Pon ]

	Assert(m_bEmbeddedImages);
	return *m_pEmbeddedImages;
}

//=========================================================
//=========================================================
void ImageCycler::ResetFade(void)
{
	ResetNextFadeTime();

	m_iFadingImage = INVALID_IMAGE;
	m_flFadeImageTime = 0.0f;
}