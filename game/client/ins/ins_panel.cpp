#include "cbase.h"

#include <vgui/isurface.h>

#include "insvgui.h"
#include "ins_panel.h"
#include "ins_imagebutton.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define SZ_SQSIZE_POWER2 10

CONTROL_SIZE(BUTTON_IMAGE,779,643,512,512);

CONTROL_SIZE(BUTTON_REAL,SZ_BUTTON_IMAGE_OFFSET_X+135,SZ_BUTTON_IMAGE_OFFSET_Y+135,242,242);

CINSPanel::CINSPanel(Panel *parent, const char *panelName, const char* pszINSVGUIname, bool bPopup) : Panel(parent, panelName)
{
/*
	SetTitleBarVisible(false);
	SetSizeable(false);
	SetMoveable(false);
*/
	if (bPopup)
	{
		SetVisible(false);
		MakePopup();
	}

	m_iBackground=m_iImage=m_iImageArmed=m_iImageDepressed=m_iImageDisabled=-1;

	Q_snprintf(m_szBackground,MAX_PATH,"VGUI/%s/top",pszINSVGUIname);
	m_pszImage="VGUI/deploy/deploy";
	m_pszImageArmed="VGUI/deploy/deploy_hover";
	m_pszImageDepressed="VGUI/deploy/deploy";
	m_pszImageDisabled="VGUI/deploy/cant_deploy";

	m_pNext=new Button(this,"DeployButton","",this,"deploy");
	m_pNext->SetVisible(true);
	m_pNext->SetPaintBorderEnabled(false);
	m_pNext->SetPaintBackgroundEnabled(false);

	m_bDrawBG=true;
}

void CINSPanel::SetScaledBounds(Panel* pPanel, int X, int Y, int W, int H)
{
	pPanel->SetBounds(m_iX+IntegerScale(X),m_iY+IntegerScale(Y),IntegerScale(W),IntegerScale(H));
}

void CINSPanel::PerformLayout()
{
	int w,h;
	engine->GetScreenSize(w,h);
	SetBounds(0,0,w,h);

	if (h>=SZ_SQSIZE) //dont scale
	{
		m_iX=(w-SZ_SQSIZE)>>1;
		m_iY=(h-SZ_SQSIZE)>>1;
		m_iS1=SZ_SQSIZE;
		m_iS2=SZ_SQSIZE_POWER2;
	}
	else
	{
		m_iY=0;
		m_iS1=h;
		m_iS2=SZ_SQSIZE_POWER2;
		m_iX=(w-h)>>1;
	}
	
	SCALE_CONTROL(m_pNext,BUTTON_REAL);
}

void CINSPanel::Activate()
{
	MoveToFront();
	RequestFocus();
	SetVisible(true);
	SetEnabled(true);

	surface()->SetMinimized(GetVPanel(), false);
	Reset();
}


const char *CINSPanel::GetName( void )
{
	return BaseClass::GetName();
}

void CINSPanel::SetData(KeyValues *data)
{

}

void CINSPanel::Reset( void )
{
	//
}

void CINSPanel::Update( void )
{
	//
}

bool CINSPanel::NeedsUpdate( void )
{
	return false;
}

bool CINSPanel::HasInputElements( void )
{
	return true;
}

void CINSPanel::ShowPanel( bool state )
{
	if (state)
	{
		if(IsVisible() != state)
			Activate();
		SetMouseInputEnabled(true);
		SetKeyBoardInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);
	}

	SetVisible( state );
	surface()->CalculateMouseVisible( );
}

VPANEL CINSPanel::GetVPanel( void )
{
	return BaseClass::GetVPanel( );
}

bool CINSPanel::IsVisible( void )
{
	return BaseClass::IsVisible( );
}

void CINSPanel::SetParent( VPANEL parent )
{
	BaseClass::SetParent( parent );
}

void CINSPanel::EnableNextButton( bool bEnable )
{
	m_pNext->SetEnabled( bEnable );
}

void CINSPanel::PaintBackground( void )
{
	if (m_pNext->IsEnabled( ))
	{
		if (m_pNext->IsDepressed( ))
		{
			UseTexture( m_iImageDepressed, m_pszImageDepressed );
		}
		else if (m_pNext->IsArmed( ))
		{
			UseTexture( m_iImageArmed, m_pszImageArmed );
		}
		else
		{
			UseTexture( m_iImage, m_pszImage );
		}
	}
	else
	{
		UseTexture(m_iImageDisabled,m_pszImageDisabled);
	}
	surface()->DrawTexturedRect(m_iX+IntegerScale(SZ_BUTTON_IMAGE_OFFSET_X),m_iY+IntegerScale(SZ_BUTTON_IMAGE_OFFSET_Y),m_iX+IntegerScale(SZ_BUTTON_IMAGE_OFFSET_X)+IntegerScale(SZ_BUTTON_IMAGE_SIZE_X),m_iY+IntegerScale(SZ_BUTTON_IMAGE_OFFSET_Y)+IntegerScale(SZ_BUTTON_IMAGE_SIZE_Y));

	if (m_bDrawBG)
	{
		UseTexture(m_iBackground,m_szBackground);
		surface()->DrawTexturedRect(m_iX,m_iY,m_iX+m_iS1,m_iY+m_iS1);
	}
}
/*
IViewPortPanel *CreatePanel__CINSPanel( CBaseViewport *pViewport )
{
	return new CINSPanel(NULL,PANEL_CUSTOMIZEGUI,"allegiance");
}
CINSViewportHelper g_Helper__CINSPanel( CreatePanel__CINSPanel );

CON_COMMAND_F(vgui_test,"",FCVAR_CLIENTDLL)
{
	gViewPortInterface->ShowPanel(PANEL_CUSTOMIZEGUI,true);
}
*/