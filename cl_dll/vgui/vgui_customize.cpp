//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <vgui/iborder.h>
#include <vgui/iinput.h>
#include <vgui/ilocalize.h>
#include <vgui/ivgui.h>
#include <vgui/ipanel.h>

#include <cdll_client_int.h>

#include <vgui/ischeme.h>
#include <vgui/isurface.h>
#include <keyvalues.h>
#include <filesystem.h>

#include <vgui_controls/panel.h>
#include <vgui_controls/editablepanel.h>
#include <vgui_controls/button.h>
#include <vgui_controls/combobox.h>
#include <vgui_controls/richtext.h>
#include <vgui_controls/insframe.h>
#include <vgui_controls/menu.h>
#include <vgui_controls/menuitem.h>

#include "ins_player_shared.h"

#include "animation.h"
#include "studio.h"
#include "playercust.h"
#include "vgui_customize.h"
#include "c_team.h"
#include "play_team_shared.h"
#include "team_lookup.h"

#include "engine/ivmodelinfo.h"
#include "engine/ivmodelrender.h"

#include "cdll_convar.h"

#include "c_baseanimating.h"

#include "tier0/memdbgon.h"

#include "engine/ivdebugoverlay.h"

using namespace vgui;

//deathz0rz [
// i might move this later
void WorldTransform( const Vector& screen, Vector& point )
{
	float w;
	VMatrix screenToWorld;
	MatrixInverseGeneral(engine->WorldToScreenMatrix(),screenToWorld);

	point[0]	= screenToWorld[0][0] * screen[0] + screenToWorld[0][1] * screen[1] + screenToWorld[0][2] * screen[2] + screenToWorld[0][3];
	point[1]	= screenToWorld[1][0] * screen[0] + screenToWorld[1][1] * screen[1] + screenToWorld[1][2] * screen[2] + screenToWorld[1][3];
	point[2]	= screenToWorld[2][0] * screen[0] + screenToWorld[2][1] * screen[1] + screenToWorld[2][2] * screen[2] + screenToWorld[2][3];
	w			= screenToWorld[3][0] * screen[0] + screenToWorld[3][1] * screen[1] + screenToWorld[3][2] * screen[2] + screenToWorld[3][3];
	if ( w )
	{
		w = 1.0 / w;
		point[0] *= w;
		point[1] *= w;
		point[2] *= w;
	}
}
//deathz0rz ]

ConVar cl_customizeguimodelz("cl_customizeguimodelz", "0.928", FCVAR_ARCHIVE,"The size of the model in the customization window.\nAdjust between 0.90 and 0.95. Higher is smaller. Default: 0.928");
ConVar cl_customizeguirotrate("cl_customizeguirotrate", "66.67", FCVAR_ARCHIVE,"The speed of the rotation of the model in the customization window.\nAdjust between 10 and 1000. Higher is faster. Default: 66.67");
ConVar cl_customizeguirotenabletime("cl_customizeguirotenabletime", "0.2", FCVAR_ARCHIVE,"The maximum mouseclick length that still enables rotation after rotation has been changed (see customization window in the manual).\nAdjust between 0.2 and 0.001. Higher is longer. Default: 0.2");

class C_InfoCustomizeView : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_InfoCustomizeView, C_BaseEntity );
	DECLARE_CLIENTCLASS();
};

IMPLEMENT_CLIENTCLASS_DT( C_InfoCustomizeView, DT_InfoCustomizeView, CInfoCustomizeView )
END_RECV_TABLE()

class C_PreviewModel : public C_BaseFlex
{
private:
	DECLARE_CLASS( C_PreviewModel, C_BaseFlex );

	Vector m_vecOrigin;
	Panel* m_pParentPanel;

	int m_iX;
	int m_iY;
	bool m_bOldXY;

