//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <cl_dll/iviewport.h>
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
#include <vgui_controls/menu.h>
#include <vgui_controls/menuitem.h>

#include <vgui_controls/editablepanel.h>

#include "ins_player_shared.h"

#include "animation.h"
#include "studio.h"
#include "playercust.h"
#include "c_team.h"
#include "play_team_shared.h"
#include "team_lookup.h"

#include "engine/ivmodelinfo.h"
#include "engine/ivmodelrender.h"

#include "cdll_convar.h"

#include "c_baseanimating.h"

#include "tier0/memdbgon.h"

#include "engine/ivdebugoverlay.h"

#include "insvgui_customizehelper.h"
#include "insvgui.h"

#include "view_scene.h"

using namespace vgui;

class CCustomizeGUI : public Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CCustomizeGUI, Frame);
	class CModelPanel;
	class EventComboBox;
	friend class CModelPanel;

public:
	CCustomizeGUI(IViewPort *pViewPort);

	virtual const char *GetName(void) { return PANEL_CUSTOMIZEGUI; }
	virtual void SetData(KeyValues *data) { };
	virtual void Update() { }
	virtual bool NeedsUpdate() { return false; };
	virtual bool HasInputElements(void) { return true; }
	virtual void Reset();
	virtual void ShowPanel(bool bShow);

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); };
	virtual bool IsVisible() { return BaseClass::IsVisible(); };
	virtual void SetParent(vgui::VPANEL parent) { BaseClass::SetParent(parent); };

protected:
	virtual void PerformLayout(void);
	virtual void OnCommand(const char *pszCommand);
	virtual void RequestFocus(int direction = 0);

	void Setup(void);
	void SetupModel(void);

private:
	void PaintModel(CModelPanel* m_pModelPanel);
	void LocalSetBodygroup(int iGroup);

	// Pongles [
	bool SendToServer(void);
	// Pongles ]

private:
	const model_t		*m_pModel;
	studiohdr_t			*m_pStuHdr;
	int					m_nBody;
	int					m_nSkin;
	bool				m_bResetModel;
	bool				m_bAlignButtons;
	//HACK HACK. when this is set, dont change the text in the
	//description panel until a submodels is chosen once.
	//This ensures that the hint is visible after popping up
	//the customization screen :)
	//bool				m_bOverrideText;

	CModelPanel			*m_pModelPanel;
	Label				*m_pDescription;
	Button				*m_apButtons[3];

	CUtlVector<Button*>	m_apLSelectButtons;
	CUtlVector<Button*>	m_apRSelectButtons;
	CUtlVector<Label*>	m_apSelectLabels;
};

#define VGUI_CUST_SELECTBUTTON_SIZE 16
#define VGUI_CUST_SELECTLABEL_SIZE 100

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

ConVar cl_customizeguimodelz("cl_customizeguimodelz", "60.0", FCVAR_ARCHIVE,"The size of the model in the customization window.\nAdjust between 40.0 and 100.0. Higher is smaller. Default: 60.0");
ConVar cl_customizeguirotrate("cl_customizeguirotrate", "50.0", FCVAR_ARCHIVE,"The speed of the rotation of the model in the customization window.\nAdjust between 10 and 1000. Higher is faster. Default: 50.0");

/*
class C_InfoCustomizeView : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_InfoCustomizeView, C_BaseEntity );
	DECLARE_CLIENTCLASS();
};

IMPLEMENT_CLIENTCLASS_DT( C_InfoCustomizeView, DT_InfoCustomizeView, CInfoCustomizeView )
END_RECV_TABLE()
*/

class C_PreviewModel : public C_BaseFlex
{
private:
	DECLARE_CLASS( C_PreviewModel, C_BaseFlex );

	Vector m_vecOrigin;
	Panel* m_pParentPanel;

	CUtlVector<int> m_aiAttachmentRequests;
	CUtlMap<int,Vector> m_avecAttachments;

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
	void ClearAttachementRequests()
	{
		m_aiAttachmentRequests.RemoveAll();
		m_avecAttachments.RemoveAll();
	}

