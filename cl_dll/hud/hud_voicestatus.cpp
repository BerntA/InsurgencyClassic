//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include <vgui_controls/panel.h>
#include <vgui/isurface.h>
#include <vgui/ilocalize.h>
#include "c_baseplayer.h"
#include "voice_status.h"
#include "clientmode_shared.h"
#include "c_playerresource.h"


//=============================================================================
// Icon for the local player using voice
//=============================================================================
class CHudVoiceSelfStatus : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudVoiceSelfStatus, vgui::Panel );

	CHudVoiceSelfStatus( const char *name );

	virtual bool ShouldDraw();	
	virtual void Paint();
	virtual void VidInit();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	int		m_iVoiceIcon;

	Color	m_clrIcon;
};


DECLARE_HUDELEMENT( CHudVoiceSelfStatus );


CHudVoiceSelfStatus::CHudVoiceSelfStatus( const char *pName ) :
	vgui::Panel( NULL, "HudVoiceSelfStatus" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );

	m_iVoiceIcon = -1;

	SetHiddenBits( 0 );

	m_clrIcon = Color(255,255,255,255);
}

	
void CHudVoiceSelfStatus::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

//#ifdef HL2MP
	SetBgColor( Color( 0, 0, 0, 0 ) );
//#endif
}

void CHudVoiceSelfStatus::VidInit( void )
{
	m_iVoiceIcon = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_iVoiceIcon, "voice/speaker4", false, false);
}

bool CHudVoiceSelfStatus::ShouldDraw()
{
	return GetClientVoiceMgr()->IsLocalPlayerSpeaking();
}

void CHudVoiceSelfStatus::Paint()
{
	int w, h;
	GetSize( w, h );

	vgui::surface()->DrawSetTexture(m_iVoiceIcon);
	vgui::surface()->DrawSetColor(m_clrIcon);
	vgui::surface()->DrawTexturedRect(0, 0, w, h);
}


//=============================================================================
// Icons for other players using voice
//=============================================================================
class CHudVoiceStatus : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudVoiceStatus, vgui::Panel );

	CHudVoiceStatus( const char *name );

	virtual bool ShouldDraw();	
	virtual void Paint();
	virtual void VidInit();
	virtual void Init();
	virtual void OnThink();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	int		m_iVoiceIcon;

	Color	m_clrIcon;

	CUtlLinkedList< int > m_SpeakingList;
	
	CPanelAnimationVar( vgui::HFont, m_NameFont, "Default", "Default" );

	CPanelAnimationVarAliasType( float, item_tall, "item_tall", "32", "proportional_float" );
	CPanelAnimationVarAliasType( float, item_wide, "item_wide", "100", "proportional_float" );

	CPanelAnimationVarAliasType( float, item_spacing, "item_spacing", "2", "proportional_float" );

	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_tall, "icon_tall", "32", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_wide, "icon_wide", "32", "proportional_float" );

	CPanelAnimationVarAliasType( float, text_ypos, "text_ypos", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_xpos, "text_xpos", "32", "proportional_float" );
};


DECLARE_HUDELEMENT( CHudVoiceStatus );


CHudVoiceStatus::CHudVoiceStatus( const char *pName ) :
	vgui::Panel( NULL, "HudVoiceStatus" ), CHudElement( pName )
{
	SetParent( g_pClientMode->GetViewport() );

	m_iVoiceIcon = -1;

	SetHiddenBits( 0 );

	m_clrIcon = Color(255,255,255,255);
}

void CHudVoiceStatus::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

//#ifdef HL2MP
	SetBgColor( Color( 0, 0, 0, 0 ) );
//#endif
}

void CHudVoiceStatus::Init( void )
{
	m_SpeakingList.RemoveAll();
}

void CHudVoiceStatus::VidInit( void )
{
	m_iVoiceIcon = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile(m_iVoiceIcon, "voice/speaker4", false, false);
}

void CHudVoiceStatus::OnThink( void )
{
	for( int i=1;i<=gpGlobals->maxClients;i++ )
	{
		bool bTalking = GetClientVoiceMgr()->IsPlayerSpeaking(i);

		// if they are in the list and not talking, remove them
		if( !bTalking )
		{
			// remove them if they are in the list
			m_SpeakingList.FindAndRemove(i);
		}
		else
		{
			// if they are talking and not in the list, add them to the end
			if( m_SpeakingList.Find(i) == m_SpeakingList.InvalidIndex() )
			{
				m_SpeakingList.AddToTail(i);
			}
		}
	}
}

bool CHudVoiceStatus::ShouldDraw()
{
	return true;
}

void CHudVoiceStatus::Paint()
{
	int x, y, w, h;
	GetBounds( x, y, w, h );

	// Heights to draw the current voice item at
	int xpos = 0;
	int ypos = h - item_tall;


	int i;
	int length = m_SpeakingList.Count();

	int iFontHeight = 0;

	if( length > 0 )
	{
		surface()->DrawSetTextFont( m_NameFont );
		surface()->DrawSetTextColor( Color(255,255,255,255) );
		iFontHeight = surface()->GetFontTall( m_NameFont );
	}

	//draw everyone in the list!
	for( i = m_SpeakingList.Head(); i != m_SpeakingList.InvalidIndex(); i = m_SpeakingList.Next(i) )
	{
		int playerIndex = m_SpeakingList.Element(i);

		// Pongles [
		//Color c = g_PR->GetTeamColor( g_PR ? g_PR->GetTeam(playerIndex) : TEAM_UNASSIGNED );
		Color c(255,0,0,255);
		// Pongles ]

		c[3] = 128;

		const char *pName = GameResources() ? GameResources()->GetPlayerName(playerIndex) : "unknown";
		wchar_t szconverted[ 64 ];
		localize()->ConvertANSIToUnicode( pName, szconverted, sizeof(szconverted)  );

		// Draw the item background
		surface()->DrawSetColor( c );
		surface()->DrawFilledRect( xpos, ypos, xpos + item_wide, ypos + item_tall );

		// Draw the voice icon
		vgui::surface()->DrawSetTexture(m_iVoiceIcon);
		vgui::surface()->DrawSetColor(m_clrIcon);
		vgui::surface()->DrawTexturedRect(xpos + icon_xpos, ypos + icon_ypos, xpos + icon_xpos + icon_wide, ypos + icon_ypos + icon_tall);

		// Draw the player's name
		surface()->DrawSetTextPos( xpos + text_xpos, ypos + ( item_tall / 2 ) - ( iFontHeight / 2 ) );

		int iTextSpace = item_wide - text_xpos;

		// write as much of the name as will fit, truncate the rest and add ellipses
		int iNameLength = wcslen(szconverted);
		int iTextWidthCounter = 0;
		for( int j=0;j<iNameLength;j++ )
		{
			iTextWidthCounter += surface()->GetCharacterWidth( m_NameFont, pName[j] );

			if( iTextWidthCounter > iTextSpace )
			{	
				if( j > 3 )
				{
					szconverted[j-2] = '.';
					szconverted[j-1] = '.';
					szconverted[j] = '\0';
				}
				break;
			}
		}

		surface()->DrawPrintText( szconverted, wcslen(szconverted) );
			
		ypos -= ( item_spacing + item_tall );
	}
}