	enum RotateState_e {
		PREVMDL_ROTATING_LEFT=1,
		PREVMDL_STOPPED_LEFT=-1,
		PREVMDL_ROTATING_RIGHT=2,
		PREVMDL_STOPPED_RIGHT=-2,
	};
	RotateState_e m_eRotating;
	float m_flCustomYaw;
	float m_flOldYaw;
	float m_flOldTime;
public:
	void SetRenderOrigin(int x, int y)
	{
		m_bOldXY=true;
		m_iX=x;
		m_iY=y;
	};

	C_PreviewModel() : BaseClass()
	{
		m_bOldXY=false;
		m_flOldYaw=sqrt(-1.0f);
		m_flCustomYaw=0.0f;
		m_eRotating=PREVMDL_ROTATING_RIGHT;
	};

	virtual void SpawnClientEntity()
	{
		AddEffects(EF_NODRAW);
		SetNextClientThink(CLIENT_THINK_ALWAYS);
		ForceClientSideAnimationOn();
	};

	void UpdateClientSideAnimation()
	{
		if (GetSequence()!=-1)
			FrameAdvance(gpGlobals->frametime);
	}

	void ClientThink( void )
	{
		BaseClass::ClientThink();
		if (!ShouldDrawPostVGUI())
			return;
		float flTime=engine->Time();
		switch (m_eRotating)
		{
			case PREVMDL_ROTATING_LEFT:
				m_flCustomYaw-=(flTime-m_flOldTime)*cl_customizeguirotrate.GetFloat();
				break;
			case PREVMDL_ROTATING_RIGHT:
				m_flCustomYaw+=(flTime-m_flOldTime)*cl_customizeguirotrate.GetFloat();
				break;
			default:
				;
		}
		m_flOldTime=flTime;
		if (m_flOldYaw!=m_flCustomYaw)
		{
			QAngle angAngles=QAngle(0,m_flCustomYaw,0);		
			SetAbsAngles(angAngles);
			m_flOldYaw=m_flCustomYaw;
		}
	};

	int DrawModel( int flags )
	{
		return 0;
	};

	int DrawModelPostVGUI( int flags )
	{
		if (m_bOldXY)
		{ //do this now, because this is the viewsetup we are going to use
			Vector vecScreen;
			int w,h;
			engine->GetScreenSize(w,h);
			vecScreen.x=m_iX/(w*.5)-1;
			vecScreen.y=m_iY/(h*-.5)+1;
			vecScreen.z=cl_customizeguimodelz.GetFloat();
			WorldTransform(vecScreen,m_vecOrigin);
			SetAbsOrigin(m_vecOrigin);
			m_bOldXY=false;
		}
		return modelrender->DrawModel( 
			flags, 
			this,
			GetModelInstance(),
			index, 
			GetModel(),
			GetRenderOrigin(),
			GetRenderAngles(),
			m_nSkin,
			m_nBody,
			0,
			NULL,
			NULL );
	};

	bool ShouldDrawPostVGUI()
	{
		return ipanel()->IsFullyVisible(m_pParentPanel->GetVPanel());
	};

	bool ShouldDraw()
	{
		return false;
	};

	void SetParentPanel(Panel* panel)
	{
		m_pParentPanel=panel;
	};

	inline Panel* GetParentPanel()
	{
		return m_pParentPanel;
	};

	inline void ToggleRotate()
	{
		m_eRotating=(RotateState_e)-m_eRotating;
	};

	void AdjustYaw(float flYaw)
	{
		if (flYaw<0)
			m_eRotating=PREVMDL_STOPPED_LEFT;
		else
			m_eRotating=PREVMDL_STOPPED_RIGHT;
		
		m_flCustomYaw+=flYaw;
	};

	virtual void Release()
	{
		m_pParentPanel->SetPostChildPaintEnabled(false);
		BaseClass::Release();
	};
};

LINK_ENTITY_TO_CLASS( client_preview_model, C_PreviewModel );

class CCustomizeGUI::CModelPanel : public Panel
{
private:
	CCustomizeGUI* m_pCustGUI;
	C_PreviewModel* m_pEntity;
	friend class CCustomizeGUI;