	void RequestAttachment(int iAttach)
	{
		m_aiAttachmentRequests.AddToTail(iAttach);
	}

	Vector GetAttachment(int iAttach)
	{
		return m_avecAttachments[m_avecAttachments.Find(iAttach)];
	}

	bool IsModelReset()
	{
		return !m_bOldXY;
	}

	void ResetRenderOrigin()
	{
		m_avecAttachments.RemoveAll();
		m_bOldXY=true;
	};

	C_PreviewModel() : BaseClass(), m_avecAttachments(DefLessFunc(int))
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
			Vector vecOrigin;
			vecOrigin.Init();
			/*CBaseHandle hEnt;
			CBaseEntity* pEnt;
			for (hEnt=g_pEntityList->FirstHandle();hEnt!=g_pEntityList->InvalidHandle();hEnt=g_pEntityList->NextHandle(hEnt))
			{
				pEnt=(CBaseEntity*)g_pEntityList->LookupEntity(hEnt);
				if (FStrEq(pEnt->GetClassName(),"C_InfoCustomizeView"))
				{
					vecOrigin=pEnt->GetAbsOrigin();
					break;
				}
			}*/
			m_vecOrigin=vecOrigin+Vector(cl_customizeguimodelz.GetFloat(),0,-36);
			//debugoverlay->AddLineOverlay(vecOrigin,m_vecOrigin,255,0,0,true,10.0f);
			SetAbsOrigin(m_vecOrigin);
			for (int i=0;i<m_aiAttachmentRequests.Count();i++)
			{
				if (!m_avecAttachments.IsValidIndex(m_avecAttachments.Find(i)))
				{
					QAngle angAttach;
					Vector vecAttach,vec2;
					BaseClass::GetAttachment(m_aiAttachmentRequests[i],vecAttach,angAttach);
					ScreenTransform(vecAttach,vec2);
					m_avecAttachments.Insert(m_aiAttachmentRequests[i],vec2);
				}
			}
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

int g_aiSecondViewportSize[4];

class CCustomizeGUI::CModelPanel : public Panel
{
private:
	CCustomizeGUI* m_pCustGUI;
	C_PreviewModel* m_pEntity;
	friend class CCustomizeGUI;

	float m_flMouseDownTime;

