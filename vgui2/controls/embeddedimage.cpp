//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>

#include <vgui/isurface.h>

#include <vgui_controls/embeddedimage.h>
#include <vgui_controls/imagepanel.h>

#include <keyvalues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//=========================================================
//=========================================================
EmbeddedImage::EmbeddedImage(Panel *pParent, const char *pszPanelName)
		: EditablePanel(pParent, pszPanelName)
{
	SetProportional(true);

	m_pImage = new ImagePanel(this, NULL);
	m_pImage->SetShouldScaleImage(true);
	m_pImage->SetPos(0, 0);
	m_pImage->SetFillColor(Color(0, 0, 0, 255));
}

//=========================================================
//=========================================================
void EmbeddedImage::SetImage(const char *pszPath)
{
	m_pImage->SetImage(pszPath);
}

//=========================================================
//=========================================================
void EmbeddedImage::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	const char *pszImageName = inResourceData->GetString("image", "");

	if(*pszImageName)
		SetImage(pszImageName);

	const char *pszBorder = inResourceData->GetString("border", "");

	if(*pszBorder)
	{
		IScheme *pScheme = scheme()->GetIScheme(GetScheme());
		SetBorder(pScheme->GetBorder(pszBorder));
	}
}

//=========================================================
//=========================================================
void EmbeddedImage::OnSizeChanged(int iWide, int iTall)
{
	int iScreenWide, iScreenTall;
	surface()->GetScreenSize(iScreenWide, iScreenTall);

	iWide *= (640 / (float)iScreenWide);
	iTall *= (480 / (float)iScreenTall);
	
	int iAdjustedWide = FindPower2Size(iWide);
	int iAdjustedTall = FindPower2Size(iTall);

	iAdjustedWide = scheme()->GetProportionalScaledValue(iAdjustedWide);
	iAdjustedTall = scheme()->GetProportionalScaledValue(iAdjustedTall);

	m_pImage->SetSize(iAdjustedWide, iAdjustedTall);

	BaseClass::OnSizeChanged(iWide, iTall);
}

//=========================================================
//=========================================================
int EmbeddedImage::FindPower2Size(int iSize)
{
	int iShifts = 0;
	iSize--;

	while(iSize)
	{
		iSize = iSize >> 1;
		iShifts++;
	}

	return 1 << iShifts;
}