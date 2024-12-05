//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ins_loadingpanel.h"

#include <vgui/isurface.h>
#include <vgui/ivgui.h>
#include <vgui_controls/textentry.h>
#include <vgui_controls/textimage.h>
#include <vgui_controls/imagepanel.h>
#include <vgui_controls/button.h>
#include <vgui_controls/buildgroup.h>
#include <vgui_controls/richtext.h>
#include "gameuipanel.h"

#include "filesystem.h"
#include "imc_config.h"
#include "ins_gamerules.h"
#include "mapname_utils.h"
#include "hud_stringfont.h"
#include "basic_colors.h"
#include "motd.h"
#include "keyvalues.h"
#include "inetchannelinfo.h"
#include "hud.h"
#include "ins_imagebutton.h"
#include "c_playerresource.h"
#include "networkstringtable_clientdll.h"
#include "ins_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CONTROL_SIZE(PROGRESS_BAR,595,127,361,26);

CONTROL_SIZE(MAP_IMAGE,51,197,517,517);

CONTROL_SIZE(LOADING_STATUS,589,102,241,18);

CONTROL_SIZE(SERVER_INFO,790,76,372,44);

CONTROL_SIZE(MAP_DESC,603,270,359,173);

CONTROL_SIZE(GAME_TYPE,603,487,359,196);

CONTROL_SIZE(TIP_TEXT,42,770,943,40);

CONTROL_SIZE(GAME_TYPE_NAME,628,432,147,60);

//=========================================================
//=========================================================
//ConVar customload( "cl_customload", "1", FCVAR_CLIENTDLL, "Toggle the Custom Load Screen", true, 0, true, 1 );

//=========================================================
//=========================================================
INSLoadingDialog::INSLoadingDialog( ) : CINSPanel( NULL, "LoadingDialogINS", "conflict", false )
{
	m_pMapImage=new Panel(this,"MapImage");
	m_pLoadingStatus=new Label(this,"LoadingStatus","Loading...");
	m_pServerInfo=new Label(this,"ServerInfo","");
	m_pMapDesc=new Label(this,"MapDesc","I love this map de dust.");
	m_pGameType=new Label(this,"GameType","InstaGib!");
	m_pGameTypeName= new Label( this, "GameTypeNamw", "Test" );
	m_pTipText = new Label( this, "TipText", "Je suis francais"	);

	m_pMapImage->SetVisible(false);

	m_pServerInfo->SetWrap(true);
	m_pServerInfo->SetContentAlignment(Label::a_west);
	m_pMapDesc->SetWrap(true);
	m_pMapDesc->SetContentAlignment(Label::a_northeast);
	m_pGameType->SetWrap(true);
	m_pGameType->SetContentAlignment(Label::a_northeast);
	m_pTipText->SetWrap( true );
	m_pTipText->SetContentAlignment( Label::a_center );
	m_pGameTypeName->SetContentAlignment( Label::a_southeast );
	m_pGameTypeName->SetFgColor( Color( 137, 137, 137 ) );

	SetScheme( "ClientScheme" );

	m_iMapImage=-1;

	m_bFading=false;
	m_bDrawBG=false;

	m_bInit = false;
}

void INSLoadingDialog::ApplySchemeSettings(IScheme *pScheme)
{
	m_pLoadingStatus->SetFont(pScheme->GetFont("DefaultSmall",true));
	m_pServerInfo->SetFont(pScheme->GetFont("Default"));
	m_pMapDesc->SetFont(pScheme->GetFont("DefaultSmall",true));
	m_pGameType->SetFont(pScheme->GetFont("DefaultSmall",true));
	m_pGameTypeName->SetFont(pScheme->GetFont("GameTypeName",true));
	m_pTipText->SetFont( pScheme->GetFont( "DefaultBold", true ) );
	
	m_pServerInfo->SetFgColor( Color( 240, 240, 240, 255 ) );
	m_pMapDesc->SetFgColor( Color( 240, 240, 240, 255 ) );
	m_pServerInfo->SetFgColor( Color( 240, 240, 240, 255 ) );
	m_pGameType->SetFgColor( Color( 240, 240, 240, 255 ) );
	m_pTipText->SetFgColor( Color( 137, 137, 137, 255 ) );
	m_pGameTypeName->SetFgColor( Color( 137, 137, 137, 255 ) );

	m_pServerInfo->SetContentAlignment(Label::a_west);
}

