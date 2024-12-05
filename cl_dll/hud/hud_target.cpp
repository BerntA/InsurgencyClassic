//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: HUD Target ID element
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hudelement.h"
#include <vgui_controls/panel.h>
#include "c_playerresource.h"
#include "vgui_entitypanel.h"
#include "iclientmode.h"
#include "vgui/ilocalize.h"
#include "ins_gamerules.h"
#include "hlscolor.h"
#include "basic_colors.h"
#include "ins_player_shared.h"
#include "ins_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PLAYER_HINT_DISTANCE	150
#define PLAYER_HINT_DISTANCE_SQ	(PLAYER_HINT_DISTANCE*PLAYER_HINT_DISTANCE)

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTargetID : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CTargetID, vgui::Panel );

public:
	CTargetID( const char *pElementName );
	void Init( void );
	virtual void	ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void	Paint( void );
	void VidInit( void );

private:
	vgui::HFont		m_hFont;
	int				m_iLastEntIndex;
	float			m_flLastChangeTime;

	Color m_WHealthColors[HEALTHTYPE_COUNT];
};

DECLARE_HUDELEMENT( CTargetID );

using namespace vgui;

#define TARGET_LUMINANCE 0.9f

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTargetID::CTargetID( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "TargetID" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_hFont = g_hFontTrebuchet24;
	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;

	for(int i = 0; i < HEALTHTYPE_COUNT; i++)
	{
		HLSColor WColor(C_INSPlayer::GetHealthColor(i));
		WColor.m_flLuminance = TARGET_LUMINANCE;

		m_WHealthColors[i] = WColor.ConvertToRGB();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Setup
//-----------------------------------------------------------------------------
void CTargetID::Init( void )
{
};

void CTargetID::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_hFont = scheme->GetFont( "TargetID", IsProportional() );

	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: clear out string etc between levels
//-----------------------------------------------------------------------------
void CTargetID::VidInit()
{
	CHudElement::VidInit();

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Draw function for the element
//-----------------------------------------------------------------------------
void CTargetID::Paint()
{
	// NEEDS DISTANCE CHECKING (ON SQUAD ETC ETC) AND DISTANCE

	wchar_t wszPlayerName[ MAX_PLAYER_NAME_LENGTH ];

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();

	if ( !pPlayer )
		return;

	Color c;

	// Get our target's ent index
	int iEntIndex = pPlayer->GetIDTarget();

	// Didn't find one?
	if ( !iEntIndex )
	{
		// Check to see if we should clear our ID
		if ( m_flLastChangeTime && (gpGlobals->curtime > (m_flLastChangeTime + 0.5)) )
		{
			m_flLastChangeTime = 0;
			wszPlayerName[0] = 0;
			m_iLastEntIndex = 0;
		}
		else
		{
			// Keep re-using the old one
			iEntIndex = m_iLastEntIndex;
		}
	}
	else
	{
		m_flLastChangeTime = gpGlobals->curtime;
	}

	// Is this an entindex sent by the server?
	if ( iEntIndex )
	{
		C_INSPlayer *pPlayer = ToINSPlayer(cl_entitylist->GetEnt( iEntIndex ));
		C_INSPlayer *pLocalPlayer = C_INSPlayer::GetLocalPlayer();

		bool bShowPlayerName = false;

		// Some entities we always want to check, cause the text may change
		// even while we're looking at it
		// Is it a player?
		if ( pPlayer && IsPlayerIndex( iEntIndex ) )
		{
			if(pLocalPlayer->GetSquadID() == pLocalPlayer->GetSquadID())
			{
				c = m_WHealthColors[pPlayer->GetHealthType()];
			}
			else
			{
				c = COLOR_LGREY;
			}

			int iDistance = UTIL_2DCaculateDistance(pPlayer, pLocalPlayer) * METERS_PER_INCH;

			char szPlayerName[MAX_PLAYER_NAME_LENGTH],
				szCompletePlayerName[MAX_PLAYER_NAME_LENGTH*2];

			if(pPlayer->GetFullPlayerName(iEntIndex, szPlayerName, sizeof(szPlayerName)))
			{
				Q_snprintf(szCompletePlayerName, sizeof(szCompletePlayerName), "%s: %im", szPlayerName, iDistance);

				bShowPlayerName = true;
				vgui::localize()->ConvertANSIToUnicode(UTIL_SafeName(szCompletePlayerName),  wszPlayerName, sizeof(wszPlayerName) );
			}
		}

	
		if ( wszPlayerName[0] )
		{
			int wide, tall;
			int ypos = YRES(420);
			int xpos = XRES(10);

			vgui::surface()->GetTextSize( m_hFont, wszPlayerName, wide, tall );

			xpos = (ScreenWidth() - wide) / 2;
			
			vgui::surface()->DrawSetTextFont( m_hFont );
			vgui::surface()->DrawSetTextPos( xpos, ypos );
			vgui::surface()->DrawSetTextColor( c );
			vgui::surface()->DrawPrintText( wszPlayerName, wcslen(wszPlayerName) );
		}
	}
}
