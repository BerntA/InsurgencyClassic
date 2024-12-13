#define PROTECTED_THINGS_H

#include "cbase.h"

// Needed for wsprintf
#include <windows.h>

#undef PROTECTED_THINGS_H
#include "protected_things.h"

// Absolute minimum of VGUI includes
#include <vgui/vgui.h>
#include <vgui_controls/image.h>
#include <vgui_controls/label.h>
#include <vgui_controls/panel.h>
#include <vgui_controls/imagepanel.h>

// Things for SteamID detect
#include "cdll_client_int.h"
#include "inetchannelinfo.h"

// INSMod watermark class.
#include "ins_watermark.h"

// Must be included last.
#include "tier0/memdbgon.h"

CINSWatermarkPanel::CINSWatermarkPanel() : vgui::Panel()
{
    this->m_pImgINSLogo     = new vgui::ImagePanel( this, "INSWatermark:INSLogo" );
    this->m_pImgINSLogo->SetImage( "vgui/logos/ins" );
    this->m_pLblVersionInfo = new vgui::Label( this, "INSWatermark:VersionInfo", L"Insurgency Mod BETA" );
    this->m_pLblUserInfo    = new vgui::Label( this, "INSWatermark:UserInfo", L"" );
    this->SetEnabled( false );
    this->SetVisible( false );
}

CINSWatermarkPanel::~CINSWatermarkPanel()
{
    delete this->m_pImgINSLogo;
    delete this->m_pLblUserInfo;
    delete this->m_pLblVersionInfo;
}

void CINSWatermarkPanel::Activate(){}

void CINSWatermarkPanel::ApplySchemeSettings( vgui::IScheme *pScheme){}

void CINSWatermarkPanel::OnCommand( const char *command ) {}

void CINSWatermarkPanel::OnThink() {}

void CINSWatermarkPanel::UpdateUserInfo()
{
    // User information string is in the following format:
    // [STEAMID] at [DATE] [TIME]
    if( !this->m_szSteamID )
    {        
        const char *szSteamID = engine->GetNetChannelInfo()->GetAddress();
        this->m_szSteamID = new wchar_t[ strlen( szSteamID ) + 1 ];
        mbstowcs( this->m_szSteamID, szSteamID, strlen( szSteamID ) + 1 );
    }

    wchar_t *pBuffer = new wchar_t[64];
    wsprintfW( pBuffer, L"%s at %s %s", this->m_szSteamID, 
               L"YYYY-MM-DD", L"HH:MM:SS" );
    this->m_pLblUserInfo->SetText( pBuffer );
    delete pBuffer;


}