INSLoadingDialog::~INSLoadingDialog()
{
	g_pGameUIPanel->DeleteLoadPanel();
}

bool IsPanelValid(VPANEL iPanel)
{
	return iPanel&&ivgui()->PanelToHandle(iPanel);
}

#define MAX_TEXTHEADER_LENGTH 512

//=========================================================
//=========================================================
void INSLoadingDialog::Paint( void )
{
	CLoadDialog &LoadDialog = g_pGameUIPanel->GetLoadDialog( );
	VPANEL iPanel=LoadDialog.GetPanel(LOADELEMENT_INFO);
	if (IsPanelValid(iPanel)&&!m_bFading)
	{
		KeyValues *kv = new KeyValues( "GetText" );
		ipanel()->RequestInfo(iPanel,kv);
		m_pLoadingStatus->SetText(kv->GetWString("text"));
		kv->deleteThis();
	}

	UseTexture(m_iMapImage,m_szMapImage);
	int x,y,w,h;
	m_pMapImage->GetBounds(x,y,w,h);
	w+=x;
	h+=y;
	surface()->DrawTexturedRect(x,y,w,h);

	if (!m_bInit && ValidPlayerResource( ) &&PlayerResource( ) )
	{
		int iCurrentPlayers = 0;
		// NOTE: there *must* be a better way of doing this
		for(int i = 1; i <= MAX_PLAYERS; i++)
		{
			if(PlayerResource( )->IsConnected( i ))
				iCurrentPlayers++;
		}

		char szText[MAX_TEXTHEADER_LENGTH];
		int iMaxClients = gpGlobals->maxClients;

		Q_snprintf(szText, sizeof(szText), "%s\n%i/%i", g_pszHostname, iCurrentPlayers, iMaxClients);

		m_pServerInfo->SetText( szText );
		m_pServerInfo->SetWrap( true );
		m_bInit = true;
	}
	
}

void LPDrawCropped(int iXPos, int iYPos, int iWide, int iHeight, int iTexWide, int iTexTall, int x, int y)
{
	TexCoord_t texCoords;
	CreateTexCoord(texCoords,0, 0, CINSPanel::SZ_SQSIZE, CINSPanel::SZ_SQSIZE, CINSPanel::SZ_SQSIZE, CINSPanel::SZ_SQSIZE);



	float fw = (float)iTexWide;
	float fh = (float)iTexTall;

	float twidth	= texCoords[ 2 ] - texCoords[ 0 ];
	float theight	= texCoords[ 3 ] - texCoords[ 1 ];

	// Interpolate coords
	float tCoords[ 4 ];
	tCoords[ 0 ] = texCoords[ 0 ] + ( (float)iXPos / fw ) * twidth;
	tCoords[ 1 ] = texCoords[ 1 ] + ( (float)iYPos / fh ) * theight;
	tCoords[ 2 ] = texCoords[ 0 ] + ( (float)(iXPos + iWide ) / fw ) * twidth;
	tCoords[ 3 ] = texCoords[ 1 ] + ( (float)(iYPos + iHeight ) / fh ) * theight;



	vgui::surface()->DrawTexturedSubRect(x, y, x + iWide, y + iHeight, tCoords[ 0 ], tCoords[ 1 ], tCoords[ 2 ], tCoords[ 3 ] );
}

