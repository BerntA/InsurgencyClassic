//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include <vgui/vgui.h>
#include "iclientmode.h"
#include "objective_status.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "c_ins_gamerules_obj.h"
#include "c_team.h"

float g_AlertColor[3] = { 255, 255, 255 }; // yes, white ... but I happen to like it

//=====================
//CObjectiveStatusLine
//=====================

//=========================================================
//=========================================================
CObjectiveStatusLine::CObjectiveStatusLine(vgui::Panel *parent, const char *panelName) : 
	vgui::RichText( parent, panelName )
{
	m_hFont = 0;
	m_flExpireTime = 0.0f;
	m_flStartTime = 0.0f;
	m_iNameLength = 0;

	SetPaintBackgroundEnabled(true);
	
	SetVerticalScrollbar(false);
}

//=========================================================
//=========================================================
void CObjectiveStatusLine::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont("Default");
	SetBorder(NULL);
	SetBgColor(Color(0, 0, 0, 0));
	SetFgColor(Color(0, 0, 0, 0));

	SetFont(m_hFont);
}


//=========================================================
//=========================================================
void CObjectiveStatusLine::SetExpireTime(void)
{
	m_flStartTime = gpGlobals->curtime;
	m_flExpireTime = m_flStartTime + OBJLINE_TIME;
	m_nCount = CObjectiveStatus::m_nLineCounter++;
}

//=========================================================
//=========================================================
bool CObjectiveStatusLine::IsReadyToExpire(void)
{
	if (!engine->IsInGame() && !engine->IsConnected())
		return true;

	if (gpGlobals->curtime >= m_flExpireTime)
		return true;

	return false;
}

//=========================================================
//=========================================================
void CObjectiveStatusLine::Expire(void)
{
	SetVisible(false);
}

//=========================================================
//=========================================================
void CObjectiveStatusLine::PerformFadeout(void)
{
	float curtime = gpGlobals->curtime;

	if(curtime <= m_flExpireTime && curtime > m_flExpireTime - OBJLINE_FADE_TIME)
	{
		float frac = (m_flExpireTime - curtime) / OBJLINE_FADE_TIME;

		int alpha = frac * 255;
		alpha = clamp(alpha, 0, 255);

		wchar_t msgbuf[4096];
		GetText(0, msgbuf, sizeof(msgbuf));

		SetText("");

		if(m_iNameLength > 0)
		{
			wchar_t namebuf[4096];
			wcsncpy( namebuf, msgbuf, m_iNameLength );

			m_clrNameColor[3] = alpha;

			InsertColorChange( m_clrNameColor );
			InsertString(namebuf);

			InsertColorChange( Color( g_AlertColor[0], g_AlertColor[1], g_AlertColor[2], alpha ) );
			InsertString( msgbuf + m_iNameLength );
			InvalidateLayout( true );
		}
		else // should never reach here
		{
			InsertColorChange(Color(g_AlertColor[0], g_AlertColor[1], g_AlertColor[2], alpha));
			InsertString( msgbuf );
		}
	}
}


//=====================
//CObjectiveStatus
//=====================

DECLARE_HUDELEMENT(CObjectiveStatus);
DECLARE_HUD_MESSAGE(CObjectiveStatus, ObjMsg);

int CObjectiveStatus::m_nLineCounter = 1;

//=========================================================
//=========================================================
CObjectiveStatus::CObjectiveStatus( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudObjective" )
{

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	SetScheme(scheme);

	CObjectiveStatusLine *line = m_ChatLines[0];

	if(line)
	{
		vgui::HFont font = line->GetFont();
		m_iFontHeight = vgui::surface()->GetFontTall(font) + 2;
	}

	m_flExpireTime = 0.0f;

	SetHiddenBits(HIDEHUD_OBJ_STATUS);
}

void CObjectiveStatus::SetVisible(bool state)
{
	BaseClass::SetVisible(state);
}

//=========================================================
//=========================================================
void CObjectiveStatus::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	m_nVisibleHeight = 0;
	SetFgColor(Color( 0, 0, 0, 0 ));
}

//=========================================================
//=========================================================
void CObjectiveStatus::Init(void)
{
	HOOK_HUD_MESSAGE(CObjectiveStatus, ObjMsg);

	for (int i = 0; i < OBJ_MAX_LINES; i++ )
	{
		char sz[32];
		Q_snprintf(sz, sizeof(sz), "ObjStatus%02i", i);
		m_ChatLines[i] = new CObjectiveStatusLine(this, sz);
		m_ChatLines[i]->SetVisible(false);
	}
}