	float m_flMouseDownTime;

	int m_OldX;
public:
	CModelPanel(CCustomizeGUI* m_pCustGUI, Panel *parent, const char *panelName) : Panel(parent,panelName)
	{
		this->m_pCustGUI=m_pCustGUI;
		m_pEntity=NULL;
		SetBgColor(Color(0,0,0,255));
		m_flMouseDownTime=-1.0f;
		m_OldX=0x7fffffff;
	};

	void SetCustomizeGUI(CCustomizeGUI* m_pCustGUI)
	{
		this->m_pCustGUI=m_pCustGUI;
	};

	virtual void Paint()
	{
		m_pCustGUI->PaintModel(this);
	}

	void ResetModel(const model_t* pModel)
	{
		if (!m_pEntity)
		{
			m_pEntity=new C_PreviewModel(); 
			m_pEntity->InitializeAsClientEntity(NULL,RENDER_GROUP_OPAQUE_ENTITY);
			m_pEntity->SetParentPanel(this);
		}
		int x,y;
		GetSize(x,y);
		x*=.5;
		y*=.94;
		LocalToScreen(x,y);
		m_pEntity->SetRenderOrigin(x,y);
		m_pEntity->SetModelPointer(pModel);
		if (pModel)
		{
			int iSequence=m_pEntity->LookupSequence("menu_idle");
			if (iSequence==-1)
				iSequence=m_pEntity->LookupSequence("idle");
			if (iSequence==-1)
				iSequence=0;
			m_pEntity->SetSequence(iSequence);
		}
	};

	void DrawModel(const model_t* pModel, int nBody, int nSkin)
	{
		if (!m_pEntity)
			ResetModel(pModel);
		
		m_pEntity->m_nBody=nBody;
		m_pEntity->m_nSkin=nSkin;
	};

	void OnCursorMoved(int x, int y)
	{
		if (m_OldX==0x7fffffff)
			m_OldX=x;
		if (m_flMouseDownTime>0.0f)
		{
			int dx=x-m_OldX;
			m_pEntity->AdjustYaw(dx);
			m_OldX=x;
		}
	}

	void OnMousePressed(MouseCode code)
	{
		if (code==MOUSE_LEFT)
		{
			m_flMouseDownTime=engine->Time();
			m_OldX=0x7fffffff;
		}
	}

	void OnMouseReleased(MouseCode code)
	{
		if (code==MOUSE_LEFT)
		{
			if ((engine->Time()-m_flMouseDownTime)<cl_customizeguirotenabletime.GetFloat())
			{
				m_pEntity->ToggleRotate();
			}
			m_flMouseDownTime=-1.0f;
		}
	}

	void SetPostChildPaintEnabled(bool state)
	{
		m_pEntity=NULL;
	}
};

class CCustomizeGUI::EventComboBox : public ComboBox
{
private:
	DECLARE_CLASS_SIMPLE(EventComboBox, ComboBox);

	CCustomizeGUI* m_pCustGUI;
protected:
	MESSAGE_FUNC_PTR( OnMenuItemSelected2, "MenuItemSelected", panel );
public:
	EventComboBox(CCustomizeGUI* m_pCustGUI, Panel *parent, const char *panelName, int numLines, bool allowEdit);
	void SetCustomizeGUI(CCustomizeGUI* m_pCustGUI);
};

CCustomizeGUI::EventComboBox::EventComboBox(CCustomizeGUI* m_pCustGUI, Panel *parent, const char *panelName, int numLines, bool allowEdit) : ComboBox(parent, panelName, numLines, allowEdit)
{
	this->m_pCustGUI=m_pCustGUI;
}

void CCustomizeGUI::EventComboBox::SetCustomizeGUI(CCustomizeGUI* m_pCustGUI)
{
	this->m_pCustGUI=m_pCustGUI;
}