void INSLoadingDialog::PaintBackground()
{
	int w,h;
	GetSize(w,h);
	int iAlpha=255;
	if (m_bFading)
	{
		iAlpha-=(gpGlobals->curtime-m_flFadeTime-1.0f)*100.0f;
		iAlpha=clamp(iAlpha,0,255);
	}
	surface()->DrawSetColor(Color(0,0,0,iAlpha));
	if (m_bFading)
	{
		surface()->DrawFilledRect(0,0,w,h);
		m_bDrawBG=true;
	}
	else //draw around the progress bar
	{
		surface()->DrawFilledRect(0,0,w,m_iY+IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y));
		surface()->DrawFilledRect(0,m_iY+IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y),m_iX+IntegerScale(SZ_PROGRESS_BAR_OFFSET_X),m_iY+IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y+SZ_PROGRESS_BAR_SIZE_Y));
		surface()->DrawFilledRect(m_iX+IntegerScale(SZ_PROGRESS_BAR_OFFSET_X+SZ_PROGRESS_BAR_SIZE_X),m_iY+IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y),w,m_iY+IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y+SZ_PROGRESS_BAR_SIZE_Y));
		surface()->DrawFilledRect(0,m_iY+IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y+SZ_PROGRESS_BAR_SIZE_Y),w,h);
	}

	BaseClass::PaintBackground();

	if (!m_bFading) //draw around the progress bar
	{
		UseTexture(m_iBackground,m_szBackground);
		LPDrawCropped(0,0,m_iS1,IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y),m_iS1,m_iS1,m_iX,m_iY);
		LPDrawCropped(0,IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y),IntegerScale(SZ_PROGRESS_BAR_OFFSET_X),IntegerScale(SZ_PROGRESS_BAR_SIZE_Y),m_iS1,m_iS1,m_iX,m_iY+IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y));
		LPDrawCropped(IntegerScale(SZ_PROGRESS_BAR_OFFSET_X+SZ_PROGRESS_BAR_SIZE_X),IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y),m_iS1-IntegerScale(SZ_PROGRESS_BAR_OFFSET_X+SZ_PROGRESS_BAR_SIZE_X),IntegerScale(SZ_PROGRESS_BAR_SIZE_Y),m_iS1,m_iS1,m_iX+IntegerScale(SZ_PROGRESS_BAR_OFFSET_X+SZ_PROGRESS_BAR_SIZE_X),m_iY+IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y));
		LPDrawCropped(0,IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y+SZ_PROGRESS_BAR_SIZE_Y),m_iS1,m_iS1-IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y+SZ_PROGRESS_BAR_SIZE_Y),m_iS1,m_iS1,m_iX,m_iY+IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y+SZ_PROGRESS_BAR_SIZE_Y));
	}
}

void INSLoadingDialog::PerformLayout()
{
	BaseClass::PerformLayout();

	SCALE_CONTROL(m_pMapImage,MAP_IMAGE);
	SCALE_CONTROL(m_pMapDesc,MAP_DESC);
	SCALE_CONTROL(m_pLoadingStatus,LOADING_STATUS);
	SCALE_CONTROL(m_pServerInfo,SERVER_INFO);
	SCALE_CONTROL(m_pGameType,GAME_TYPE);
	SCALE_CONTROL(m_pGameTypeName,GAME_TYPE_NAME);
	SCALE_CONTROL(m_pTipText,TIP_TEXT);
}

//=========================================================
//=========================================================
void INSLoadingDialog::MOTDInit( void )
{

}

void INSLoadingDialog::SetupContents()
{
	int w,h;
	engine->GetScreenSize(w,h);
	SetBounds( 0, 0, w, h );

	char szMapName[ 128 ];
	UTIL_ParseMapName( szMapName, sizeof( szMapName ) );

	Q_snprintf(m_szMapImage,MAX_PATH,"maps/overviews/%s",szMapName);
	m_pMapDesc->SetText(IMCConfig()->GetMapOverview());
}

#include <string>

const char *g_pszHostname = NULL;

