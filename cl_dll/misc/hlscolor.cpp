//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "hlscolor.h"

//=========================================================
//=========================================================
void HLSColor::ConvertFromRGB(const Color &Color)
{
	unsigned char minval = min(Color.r(), min(Color.g(), Color.b()));
	unsigned char maxval = max(Color.r(), max(Color.g(), Color.b()));
	float mdiff  = float(maxval) - float(minval);
	float msum   = float(maxval) + float(minval);

	m_flLuminance = msum / 510.0f;

	if (maxval == minval) 
	{
		m_flSaturation = 0.0f;
		m_flHue = 0.0f; 
	}   
	else 
	{ 
		float rnorm = (maxval - Color.r()  ) / mdiff;      
		float gnorm = (maxval - Color.g()) / mdiff;
		float bnorm = (maxval - Color.b() ) / mdiff;   

		m_flSaturation = (m_flLuminance <= 0.5f) ? (mdiff / msum) : (mdiff / (510.0f - msum));

		if (Color.r()   == maxval) m_flHue = 60.0f * (6.0f + bnorm - gnorm);
		if (Color.g() == maxval) m_flHue = 60.0f * (2.0f + rnorm - bnorm);
		if (Color.b()  == maxval) m_flHue = 60.0f * (4.0f + gnorm - rnorm);
		if (m_flHue > 360.0f) m_flHue = m_flHue - 360.0f;
	}
}

//=========================================================
//=========================================================
Color HLSColor::ConvertToRGB(void)
{
	int iRed, iGreen, iBlue;

	if (m_flSaturation == 0.0) // Grauton, einfacher Fall
	{
		iRed = iGreen = iBlue = unsigned char(m_flLuminance * 255.0);
	}
	else
	{
		float rm1, rm2;

		if (m_flLuminance <= 0.5f) rm2 = m_flLuminance + m_flLuminance * m_flSaturation;  
		else                     rm2 = m_flLuminance + m_flSaturation - m_flLuminance * m_flSaturation;
		rm1 = 2.0f * m_flLuminance - rm2;   
		iRed   = ToRGB1(rm1, rm2, m_flHue + 120.0f);   
		iGreen = ToRGB1(rm1, rm2, m_flHue);
		iBlue  = ToRGB1(rm1, rm2, m_flHue - 120.0f);
	}

	return Color(iRed, iGreen, iBlue, 255);
}

//=========================================================
//=========================================================
unsigned char HLSColor::ToRGB1(float rm1, float rm2, float rh)
{
	if      (rh > 360.0f) rh -= 360.0f;
	else if (rh <   0.0f) rh += 360.0f;

	if      (rh <  60.0f) rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;   
	else if (rh < 180.0f) rm1 = rm2;
	else if (rh < 240.0f) rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;      

	return static_cast<unsigned char>(rm1 * 255);
}

//=========================================================
//=========================================================
Color AdjustLuminance(Color &InputColor, float flLuminance)
{
	HLSColor NewColor(InputColor);
	NewColor.m_flLuminance = flLuminance;

	return NewColor.ConvertToRGB();
}