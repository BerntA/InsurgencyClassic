//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef IMAGECYCLER_H
#define IMAGECYCLER_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/vgui.h>
#include <vgui_controls/panel.h>
#include "utlvector.h"

//=========================================================
//=========================================================
#define MAX_CYCLE_IMAGES 25

#define DEFAULT_HOLD_TIME 2.5f
#define DEFAULT_FADE_TIME 0.0f

namespace vgui
{

class ImagePanel;
class EmbeddedImage;

//=========================================================
//=========================================================
class ImageCycler : public Panel
{
	DECLARE_CLASS_SIMPLE(ImageCycler, Panel);

public:
	ImageCycler(Panel *pParent, const char *pszName);
	virtual ~ImageCycler();

	void Reset(void);
	void ResetActiveImage(void);

	void SetUseEmbedded(bool bState, bool bInt = false);

	void SetFadeImages(bool bFadeImages);
	void SetHoldTime(float flHoldTime) { m_flHoldTime = flHoldTime; }
    void SetFadeTime(float flFadeTime) { m_flFadeTime = flFadeTime; }

	void AddImages(const char *pszPath);
	void SetupImages(bool bTimed = true);

	int GetImageCount(void) const;

protected:
	virtual void Think(void);
	virtual void ApplySettings(KeyValues *inResourceData);
	virtual void PaintBackground(void);
	virtual void OnVisibilityChange(int iOldVisible);
	virtual void OnSizeChanged(int iWide, int iTall);

private:
	void DrawActiveImage(void);
	void DrawBlank(void);
	void DrawBlank(Color &Color);

	void UpdateActiveImage(void);
	void UpdateAlphaImageIn(void);
	void UpdateAlphaImageOut(void);

	int GetNextImage(void);
	inline int GetLastImageID(void) { return (GetImageCount() - 1); }

	bool CanFadeImages(void) const { return GetImageCount() > 1; }
	void UpdateFadingImage(bool bForceOut = false);
	void ResetNextFadeTime(void);
	void ResetFade(void);

	void AddImagePanel(const char *pszPath);
	void AddEmbeddedImage(const char *pszPath);
	void StartImageAdd(Panel *pImage);

	void AllocateImageList(void);
	void DestroyImageList(void);
	void ClearImages(void);

	Panel *GetImage(int iID) const;

	CUtlVector<ImagePanel*> &GetImages(void);
	const CUtlVector<ImagePanel*> &GetImages(void) const;

	CUtlVector<EmbeddedImage*> &GetEmbeddedImages(void);
	const CUtlVector<EmbeddedImage*> &GetEmbeddedImages(void) const;

private:
	CUtlVector<int> m_ValidImages;

	bool m_bEmbeddedImages;

	//union {
		CUtlVector<ImagePanel*> *m_pImages;
		CUtlVector<EmbeddedImage*> *m_pEmbeddedImages;

	//} m_ImageTypes;

	void (ImageCycler::*AddImage)(const char *pszPath);

	bool m_bFadeImages;
	float m_flFadeTime;
	float m_flHoldTime;

	void (ImageCycler::*DrawCycler)(void);

	int m_iActiveImage;
	float m_flNextFade;

	int m_iFadingImage;
	float m_flFadeImageTime;

	float m_flSetupImage;

	bool m_bAllocated;
};

} // namespace vgui

#endif // IMAGECYCLER_H