//=========================================================
//=========================================================
void CObjectiveStatus::MsgFunc_ObjMsg(bf_read &msg)
{
	int iEntity = msg.ReadByte();
	C_INSObjective *pEntity = (C_INSObjective*)cl_entitylist->GetEnt(iEntity);

	int iMessageType = msg.ReadByte();

	char szMessage[1024];
	memset(szMessage, 0, 1024);

	C_Team *pTeam = GetGlobalTeam(pEntity->GetCapturingTeam());

	const char *pszTeamName = pTeam->Get_Name();

	switch(iMessageType)
	{
		case OBJ_CAPTURE_START:
		{
			sprintf(szMessage, "Being Captured by the %s", pszTeamName);
			break;
		}
		case OBJ_NOT_ENOUGH_PLAYERS:
		{
			strcpy(szMessage, "Not Enough Players to Capture");
			break;
		}
		/*
		case OBJ_OUTSIDE:
		{
			break;
		}
		*/
		case OBJ_CAPTURED:
		{
			sprintf(szMessage, "Been Captured by the %s", pszTeamName);
			break;
		}
		/*
		case OBJ_NO_RECAPTURE:
		{
			break;
		}
		*/
		default:
		{
			// if you hit this assert then something went wrong when sending
			// the message to update the objective status
			Assert(false);
			break;
		}

	}

	Print(pEntity, szMessage);

	m_flExpireTime = gpGlobals->curtime + OBJ_FADE_TIME;
	SetVisible(true);
}

//=========================================================
//=========================================================
void CObjectiveStatus::Print(C_INSObjective *obj, const char *msg)
{
	if (!*msg)
		return;

	CObjectiveStatusLine *line = FindUnusedLine();

	char parsemsg[4096];
	sprintf(parsemsg, ": %s", msg);
	line->SetNameLength(strlen(obj->GetName()));

	Color newcolor;
	newcolor.SetColor(obj->GetColor().r, obj->GetColor().g, obj->GetColor().b, 255);
	line->SetNameColor(newcolor);

	if ( !line )
	{
		ExpireOldest();
		line = FindUnusedLine();
	}

	if ( !line )
	{
		return;
	}

	line->SetText( "" );
	
	/*
	player_info_t sPlayerInfo;
	if ( iPlayerIndex == 0 )
	{
		Q_memset( &sPlayerInfo, 0, sizeof(player_info_t) );
		Q_strncpy( sPlayerInfo.name, "Console", sizeof(sPlayerInfo.name)  );	
	}
	else
	{
		engine->GetPlayerInfo( iPlayerIndex, &sPlayerInfo );
	}


	const char *pName = sPlayerInfo.name;

	if ( pName )
	{
		const char *nameInString = strstr( pmsg, pName );

		if ( nameInString )
		{
			iNameLength = strlen( pName ) + (nameInString - pmsg);
		}
	}
	else
	*/
		//line->InsertColorChange( Color( g_ColorYellow[0], g_ColorYellow[1], g_ColorYellow[2], 255 ) );
	//char *buf = static_cast<char *>( _alloca( strlen( pmsg ) + 1  ) );
	//wchar_t *wbuf = static_cast<wchar_t *>( _alloca( (strlen( pmsg ) + 1 ) * sizeof(wchar_t) ) );
	if ( msg )
	{
		//float *flColor = GetClientColor( iPlayerIndex );

		line->SetExpireTime();
	
		// draw the first x characters in the player color
		//Q_strncpy( buf, pmsg, min( iNameLength + 1, MAX_PLAYER_NAME_LENGTH+32) );
		//buf[ min( iNameLength, MAX_PLAYER_NAME_LENGTH+31) ] = 0;
		//line->InsertColorChange( Color( flColor[0], flColor[1], flColor[2], 255 ) );
		//line->InsertString( buf );
		//Q_strncpy( buf, pmsg + iNameLength, strlen( pmsg ));
		//buf[ strlen( pmsg + iNameLength ) ] = '\0';
		//line->InsertColorChange( Color( g_AlertColor[0], g_AlertColor[1], g_AlertColor[2], 255 ) );
		//vgui::localize()->ConvertANSIToUnicode( buf, wbuf, strlen(pmsg)*sizeof(wchar_t));
		//line->InsertString( parsemsg );

			line->InsertColorChange( newcolor );
			line->InsertString(obj->GetName());
			line->InsertColorChange( Color( g_AlertColor[0], g_AlertColor[1], g_AlertColor[2], 255 ) );
			line->InsertString( parsemsg );


		line->SetVisible( true );
		//line->SetNameLength( iNameLength );
		//line->SetNameColor( 255, flColor[1], flColor[2], 255 ) );
	}
}