	int m_OldX;
	int m_iScreenY;
public:
	CModelPanel(CCustomizeGUI* m_pCustGUI, Panel *parent, const char *panelName) : Panel(parent,panelName)
	{
		this->m_pCustGUI=m_pCustGUI;
		m_pEntity=NULL;
		SetBgColor(Color(0,0,0,255));
		m_flMouseDownTime=-1.0f;
		m_OldX=0x7fffffff;
		m_iScreenY=0;
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
		int x,y,w,h;
		GetBounds(x,y,w,h);
		w+=x;
		h+=y;
		GetParent()->LocalToScreen(x,y);
		GetParent()->LocalToScreen(w,h);
		m_iScreenY=g_aiSecondViewportSize[1]=y;
		g_aiSecondViewportSize[3]=h-y;
		g_aiSecondViewportSize[2]=g_aiSecondViewportSize[3]>>1;
		g_aiSecondViewportSize[0]=x+((w-x)>>1)-(g_aiSecondViewportSize[2]>>1);
		
		m_pEntity->ResetRenderOrigin();
		m_pEntity->SetModelPointer(pModel);
		if (pModel)
		{
			int iSequence=m_pEntity->LookupSequence("menu_idle");
			if (iSequence==-1)
				iSequence=m_pEntity->LookupSequence("Idle_AK47");
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

	/*void OnCursorMoved(int x, int y)
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
	}*/

	void SetPostChildPaintEnabled(bool state)
	{
		m_pEntity=NULL;
	}

	int GetAttachmentY(const char* pszAttach)
	{
		if (m_pEntity&&m_pEntity->IsModelReset())
		{
			int iAttach=m_pEntity->LookupAttachment(pszAttach);
			if (iAttach>=0)
			{
				Vector vecAttach=m_pEntity->GetAttachment(iAttach);
				
				int h=GetTall()>>1;
				return (1-vecAttach.y)*h;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return -2;
		}
	}
};

//CREATE_INSVIEWPORT_PANEL( CCustomizeGUI );

//=========================================================
//=========================================================
CCustomizeGUI::CCustomizeGUI(IViewPort *pViewPort) : Frame(NULL, PANEL_CUSTOMIZEGUI)
{
	SetScheme("ClientScheme");

	m_pModelPanel = new CModelPanel(this,this,"ModelPanel");

	/*m_pModels = new EventComboBox(this,this,"ModelSelect",6,false);

	m_pSkins = new EventComboBox(this,this,"SkinSelect",6,false);
	m_pBodygroups = new EventComboBox(this,this,"BdyGrpSelect",6,false);
	m_pSubmodels = new EventComboBox(this,this,"SubMdlSelect",6,false);*/

	m_pDescription = new Label(this,"SelectionDesc","");
	//m_pDescription->SetAllowMouse(false);
	m_pDescription->SetText("");

	m_apButtons[0] = new Button(this,"OKButton","");
	m_apButtons[1] = new Button(this,"CancelButton","");
	m_apButtons[2] = new Button(this,"ResetButton","");

	SetProportional(true);
	// load settings
	LoadControlSettings("Resource/UI/Frames/CustomizeGUI.res");

	// set the new parent
	//m_pModelPanel->SetParent(GetClientArea());
	/*m_pSkins->SetParent(GetClientArea());
	m_pBodygroups->SetParent(GetClientArea());
	m_pSubmodels->SetParent(GetClientArea());*/
	//m_pDescription->SetParent(GetClientArea());
	/*for (int i=0;i<sizeof(m_apButtons)/sizeof(m_apButtons[0]);i++)
	{
		m_apButtons[i]->SetParent(GetClientArea());
	}*/

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

	int wide,tall;
	GetSize(wide,tall);

	m_pModelPanel->SetBounds(VGUI_CUST_SELECTLABEL_SIZE+80,40,wide-160-VGUI_CUST_SELECTLABEL_SIZE,tall-110);

	m_pDescription->SetBounds(20,tall-90,wide-20,VGUI_CUST_SELECTBUTTON_SIZE);

	// move the buttons to the bottom-left corner
	int xpos = 10;
	int ypos = tall - 34;

	m_apButtons[1]->SetBounds(xpos, ypos, 72, 24);
	xpos += 72 + 3;
	m_apButtons[0]->SetBounds(xpos, ypos, 72, 24);
	xpos += 72 + 3;
	m_apButtons[2]->SetBounds(xpos, ypos, 72, 24);

	m_bResetModel=true;

	Repaint();
}

//=========================================================
//=========================================================
void CCustomizeGUI::OnCommand(const char *command)
{
	if(FStrEq(command, "Finish"))
	{
		GetINSVGUIHelper()->GetCustomizeHelper()->SaveModel();
		//SaveModelCustomization(*m_pMdlCust,SendToServer(),m_nBody,m_nSkin,0);

		gViewPortInterface->ShowPanel(this, false);
		SetVisible(false);
		Reset();
	}
	else if(FStrEq(command, "Cancel"))
	{
		gViewPortInterface->ShowPanel(this, false);
		SetVisible(false);
		Reset();
	}
	else if(FStrEq(command, "Reset"))
	{
		Setup();
	}
	else if(Q_strncmp(command,"sb",2)==0)
	{
		if (command[2]=='l'||command[2]=='r')
		{
			ICustomizeHelper* pCust=GetINSVGUIHelper()->GetCustomizeHelper();

			bool bDir=command[2]=='r';
			int iInd=((int)command[3])-40;
			int iCount=pCust->GetCustomizeOptionCount(iInd);
			int iCur=pCust->GetSelectedCustomizeOption(iInd);
			iCur+=bDir?1:-1;
			if (iCur<0)
				iCur=iCount-1;
			if (iCur>=iCount)
				iCur=0;
			pCust->SetSelectedCustomizeOption(iInd,iCur);
		}
	}

	BaseClass::OnCommand(command);
}

/*void CCustomizeGUI::OnMenuItemSelected(Panel *panel)
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
}*/

//=========================================================
//=========================================================
bool CCustomizeGUI::SendToServer(void)
{
	return false;/*
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();

	if(!pPlayer)
		return false;

	return (pPlayer->OnPlayTeam() ? (pPlayer->GetModelIndex() == m_pMdlCust->nModelIndex) : false);*/
}

//=========================================================
//=========================================================
void CCustomizeGUI::RequestFocus(int direction)
{
	InvalidateLayout();
	Setup();
	BaseClass::RequestFocus(direction);
}

//=========================================================
//=========================================================
void CCustomizeGUI::Setup(void)
{
	Reset();

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();
	Assert(pPlayer);

	int iTeamLookupID;

	if (pPlayer->OnPlayTeam())
	{
		//if the player has joined a team, use that one as default
		iTeamLookupID=GetGlobalPlayTeam(pPlayer->GetTeamID())->GetTeamLookupID();
	}
	else
	{
		iTeamLookupID=GetGlobalPlayTeam(TEAM_ONE)->GetTeamLookupID();
	}

	m_pModelPanel->ResetModel(NULL);

	GetINSVGUIHelper()->GetCustomizeHelper()->LoadModel(iTeamLookupID,pPlayer->GetClassID());

	SetupModel();
}

void CCustomizeGUI::SetupModel(void)
{
	m_apLSelectButtons.PurgeAndDeleteElements();
	m_apRSelectButtons.PurgeAndDeleteElements();
	m_apSelectLabels.PurgeAndDeleteElements();

	ICustomizeHelper* pCust=GetINSVGUIHelper()->GetCustomizeHelper();

	int iCount=pCust->GetCustomizeCount();

	char szTmp[5] = { 's', 'b', 0, 0, 0 };

	m_pModelPanel->m_pEntity->ClearAttachementRequests();

	m_pModel=modelinfo->GetModel(pCust->GetModelIndex());
	m_pModelPanel->ResetModel(m_pModel);

	for (int i=-1;i<iCount;i++)
	{
		szTmp[3]=i+40;
		Button* pButton;

		szTmp[2]='l';
		pButton=new Button(this,"","<",this,szTmp);
		pButton->SetBounds(0,0,VGUI_CUST_SELECTBUTTON_SIZE,VGUI_CUST_SELECTBUTTON_SIZE);
		pButton->SetVisible(false);
		pButton->SetPaintBorderEnabled(false);
		m_apLSelectButtons.AddToTail(pButton);

		szTmp[2]='r';
		pButton=new Button(this,"",">",this,szTmp);
		pButton->SetBounds(0,0,VGUI_CUST_SELECTBUTTON_SIZE,VGUI_CUST_SELECTBUTTON_SIZE);
		pButton->SetVisible(false);
		pButton->SetPaintBorderEnabled(false);
		m_apRSelectButtons.AddToTail(pButton);

		m_apSelectLabels.AddToTail(new Label(this,"",pCust->GetCustomizeName(i)));

		const char* pszAttach=pCust->GetCustomizeAttachment(i);
		if (pszAttach)
		{
			int iAttach=m_pModelPanel->m_pEntity->LookupAttachment(pszAttach);
			if (iAttach>=0)
				m_pModelPanel->m_pEntity->RequestAttachment(iAttach);
		}
	}

	m_bResetModel=true;
}

//=========================================================
//=========================================================
void CCustomizeGUI::ShowPanel( bool bShow )
{
	if(bShow)
	{
		if(IsVisible() != bShow)
			Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}
}

typedef struct button_pos_s {
	int iButton;
	int iY;
	button_pos_s(int _iButton,int _iY) : iButton(_iButton), iY(_iY) {};
} button_pos_t;

int __cdecl ButtonSort(const button_pos_t *a, const button_pos_t *b)
{
	if (a->iY<b->iY)
		return -1;
	else if (a->iY==b->iY)
		return 0;
	else
		return 1;
}

void CCustomizeGUI::PaintModel(CModelPanel* m_pModelPanel)
{
	if(!IsVisible())
		return;

	Assert(m_pModelPanel==this->m_pModelPanel);

	if (m_bResetModel)
	{
		m_bResetModel=false;
		m_bAlignButtons=true;
		m_pModelPanel->ResetModel(m_pModel);
	}

	if (m_bAlignButtons&&m_pModelPanel->m_pEntity&&m_pModelPanel->m_pEntity->IsModelReset())
	{
		m_bAlignButtons=false;
		ICustomizeHelper* pCust=GetINSVGUIHelper()->GetCustomizeHelper();

		int iCount=pCust->GetCustomizeCount();

		CUtlVector<button_pos_t> m_aButtons;

		for (int i=-1;i<iCount;i++)
		{
			const char* pszAttach=pCust->GetCustomizeAttachment(i);
			if (pszAttach)
			{
				int y,top;
				m_pModelPanel->GetPos(y,top);
				y=m_pModelPanel->GetAttachmentY(pszAttach);
				if (y>=0)
				{
					y+=top;
					m_aButtons.AddToTail(button_pos_t(i+1,y));
					m_apLSelectButtons[i+1]->SetVisible(true);
					m_apRSelectButtons[i+1]->SetVisible(true);
				}
				else
				{
					m_apLSelectButtons[i+1]->SetVisible(false);
					m_apRSelectButtons[i+1]->SetVisible(false);
				}
			}
			else
			{
				m_apLSelectButtons[i+1]->SetVisible(false);
				m_apRSelectButtons[i+1]->SetVisible(false);
			}
		}

		m_aButtons.Sort(ButtonSort);

		iCount=m_aButtons.Count();
		int yLast,xL,xR;
		m_pModelPanel->GetPos(xL,yLast);
		xL=20;
		xR=GetWide()-20-VGUI_CUST_SELECTBUTTON_SIZE;
		yLast-=VGUI_CUST_SELECTBUTTON_SIZE;
		for (int i=0;i<iCount;i++)
		{
			button_pos_t& b=m_aButtons[i];
			yLast=max(b.iY,yLast+VGUI_CUST_SELECTBUTTON_SIZE);
			m_apSelectLabels[b.iButton]->SetBounds(xL,yLast,xR+VGUI_CUST_SELECTBUTTON_SIZE-20,VGUI_CUST_SELECTBUTTON_SIZE);
			m_apSelectLabels[b.iButton]->SetZPos(1000);
			m_apLSelectButtons[b.iButton]->SetPos(xL+VGUI_CUST_SELECTLABEL_SIZE,yLast);
			m_apLSelectButtons[b.iButton]->SetZPos(1001);
			m_apRSelectButtons[b.iButton]->SetPos(xR,yLast);
			m_apRSelectButtons[b.iButton]->SetZPos(1001);
		}
	}
	
	ICustomizeHelper* pCust=GetINSVGUIHelper()->GetCustomizeHelper();

	m_pModelPanel->DrawModel(m_pModel,pCust->GetSelectedRealBody(),pCust->GetSelectedRealSkin());
}

void CCustomizeGUI::LocalSetBodygroup(int iGroup)
{
	/*if (iGroup==-1)
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
	}*/
}
/*

CON_COMMAND_F(vgui_customize,"",FCVAR_CLIENTDLL)
{
	static CCustomizeGUI* pCustomizeGUI=NULL;
	if (!pCustomizeGUI)
		pCustomizeGUI=new CCustomizeGUI(gViewPortInterface);
	gViewPortInterface->ShowPanel(PANEL_CUSTOMIZEGUI,true);
	//pCustomizeGUI->ShowPanel(true);
}*/