//=========================================================
//=========================================================
void INSLoadingDialog::Setup( void )
{
	CLoadDialog &LoadDialog = g_pGameUIPanel->GetLoadDialog( );

	// set parent and correct our size
	VPANEL Parent = LoadDialog.GetPanel( LOADELEMENT_PARENT );
	if (!IsPanelValid(Parent))
		return;
	SetParent( Parent );
	Activate();

	// translate and apply modified elements
	TranslateLoadDimensions( );

	SetupContents();

	KeyValues *pLoadingScreenData = new KeyValues( "LoadingScreen" );
	Assert( pLoadingScreenData );

	if( !pLoadingScreenData )
		return;

	pLoadingScreenData->LoadFromFile( ::filesystem, "scripts/loadingscreen.txt" );

	KeyValues *pGameTypes = pLoadingScreenData->FindKey( "GameTypes" );

	if( pGameTypes )
	{
		const char *pszGameType;
		// int iHitGTID;

		for( KeyValues *pGameType = pGameTypes->GetFirstTrueSubKey( ); pGameType; pGameType = pGameType->GetNextKey( ) )
		{
			pszGameType = pGameType->GetName( );
			
			if( pszGameType && IMCConfig( )->GetGameName( ) )
			{
				if(!Q_strcmp( pszGameType, IMCConfig( )->GetGameName( ) ) )
				{
					m_pGameType->SetText( pGameType->GetString( "desc" ) );
					char szGameModeName[128];
					Q_snprintf( szGameModeName, sizeof( szGameModeName ), "%s GAMEMODE", pszGameType );
					m_pGameTypeName->SetText( szGameModeName );
				}
			}
		}
	}

	KeyValues *pTips = pLoadingScreenData->FindKey( "Tips" );
	
	if (pTips)
	{
		int i = 0;
		for( KeyValues *pTip = pTips->GetFirstSubKey( ); pTip; pTip = pTip->GetNextKey( ) )
		{
			m_tips[i] = pTip->GetString( );
			i++;
		}
	}
	
	// Lets choose a random tip
	const char *pszTip = m_tips[random->RandomInt(0, m_tips.size( ))].c_str( );
	string finalTip = "";
	string command	= "";
	int i = 0;
	while( i < Q_strlen( pszTip ) )
	{
		if(pszTip[i] == '\0')
			break;
		if(pszTip[i] == '{')
		{
			i++; // Skip '{'
			while(pszTip[i] != '}' && i < Q_strlen( pszTip ) )
			{
				command += pszTip[i];
				i++;
			}
			i++; // skip '}'
			const char *pszButton = engine->Key_LookupBinding( command.c_str( ) );
			if( !pszButton )
				finalTip += "<unbound>";
			else
			{
				if(Q_strlen( pszButton ) < 2)
					finalTip += toupper( *pszButton );
				else
					finalTip += pszButton;
			}
			continue;
		}
		finalTip += pszTip[i];

		i++;
	}

	m_pTipText->SetText( finalTip.c_str( ) );

	g_pszHostname = cvar->FindVar( "hostname" )->GetString( );
	if(!g_pszHostname)
		g_pszHostname = "no server";

	m_bInit = false;

	// apply our changes
	LoadDialog.Apply( m_ModifiedElements );
}

//=========================================================
//=========================================================
void INSLoadingDialog::TranslateLoadDimensions( void )
{
	// find all the parts
	CLoadElementDimensions &Parent = m_ModifiedElements.m_Parent;
	CLoadElementDimensions &Progress = m_ModifiedElements.m_Progress;

	// ... parent
	int iParentWide, iParentTall;
	engine->GetScreenSize(iParentWide,iParentTall);

	PerformLayout();

	Parent.Init( 0, 0, iParentWide, iParentTall );
	Progress.Init( m_iX+IntegerScale(SZ_PROGRESS_BAR_OFFSET_X), m_iY+IntegerScale(SZ_PROGRESS_BAR_OFFSET_Y), IntegerScale(SZ_PROGRESS_BAR_SIZE_X), IntegerScale(SZ_PROGRESS_BAR_SIZE_Y) );
}

//=========================================================
//=========================================================
void INSLoadingDialog::LoadingUpdate( void )
{
	EnableNextButton(false);
	m_bFading=false;

	// find IMC
	CIMCConfig *pIMCConfig = CIMCConfig::IMCConfig( );

	// ... check valid IMC
	if( !pIMCConfig )
	{
		Assert( false );
		return;
	}

	// ... setup when needed
	if( !pIMCConfig->IsInit( ) )
		pIMCConfig->Setup( );

	// check to see if we want to change it
	//if( !customload.GetBool( ) )
		//return;

	// setup panel or update it
	CLoadDialog &LoadDialog = g_pGameUIPanel->GetLoadDialog( );

	VPANEL iPanel=LoadDialog.GetPanel( LOADELEMENT_PARENT );

	if (!IsPanelValid(iPanel))
		return;

	if( LoadDialog.IsModified( ) )
	{
		// ... update the parent
		m_ModifiedElements.m_Parent.Apply( iPanel );
	}
	else if( IMCConfig( ) && !IMCConfig( )->UsingDefaults( ) )
	{
		// ... setup
		Setup( );
	}
}

//=========================================================
//=========================================================
void INSLoadingDialog::LoadedUpdate( void )
{
	SetParent(NULL);
	SetupContents();
	PerformLayout();
	m_pLoadingStatus->SetText("Map loaded.");
	MakePopup();
	Activate();
	EnableNextButton(true);
	m_bFading=true;
	m_flFadeTime=gpGlobals->curtime;
}

//=========================================================
//=========================================================
void INSLoadingDialog::OnCommand(const char *pszCommand)
{
	if( FStrEq( pszCommand, "deploy" ) )
	{
		// close the panel
		ShowPanel( false );

		g_pGameUIPanel->JoinServer();
	}
}