//=========================================================
//=========================================================
static int __cdecl SortLines( void const *line1, void const *line2 )
{
	CObjectiveStatusLine *l1 = *( CObjectiveStatusLine ** )line1;
	CObjectiveStatusLine *l2 = *( CObjectiveStatusLine ** )line2;

	// Invisible at bottom
	if ( l1->IsVisible() && !l2->IsVisible() )
		return -1;
	else if ( !l1->IsVisible() && l2->IsVisible() )
		return 1;

	// Oldest start time at top
	if ( l1->GetStartTime() < l2->GetStartTime() )
		return -1;
	else if ( l1->GetStartTime() > l2->GetStartTime() )
		return 1;

	// Otherwise, compare counter
	if ( l1->GetCount() < l2->GetCount() )
		return -1;
	else if ( l1->GetCount() > l2->GetCount() )
		return 1;

	return 0;
}

//=========================================================
//=========================================================
void CObjectiveStatus::OnThink( void )
{
	int i;
	/*
	float curtime = gpGlobals->curtime;

	if(curtime <= m_flExpireTime && curtime > m_flExpireTime - OBJ_FADE_TIME)
	{
		SetVisible(false);
	}
	*/

	for (i = 0; i < OBJ_MAX_LINES; i++)
	{
		CObjectiveStatusLine *line = m_ChatLines[ i ];

		if (!line)
			continue;

		if (!line->IsVisible())
			continue;

		if (!line->IsReadyToExpire())
			continue;

		line->Expire();
	}

	int w, h;

	GetSize(w, h);

	CObjectiveStatusLine *line = m_ChatLines[0];

	if (line)
	{
		vgui::HFont font = line->GetFont();

		if (font)
		{
			m_iFontHeight = vgui::surface()->GetFontTall(font) + 2;
		}
	}

	// Sort chat lines 
	qsort(m_ChatLines, OBJ_MAX_LINES, sizeof(CObjectiveStatusLine*), SortLines);

	// Step backward from bottom
	int currentY = h - m_iFontHeight - 1;
	int startY = currentY;
	int ystep = m_iFontHeight;

	// Walk backward
	for (i = OBJ_MAX_LINES - 1; i >= 0 ; i--)
	{
		CObjectiveStatusLine *line = m_ChatLines[i];

		if (!line)
			continue;

		if (!line->IsVisible())
		{
			line->SetSize( w, m_iFontHeight );
			continue;
		}

		line->PerformFadeout();
		line->SetSize( w, m_iFontHeight * line->GetNumLines() );
		line->SetPos( 0, ( currentY+m_iFontHeight) - m_iFontHeight * line->GetNumLines() );
		
		currentY -= ystep * line->GetNumLines();
	}

	if (currentY != startY)
	{
		m_nVisibleHeight = startY - currentY + 2;
	}
	else
	{
		m_nVisibleHeight = 0;
	}

	vgui::surface()->MovePopupToBack( GetVPanel() );
}

//=========================================================
//=========================================================
void CObjectiveStatus::Reset( void )
{
	m_nVisibleHeight = 0;
	Clear();
}

//=========================================================
//=========================================================
CObjectiveStatusLine *CObjectiveStatus::FindUnusedLine( void )
{
	for ( int i = 0; i < OBJ_MAX_LINES; i++ )
	{
		CObjectiveStatusLine *line = m_ChatLines[ i ];
		if ( !line )
			continue;

		if ( line->IsVisible() )
			continue;

		return line;
	}
	return NULL;
}

//=========================================================
//=========================================================
void CObjectiveStatus::ExpireOldest( void )
{
	float oldestTime = 100000000.0f;
	CObjectiveStatusLine *oldest = NULL;

	for ( int i = 0; i < OBJ_MAX_LINES; i++ )
	{
		CObjectiveStatusLine *line = m_ChatLines[ i ];
		if ( !line )
			continue;

		if ( !line->IsVisible() )
			continue;

		if ( !oldest )
		{
			oldest = line;
			oldestTime = line->GetStartTime();
			continue;
		}

		if ( line->GetStartTime() < oldestTime )
		{
			oldest = line;
			oldestTime = line->GetStartTime();
		}
	}

	if ( !oldest )
	{
		oldest = m_ChatLines[ 0  ];
	}

	oldest->Expire(); 
}

//=========================================================
//=========================================================
void CObjectiveStatus::Clear( void )
{
	for ( int i = 0; i < OBJ_MAX_LINES; i++ )
	{
		CObjectiveStatusLine *line = m_ChatLines[i];

		if (!line)
			continue;

		if (!line->IsVisible())
			continue;

		line->Expire();
	}
}