void CCustomizeGUI::EventComboBox::OnMenuItemSelected2(Panel* panel)
{
	m_pCustGUI->OnMenuItemSelected(panel);
}

//=========================================================
//=========================================================
CCustomizeGUI::CCustomizeGUI(IViewPort *pViewPort) : INSFrame(NULL, PANEL_CUSTOMIZEGUI)
{
	SetScheme("ClientScheme");

	m_pModelPanel = new CModelPanel(this,this,"ModelPanel");

	m_pModels = new EventComboBox(this,this,"ModelSelect",6,false);

	m_pSkins = new EventComboBox(this,this,"SkinSelect",6,false);
	m_pBodygroups = new EventComboBox(this,this,"BdyGrpSelect",6,false);
	m_pSubmodels = new EventComboBox(this,this,"SubMdlSelect",6,false);

	m_pDescription = new RichText(this,"SelectionDesc");
	m_pDescription->SetAllowMouse(false);
	m_pDescription->ClearText();

	m_pMessage = new Label(this,"MessageWindow","NOTE: We are aware of any bugs in the customization window. Do not submit bug reports about them, we are awaiting the SDK update to fix the bugs.");

	m_apButtons[0] = new Button(this,"OKButton","");
	m_apButtons[1] = new Button(this,"CancelButton","");
	m_apButtons[2] = new Button(this,"ResetButton","");
	
	// load settings
	LoadControlSettings("Resource/UI/Frames/CustomizeGUI.res");

	// set the new parent
	m_pModelPanel->SetParent(GetClientArea());
	m_pSkins->SetParent(GetClientArea());
	m_pBodygroups->SetParent(GetClientArea());
	m_pSubmodels->SetParent(GetClientArea());
	m_pDescription->SetParent(GetClientArea());
	m_pMessage->SetParent(GetClientArea());
	for (int i=0;i<sizeof(m_apButtons)/sizeof(m_apButtons[0]);i++)
	{
		m_apButtons[i]->SetParent(GetClientArea());
	}

	m_pModel=NULL;

	// reset!
	Reset();
}

//=========================================================
//=========================================================
void CCustomizeGUI::Reset(void)
{
}

//=========================================================
//=========================================================
void CCustomizeGUI::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pMessage->SetFgColor(Color(255,0,0,255));

	int tall=GetClientArea()->GetTall();

	// move the buttons to the bottom-left corner
	int xpos = 10;
	int ypos = tall - 34;

	m_apButtons[1]->SetBounds(xpos, ypos, 72, 24);
	ypos -= 24 + 3;
	m_apButtons[0]->SetBounds(xpos, ypos, 72, 24);
	ypos -= 24 + 3 + 24 + 3;
	m_apButtons[2]->SetBounds(xpos, ypos, 72, 24);

	if (m_pModel)
		m_pModelPanel->ResetModel(m_pModel);
	Repaint();
}

//=========================================================
//=========================================================
void CCustomizeGUI::OnCommand(const char *command)
{
	if(FStrEq(command, "Finish"))
	{
		SaveModelCustomization(*m_pMdlCust,SendToServer(),m_nBody,m_nSkin);

		gViewPortInterface->ShowPanel(this, false);
		Reset();
	}
	else if(FStrEq(command, "Cancel"))
	{
		gViewPortInterface->ShowPanel(this, false);
		Reset();
	}
	else if(FStrEq(command, "Reset"))
	{
		Setup();
	}

	BaseClass::OnCommand(command);
}

