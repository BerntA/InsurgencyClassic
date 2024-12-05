#include <vgui_controls/button.h>

#ifndef INS_IMAGEBUTTON_H
#define INS_IMAGEBUTTON_H

#pragma once

inline bool ValidTexture(int iTex);
int UseTexture(int &iTex, const char* pszFile, bool bForceReload=false);

class ImageButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE(ImageButton, vgui::Button);

	const char* m_pszImage;
	const char* m_pszImageArmed;
	const char* m_pszImageDepressed;
	const char* m_pszImageDisabled;

	int m_iImage;
	int m_iImageArmed;
	int m_iImageDepressed;
	int m_iImageDisabled;

	int m_iImW, m_iImH;

protected:
	virtual void DrawFocusBorder(int tx0, int ty0, int tx1, int ty1) {};
	virtual vgui::IBorder *GetBorder(bool depressed, bool armed, bool selected, bool keyfocus) { return NULL; };

public:
	ImageButton( const char* pszImage, const char* pszImageArmed, const char* pszImageDepressed, const char* pszImageDisabled, Panel *parent, const char *panelName, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL );

	virtual void SetImages( const char* pszImage=NULL, const char* pszImageArmed=NULL, const char* pszImageDepressed=NULL, const char* pszImageDisabled=NULL );

	virtual void SetScaledImageSize( int iImW, int iImH );

	virtual void Paint() {};
	virtual void PaintBorder() {};
	virtual void PaintBackground();
};

#endif //INS_IMAGEBUTTON_H