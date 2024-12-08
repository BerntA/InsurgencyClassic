#ifndef HLS_COLOR_H
#define HLS_COLOR_H

#ifdef _WIN32
#pragma once
#endif

class HLSColor
{
public:
	HLSColor(const Color& col);

	void ConvertFromRGB(const Color& Color);
	Color ConvertToRGB(void);
	unsigned char ToRGB1(float rm1, float rm2, float rh);

	float m_flLuminance;
	float m_flSaturation;
	float m_flHue;
};

Color AdjustLuminance(const Color& col, float flLuminance);

#endif // HLS_COLOR_H