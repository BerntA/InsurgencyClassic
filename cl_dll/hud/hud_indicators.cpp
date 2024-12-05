


#include "cbase.h"
#include "hud_indicators.h"
#include "iclientmode.h"

CHUDIndicators::CHUDIndicators(const char *pszElementName, const char *pszPanelName)
	: CHudElement(pszElementName), BaseClass(NULL, pszPanelName)
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetPaintBackgroundEnabled(false);
	SetProportional(true);
	SetPos(0, 0);
	SetSize(640, 480);

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	SetScheme(scheme);
}

void CHUDIndicators::DrawArrow(Vector &vecOrigin, Color &Color)
{
	// find the position where the to draw the arrow
	// to show the player where to go to find the 
	// object in question
}