void CCustomizeGUI::OnMenuItemSelected(Panel *panel)
{
	Panel* cppanel=((MenuItem*)panel)->GetParentMenu()->GetParent();
	if (cppanel==m_pSkins)
	{
		m_nSkin=m_pSkins->GetActiveItemUserData()->GetInt("Skin");
		m_pModel=modelinfo->GetModel(m_pMdlCust->nModelIndex);
		m_pModelPanel->ResetModel(m_pModel);
		if (m_bOverrideText)
			return;
		skincustomization_t& skin=m_pMdlCust->aSkins[m_nSkin];
		int iLen=Q_strlen(skin.pszName)+Q_strlen(skin.pszDesc)+20;
		char* pszTmp=new char[iLen];
		Q_snprintf(pszTmp, iLen, "Skin: %s\n%s", skin.pszName, skin.pszDesc);
		m_pDescription->ClearText();
		m_pDescription->SetText(pszTmp);
		delete [] pszTmp;
	}
	else if (cppanel==m_pBodygroups)
	{
		LocalSetBodygroup(m_pBodygroups->GetActiveItemUserData()->GetInt("Bodygroup"));
	}
	else if (cppanel==m_pSubmodels)
	{
		SetBodygroup(m_pStuHdr,m_nBody,m_pSubmodels->GetActiveItemUserData()->GetInt("Bodygroup"),m_pSubmodels->GetActiveItemUserData()->GetInt("Submodel"));
		if (m_bOverrideText)
		{
			m_bOverrideText=false;
			return;
		}
		bodygroupcustomization_t& group=m_pMdlCust->aBodygroups[m_pSubmodels->GetActiveItemUserData()->GetInt("Bodygroup")];
		bodygroupmodelcustomization_t& model=group.aSubmodels[m_pSubmodels->GetActiveItemUserData()->GetInt("Submodel")];
		int iLen=Q_strlen(group.pszName)+Q_strlen(model.pszName)+Q_strlen(model.pszDesc)+20;
		char* pszTmp=new char[iLen];
		Q_snprintf(pszTmp, iLen, "%s: %s\n%s", group.pszName, model.pszName, model.pszDesc);
		m_pDescription->ClearText();
		m_pDescription->SetText(pszTmp);
		delete [] pszTmp;
	}
	else if (cppanel==m_pModels)
	{
		// Pongles [
		if (m_pMdlCust)
			SaveModelCustomization(*m_pMdlCust,SendToServer(),m_nBody,m_nSkin);
		if (!m_pModels->GetActiveItemUserData())
			return;
		m_pMdlCust=&g_pTeamLookup[m_pModels->GetActiveItemUserData()->GetInt("TeamLookupID")]->GetCustomizationForModify();
		SetupModel();
		// Pongles ]
	}
}

//=========================================================
//=========================================================
bool CCustomizeGUI::SendToServer(void)
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();

	if(!pPlayer)
		return false;

	return (pPlayer->OnPlayTeam() ? (pPlayer->GetModelIndex() == m_pMdlCust->nModelIndex) : false);
}

//=========================================================
//=========================================================
void CCustomizeGUI::RequestFocus(int direction)
{
	InvalidateLayout();
	Setup();
}

//=========================================================
//=========================================================
void CCustomizeGUI::Setup(void)
{
	Reset();

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();
	Assert(pPlayer);

	// Pongles [
	int iTeamLookupID, i;
	iTeamLookupID = 0;

	if (pPlayer->OnPlayTeam())
	{
		//if the player has joined a team, use that one as default
		CTeamLookup* pLookup=GetGlobalPlayTeam(pPlayer->GetTeamID())->GetTeamLookup();

		for (int i=0;i<MAX_LOOKUP_TEAMS;i++)
		{
			if (pLookup==g_pTeamLookup[i])
			{
				iTeamLookupID=i;
				break;
			}
		}
	}
	else
	{
		iTeamLookupID = GetGlobalPlayTeam(TEAM_ONE)->GetTeamLookupID();
	}
	// Pongles ]

	m_pMdlCust=NULL;
	m_pModels->RemoveAll();

	for (i=0;i<MAX_LOOKUP_TEAMS;i++)
	{	
		m_pModels->AddItem(g_pTeamLookup[i]->GetFullName(),new KeyValues("ModelMenuItem","TeamLookupID",i));
	}

	m_pModels->ActivateItem(iTeamLookupID);

	m_bOverrideText=true;
	m_pDescription->ClearText();
	m_pDescription->SetText("Hint: use the mouse to rotate the model. Controls are:\n<Click> - Toggle rotation\n<Drag> - Rotate manually. When you resume rotation, it will start rotating in the direction you last rotated in.\n\nNOTE: Yes, this was in since IR5. I guess you just didn't notice :P");
}

