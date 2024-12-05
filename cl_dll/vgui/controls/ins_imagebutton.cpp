#include "cbase.h"

#include <vgui/isurface.h>

#include "ins_imagebutton.h"

#include "tier0/memdbgon.h"

using namespace vgui;


inline bool ValidTexture(int iTex)
{
	return iTex!=-1&&surface()->IsTextureIDValid(iTex);
}

int UseTexture(int &iTex, const char* pszFile, bool bForceReload)
{
	if (!ValidTexture(iTex))
	{
		iTex=surface()->DrawGetTextureId(pszFile);
		if (!ValidTexture(iTex))
		{
			iTex=surface()->CreateNewTextureID();
			surface()->DrawSetTextureFile(iTex, pszFile, false, true);
		}
	}
	if (bForceReload)
	{
		surface()->DrawSetTextureFile(iTex, pszFile, false, true);
	}

	surface()->DrawSetColor(Color(255,255,255,255));
	surface()->DrawSetTexture(iTex);

	return iTex;
}

ImageButton::ImageButton( const char* pszImage, const char* pszImageArmed, const char* pszImageDepressed, const char* pszImageDisabled, Panel *parent, const char *panelName, Panel *pActionSignalTarget, const char *pCmd ) : vgui::Button( parent, panelName, "", pActionSignalTarget, pCmd )
{
	m_iImage=m_iImageArmed=m_iImageDepressed=m_iImageDisabled=-1;

	m_iImH=m_iImW=0;

	m_pszImage=pszImage;
	m_pszImageArmed=pszImageArmed;
	m_pszImageDepressed=pszImageDepressed;
	m_pszImageDisabled=pszImageDisabled;
}

void ImageButton::SetImages( const char* pszImage, const char* pszImageArmed, const char* pszImageDepressed, const char* pszImageDisabled )
{
	if (pszImage)
	{
		m_pszImage=pszImage;
		UseTexture(m_iImageDepressed,m_pszImageDepressed,true);
	}
	if (pszImageArmed)
	{
		m_pszImageArmed=pszImageArmed;
		UseTexture(m_iImageArmed,m_pszImageArmed,true);
	}
	if (pszImageDepressed)
	{
		m_pszImageDepressed=pszImageDepressed;
		UseTexture(m_iImage,m_pszImage,true);
	}
	if (pszImageDisabled)
	{
		m_pszImageDisabled=pszImageDisabled;
		UseTexture(m_iImageDisabled,m_pszImageDisabled,true);
	}
}

void ImageButton::PaintBackground()
{
	if (IsEnabled())
	{
		if (IsDepressed())
		{
			UseTexture(m_iImageDepressed,m_pszImageDepressed);
		}
		else if (IsArmed())
		{
			UseTexture(m_iImageArmed,m_pszImageArmed);
		}
		else
		{
			UseTexture(m_iImage,m_pszImage);
		}
	}
	else
	{
		UseTexture(m_iImageDisabled,m_pszImageDisabled);
	}

	surface()->DrawTexturedRect(0,0,m_iImW,m_iImH);
}

void ImageButton::SetScaledImageSize( int iImW, int iImH )
{
	m_iImW=iImW;
	m_iImH=iImH;
}