void CCustomizeGUI::SetupModel(void)
{
	m_pSkins->RemoveAll();
	m_pBodygroups->RemoveAll();
	LocalSetBodygroup(-1);

	m_pModel=modelinfo->GetModel(m_pMdlCust->nModelIndex);
	m_pStuHdr=modelinfo->GetStudiomodel(m_pModel);
	m_nSkin=m_nBody=0;
	LoadModelCustomization(*m_pMdlCust,false,m_nBody,m_nSkin);
	int i;
	for (i=0;i<m_pMdlCust->aSkins.Count();i++)
	{
		if (!m_pMdlCust->aSkins[i].bLoaded)
			continue;
		if (!(m_pMdlCust->aSkins[i].iFlags&CUSTOMIZE_FL_HIDDEN))
			m_pSkins->AddItem(m_pMdlCust->aSkins[i].pszName,new KeyValues("SkinMenuItem","Skin",m_pMdlCust->aSkins[i].iSkin));
	}
	m_pSkins->ActivateItem(m_nSkin);
	for (i=0;i<m_pMdlCust->aBodygroups.Count();i++)
	{
		if (!m_pMdlCust->aBodygroups[i].bLoaded)
			continue;
		if (!(m_pMdlCust->aBodygroups[i].iFlags&CUSTOMIZE_FL_HIDDEN))
			m_pBodygroups->AddItem(m_pMdlCust->aBodygroups[i].pszName,new KeyValues("BdyGrpMenuItem","Bodygroup",m_pMdlCust->aBodygroups[i].iGroup));
	}
	m_pBodygroups->ActivateItem(0);
	m_bResetModel=true;
}

//=========================================================
//=========================================================
void CCustomizeGUI::ShowPanel( bool bShow )
{
	if(IsVisible() == bShow)
		return;

	if(bShow)
	{
		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}
}

void CCustomizeGUI::PaintModel(CModelPanel* m_pModelPanel)
{
	if(!IsVisible())
		return;

	Assert(m_pModelPanel==this->m_pModelPanel);

	m_pModelPanel->DrawModel(m_pModel,m_nBody,m_nSkin);

	if (m_bResetModel)
	{
		m_bResetModel=false;
		m_pModelPanel->ResetModel(m_pModel);
	}
}

void CCustomizeGUI::LocalSetBodygroup(int iGroup)
{
	if (iGroup==-1)
	{
		m_pSubmodels->RemoveAll();
		m_pSubmodels->SetEnabled(false);
	}
	else
	{
		m_pSubmodels->RemoveAll();
		int i;
		for (i=0;i<m_pMdlCust->aBodygroups[iGroup].aSubmodels.Count();i++)
		{
			if (!m_pMdlCust->aBodygroups[iGroup].aSubmodels[i].bLoaded)
				continue;
			if (!(m_pMdlCust->aBodygroups[iGroup].aSubmodels[i].iFlags&CUSTOMIZE_FL_HIDDEN))
				m_pSubmodels->AddItem(m_pMdlCust->aBodygroups[iGroup].aSubmodels[i].pszName,new KeyValues("SubmodelMenuItem","Bodygroup",m_pMdlCust->aBodygroups[iGroup].aSubmodels[i].iGroup,"Submodel",m_pMdlCust->aBodygroups[iGroup].aSubmodels[i].iGroupValue));
		}
		m_pSubmodels->SetEnabled(true);
		m_pSubmodels->ActivateItem(GetBodygroup(m_pStuHdr,m_nBody,iGroup));
	}
}