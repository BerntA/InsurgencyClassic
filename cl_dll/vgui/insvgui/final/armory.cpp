#include "cbase.h"

#include "insvgui.h"
#include "insvgui_customizehelper.h"
#include "ins_panel.h"
#include "ins_imagebutton.h"

#include "view_scene.h"

#include "igameresources.h"
#include "C_Team.h"
#include "insvgui.h"
#include "ins_gamerules.h"
#include "ins_player_shared.h"
#include "insvgui.h"
#include "play_team_shared.h"
#include "ins_squad_shared.h"

#include "vgui/iinput.h"


#include "tier0/memdbgon.h"
#include "gameuipanel.h"

using namespace vgui;

CONTROL_SIZE(CB_CLASS,53,24,109,25);
CONTROL_SIZE(CB_PLAYER,53,60,109,25);

CONTROL_SIZE(BUTTON_1A,85,267,184,107);
CONTROL_SIZE(BUTTON_1B,296,267,184,107);
CONTROL_SIZE(BUTTON_2A,85,372,184,107);
CONTROL_SIZE(BUTTON_2B,296,372,184,107);
CONTROL_SIZE(BUTTON_3A,85,482,184,107);
CONTROL_SIZE(BUTTON_3B,296,482,184,107);
CONTROL_SIZE(BUTTON_4A,85,590,184,107);
CONTROL_SIZE(BUTTON_4B,296,590,184,107);

CONTROL_SIZE(MODELPANEL,606,259,301,426);
CONTROL_SIZE(WEAPONPANEL,568,31,374,134);

#define VGUI_CUST_SELECTBUTTON_SIZE_X (IntegerScale(35))
#define VGUI_CUST_SELECTBUTTON_SIZE_Y (IntegerScale(40))
#define VGUI_CUST_SELECTLABEL_SIZE 0 //100


#include "insvgui_utils.h"

extern TeamSelectionData_t g_selectionData;


//===================== CLASS BUTTON =====================

class ClassButton : public ImageButton
{
	DECLARE_CLASS_SIMPLE( ClassButton, ImageButton );

public:
	enum {
		CB_STATE_FREE=0,
		CB_STATE_TAKEN,
		CB_STATE_SELECTED,
	};

private:
	int m_iState;
	const char* m_pszImageTaken;
	const char* m_pszImageSelected;

	bool m_bColumn; // Column A or B

	class LabelMouseReflect : public Label
	{
	public:
		LabelMouseReflect(Panel *parent, const char *panelName, const char *text) : Label(parent,panelName,text)
		{

		};

		void OnCursorEntered( )
		{
			GetParent( )->OnCursorEntered( );
		};

		void OnCursorExited( )
		{
			GetParent( )->OnCursorExited( );
		};

		void OnMousePressed(MouseCode code)
		{
			GetParent( )->OnMousePressed(code);
		};

		void OnMouseDoublePressed(MouseCode code)
		{
			GetParent( )->OnMouseDoublePressed(code);
		};

		void OnMouseTriplePressed(MouseCode code)
		{
			GetParent( )->OnMouseTriplePressed(code);
		};

		void OnMouseReleased(MouseCode code)
		{
			GetParent( )->OnMouseReleased(code);
		};
	};

	LabelMouseReflect* m_pClass;
	LabelMouseReflect* m_pPlayer;

public:
	ClassButton( bool bCulumn, const char* pszImage, const char* pszImageArmed, const char* pszImageDepressed, const char* pszImageTaken, const char* pszImageSelected, Panel *parent, const char *panelName, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL ) : ImageButton(pszImage,pszImageArmed,pszImageDepressed,pszImageTaken,parent,panelName,pActionSignalTarget,pCmd)
	{
		m_pszImageTaken=pszImageTaken;
		m_bColumn = bCulumn;
		m_pszImageSelected=pszImageSelected;

		m_pClass=new LabelMouseReflect(this,"ClassLabel","");
		m_pClass->SetContentAlignment(Label::a_northeast);
		m_pPlayer=new LabelMouseReflect(this,"PlayerLabel","");
		m_pPlayer->SetContentAlignment(Label::a_northeast);

		SetState(CB_STATE_FREE);

		//SetUseCaptureMouse(false);
	}

	void ApplySchemeSettings( IScheme *pScheme )
	{
		//m_pClass->SetFont( pScheme->GetFont( "ClassNameArmory" ) );
		//m_pPlayer->SetFont( pScheme->GetFont( "PlayerNameArmory" ) );
	}

	void SetClassName(const char* pszClassName)
	{
		m_pClass->SetText(pszClassName);
	}

	void SetState(int iState, const char* pszPlayer=NULL)
	{
		m_pPlayer->SetText(pszPlayer?pszPlayer:"-EMPTY-");

		m_iState=iState;
		switch (iState)
		{
			case CB_STATE_FREE:
				SetEnabled(true);
				m_pClass->SetFgColor(Color(0,0,0,255));
				m_pPlayer->SetFgColor(Color(0,0,0,102));
				break;
			case CB_STATE_TAKEN:
				SetEnabled(false);
				SetImages(NULL,NULL,NULL,m_pszImageTaken);
				m_pClass->SetFgColor(Color(0,0,0,255));
				if(m_bColumn)
					m_pPlayer->SetFgColor(Color(140,0,0,128));
				else
					m_pPlayer->SetFgColor(Color(0,100,0,128));
				break;
			case CB_STATE_SELECTED:
				SetEnabled(false);
				SetImages(NULL,NULL,NULL,m_pszImageSelected);
				m_pClass->SetFgColor(Color(0,0,0,128));
				m_pPlayer->SetFgColor(Color(0,0,0,102));
				break;
		}
	}

	void SetScaledBounds( Panel* pPanel, int X, int Y, int W, int H )
	{
		CINSPanel* pParent=(CINSPanel*)GetParent( );
		pPanel->SetBounds(pParent->IntegerScale(X),pParent->IntegerScale(Y),pParent->IntegerScale(W),pParent->IntegerScale(H));
	}

	void SetScaledImageSize( int iImW, int iImH )
	{
		BaseClass::SetScaledImageSize(iImW,iImH);

		SCALE_CONTROL(m_pClass,CB_CLASS);
		SCALE_CONTROL(m_pPlayer,CB_PLAYER);
	}

	int GetState( void ) const
	{
		return m_iState;
	}

	void OnMouseReleased(MouseCode code)
	{
		// ensure mouse capture gets released
		if (IsUseCaptureMouseEnabled())
		{
			input()->SetMouseCapture(NULL);
		}

		if (!IsMouseClickEnabled(code))
			return;

		if (!IsSelected())
			return;

		// it has to be both enabled and (mouse over the button or using a key) to fire
		VPANEL over=input()->GetMouseOver();
		VPANEL p1=GetVPanel(),p2=m_pClass->GetVPanel(),p3=m_pPlayer->GetVPanel();
		if ( IsEnabled() && ( over == p1 || over == p2 || over == p3 ) )
		{
			DoClick();
		}
		else
		{
			SetSelected(false);
		}

		// make sure the button gets unselected
		Repaint();
	}
};

class CArmory : public CINSPanel {
	DECLARE_CLASS_SIMPLE( CArmory, CINSPanel );

	class CModelPanel;
	friend class CModelPanel;

public:
	CArmory(void*);

	void PerformLayout( );

	void Reset( );

	void Paint( );

	void OnCommand( const char *pszCommand );

	void UpdateSquadButtons();
	
private:
	//Customization:
	const model_t		*m_pModel;
	studiohdr_t			*m_pStuHdr;
	bool				m_bResetModel;
	bool				m_bAlignButtons;

	void SetupModel(void);
	void AlignButtons(void);
	void PaintModel(CModelPanel* m_pModelPanel);

	//Selection:
	int m_iSelection;

	int m_iWeapon;
	char m_szWeapon[MAX_PATH];

	//Panels:
	ClassButton *m_pButtons[8];

	CModelPanel* m_pModelPanel;
	Panel* m_pWeaponPanel;

	CUtlVector<ImageButton*>	m_apLSelectButtons;
	CUtlVector<ImageButton*>	m_apRSelectButtons;
	//CUtlVector<Label*>	m_apSelectLabels;

	CUtlMap< int, EncodedSquadData_t > m_SquadData;

	ImageButton* m_pLSelectButton;
	ImageButton* m_pRSelectButton;
};

//===================== ENTITIES =====================

ConVar cl_customizeguimodelz("cl_customizeguimodelz", "60.0", FCVAR_ARCHIVE,"The size of the model in the customization window.\nAdjust between 40.0 and 100.0. Higher is smaller. Default: 60.0");

class C_InfoCustomizeView : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_InfoCustomizeView, C_BaseEntity );
	DECLARE_CLIENTCLASS( );
};

IMPLEMENT_CLIENTCLASS_DT( C_InfoCustomizeView, DT_InfoCustomizeView, CInfoCustomizeView )
END_RECV_TABLE( )

class C_PreviewModel : public C_BaseFlex
{
private:
	DECLARE_CLASS( C_PreviewModel, C_BaseFlex );

	Vector m_vecOrigin;
	Panel* m_pParentPanel;

	CUtlVector<int> m_aiAttachmentRequests;
	CUtlMap<int,Vector> m_avecAttachments;

	bool m_bOldXY;

public:
	void ClearAttachementRequests( )
	{
		m_aiAttachmentRequests.RemoveAll( );
		m_avecAttachments.RemoveAll( );
	}

	void RequestAttachment(int iAttach)
	{
		m_aiAttachmentRequests.AddToTail(iAttach);
	}

	Vector GetAttachment(int iAttach)
	{
		return m_avecAttachments[m_avecAttachments.Find(iAttach)];
	}

	bool IsModelReset( )
	{
		return !m_bOldXY;
	}

	void ResetRenderOrigin( )
	{
		m_avecAttachments.RemoveAll( );
		m_bOldXY=true;
	};

	C_PreviewModel( ) : BaseClass( ), m_avecAttachments(DefLessFunc(int))
	{
		m_bOldXY=false;
	};

	virtual void SpawnClientEntity( )
	{
		AddEffects(EF_NODRAW);
		SetAbsAngles(QAngle(0,180,0));
		ForceClientSideAnimationOn( );
	};

	void UpdateClientSideAnimation( )
	{
		if (GetSequence( )!=-1)
			FrameAdvance(gpGlobals->frametime);
	}

	int DrawModel( int flags )
	{
		return 0;
	};

	int DrawModelPostVGUI( int flags )
	{
		if (m_bOldXY)
		{ //do this now, because this is the viewsetup we are going to use
			Vector vecOrigin;
			vecOrigin.Init( );
			CBaseHandle hEnt;
			CBaseEntity* pEnt;
			for (hEnt=g_pEntityList->FirstHandle( );hEnt!=g_pEntityList->InvalidHandle( );hEnt=g_pEntityList->NextHandle(hEnt))
			{
				pEnt=(CBaseEntity*)g_pEntityList->LookupEntity(hEnt);
				if (FStrEq(pEnt->GetClassName( ),"C_InfoCustomizeView"))
				{
					vecOrigin=pEnt->GetAbsOrigin( );
					break;
				}
			}
			m_vecOrigin=vecOrigin+Vector(cl_customizeguimodelz.GetFloat( ),0,-36);
			SetAbsOrigin(m_vecOrigin);
			for (int i=0;i<m_aiAttachmentRequests.Count( );i++)
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
			GetModelInstance( ),
			index, 
			GetModel( ),
			GetRenderOrigin( ),
			GetRenderAngles( ),
			m_nSkin,
			m_nBody,
			0,
			NULL,
			NULL );
	};

	bool ShouldDrawPostVGUI( )
	{
		return ipanel( )->IsFullyVisible(m_pParentPanel->GetVPanel( ));
	};

	bool ShouldDraw( )
	{
		return false;
	};

	void SetParentPanel(Panel* panel)
	{
		m_pParentPanel=panel;
	};

	virtual void Release( )
	{
		m_pParentPanel->SetPostChildPaintEnabled(false);
		BaseClass::Release( );
	};
};

LINK_ENTITY_TO_CLASS( client_preview_model, C_PreviewModel );

//===================== MODEL PANEL =====================

int g_aiSecondViewportSize[4];

class CArmory::CModelPanel : public Panel
{
	DECLARE_CLASS_SIMPLE( CModelPanel, Panel );

	CArmory* m_pArmory;
	C_PreviewModel* m_pEntity;
	friend class CArmory;

	int m_iScreenY;
public:
	CModelPanel(CArmory* pArmory, Panel *parent, const char *panelName) : Panel(parent,panelName)
	{
		this->m_pArmory=pArmory;
		m_pEntity=NULL;
		m_iScreenY=0;
		SetPaintBackgroundEnabled(false);
	};

	virtual void Paint( )
	{
		m_pArmory->PaintModel(this);
	}

	void ResetModel(const model_t* pModel)
	{
		if (!m_pEntity)
		{
			m_pEntity=new C_PreviewModel( ); 
			m_pEntity->InitializeAsClientEntity(NULL,RENDER_GROUP_OPAQUE_ENTITY);
			m_pEntity->SetParentPanel(this);
		}
		int x,y,w,h;
		GetBounds(x,y,w,h);
		w+=x;
		h+=y;
		GetParent( )->LocalToScreen(x,y);
		GetParent( )->LocalToScreen(w,h);
		m_iScreenY=g_aiSecondViewportSize[1]=y;
		g_aiSecondViewportSize[3]=h-y;
		g_aiSecondViewportSize[2]=g_aiSecondViewportSize[3]>>1;
		g_aiSecondViewportSize[0]=x+((w-x)>>1)-(g_aiSecondViewportSize[2]>>1);

		m_pEntity->ResetRenderOrigin( );
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

	void SetPostChildPaintEnabled(bool state)
	{
		m_pEntity=NULL;
	}

	int GetAttachmentY(const char* pszAttach)
	{
		if (m_pEntity&&m_pEntity->IsModelReset( ))
		{
			int iAttach=m_pEntity->LookupAttachment(pszAttach);
			if (iAttach>=0)
			{
				Vector vecAttach=m_pEntity->GetAttachment(iAttach);

				int h=GetTall( )>>1;
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

//===================== ARMORY PANEL =====================

CREATE_INSVIEWPORT_PANEL( CArmory );

bool SquadDataLess( const int &iLeft, const int &iRight )
{
	return ( iLeft > iRight );
}

class CArmory *g_pArmory = NULL;

CArmory::CArmory( void * ) : CINSPanel( NULL, PANEL_CUSTOMIZEGUI, "armory" )
{
	m_pButtons[0]=new ClassButton( true, "VGUI/armory/1a_optional","VGUI/armory/1a_optional_hover","VGUI/armory/1a_optional","VGUI/armory/1a_taken","VGUI/armory/1a_selected",this,"SelectButton1a",this,"button1");
	m_pButtons[1]=new ClassButton( true, "VGUI/armory/2a_optional","VGUI/armory/2a_optional_hover","VGUI/armory/2a_optional","VGUI/armory/2a_taken","VGUI/armory/2a_selected",this,"SelectButton2a",this,"button2");
	m_pButtons[2]=new ClassButton( true, "VGUI/armory/3a_optional","VGUI/armory/3a_optional_hover","VGUI/armory/3a_optional","VGUI/armory/3a_taken","VGUI/armory/3a_selected",this,"SelectButton3a",this,"button3");
	m_pButtons[3]=new ClassButton( true, "VGUI/armory/4a_optional","VGUI/armory/4a_optional_hover","VGUI/armory/4a_optional","VGUI/armory/4a_taken","VGUI/armory/4a_selected",this,"SelectButton4a",this,"button4");
	m_pButtons[4]=new ClassButton( false, "VGUI/armory/1b_optional","VGUI/armory/1b_optional_hover","VGUI/armory/1b_optional","VGUI/armory/1b_taken","VGUI/armory/1b_selected",this,"SelectButton1b",this,"button5");
	m_pButtons[5]=new ClassButton( false, "VGUI/armory/2b_optional","VGUI/armory/2b_optional_hover","VGUI/armory/2b_optional","VGUI/armory/2b_taken","VGUI/armory/2b_selected",this,"SelectButton2b",this,"button6");
	m_pButtons[6]=new ClassButton( false, "VGUI/armory/3b_optional","VGUI/armory/3b_optional_hover","VGUI/armory/3b_optional","VGUI/armory/3b_taken","VGUI/armory/3b_selected",this,"SelectButton3b",this,"button7");
	m_pButtons[7]=new ClassButton( false, "VGUI/armory/4b_optional","VGUI/armory/4b_optional_hover","VGUI/armory/4b_optional","VGUI/armory/4b_taken","VGUI/armory/4b_selected",this,"SelectButton4b",this,"button8");

	m_pModelPanel=new CModelPanel(this,this,"ModelPanel");
	m_pModelPanel->SetBgColor(Color(0,255,0,128));
	m_pWeaponPanel=new Panel(this,"WeaponPanel");
	m_pWeaponPanel->SetBgColor(Color(0,255,0,128));
	m_pWeaponPanel->SetVisible(false);

	m_pLSelectButton=new ImageButton("VGUI/arrows/left_optional","VGUI/arrows/left_optional_hover","VGUI/arrows/left_optional","VGUI/arrows/left_cantclick",this,"sbl&",this,"sbl&");
	m_pLSelectButton->SetVisible(false);
	m_pRSelectButton=new ImageButton("VGUI/arrows/right_optional","VGUI/arrows/right_optional_hover","VGUI/arrows/right_optional","VGUI/arrows/right_cantclick",this,"sbr&",this,"sbr&");
	m_pRSelectButton->SetVisible(false);

	m_iWeapon=-1;

	m_SquadData.SetLessFunc( SquadDataLess );

	g_pArmory = this;
}

void CArmory::PerformLayout( void )
{
	BaseClass::PerformLayout( );

	SCALE_CONTROL(m_pButtons[0],BUTTON_1A);
	m_pButtons[0]->SetScaledImageSize(IntegerScale(256),IntegerScale(128));
	SCALE_CONTROL(m_pButtons[1],BUTTON_2A);
	m_pButtons[1]->SetScaledImageSize(IntegerScale(256),IntegerScale(128));
	SCALE_CONTROL(m_pButtons[2],BUTTON_3A);
	m_pButtons[2]->SetScaledImageSize(IntegerScale(256),IntegerScale(128));
	SCALE_CONTROL(m_pButtons[3],BUTTON_4A);
	m_pButtons[3]->SetScaledImageSize(IntegerScale(256),IntegerScale(128));
	SCALE_CONTROL(m_pButtons[4],BUTTON_1B);
	m_pButtons[4]->SetScaledImageSize(IntegerScale(256),IntegerScale(128));
	SCALE_CONTROL(m_pButtons[5],BUTTON_2B);
	m_pButtons[5]->SetScaledImageSize(IntegerScale(256),IntegerScale(128));
	SCALE_CONTROL(m_pButtons[6],BUTTON_3B);
	m_pButtons[6]->SetScaledImageSize(IntegerScale(256),IntegerScale(128));
	SCALE_CONTROL(m_pButtons[7],BUTTON_4B);
	m_pButtons[7]->SetScaledImageSize(IntegerScale(256),IntegerScale(128));

	SCALE_CONTROL(m_pModelPanel,MODELPANEL);
	SCALE_CONTROL(m_pWeaponPanel,WEAPONPANEL);
	int xL,xR,y,w,h;
	m_pWeaponPanel->GetBounds(xL,y,xR,h);
	xR+=xL;
	w=VGUI_CUST_SELECTBUTTON_SIZE_X;
	y+=(h-VGUI_CUST_SELECTBUTTON_SIZE_Y)>>1;
	h=VGUI_CUST_SELECTBUTTON_SIZE_Y;
	m_pLSelectButton->SetBounds(xL-w,y,w,h);
	m_pLSelectButton->SetScaledImageSize(IntegerScale(64),IntegerScale(64));
	m_pRSelectButton->SetBounds(xR,y,w,h);
	m_pRSelectButton->SetScaledImageSize(IntegerScale(64),IntegerScale(64));
}


void CArmory::Reset( void )
{
	m_iSelection=0;
	m_pModel=NULL;
	m_pStuHdr=NULL;
	m_bAlignButtons=m_bResetModel=false;

	m_pModelPanel->ResetModel(NULL);
	SetupModel( );

	m_apLSelectButtons.PurgeAndDeleteElements( );
	m_apRSelectButtons.PurgeAndDeleteElements( );

	m_pLSelectButton->SetVisible( false );
	m_pRSelectButton->SetVisible( false );

	UpdateSquadButtons();
}


void CArmory::SetupModel(void)
{
	int iSelectedTeam=0,iSelectedClass=0;
	AssertMsg(false,"Team/Class not determined!");
	GetINSVGUIHelper( )->GetCustomizeHelper( )->LoadModel(iSelectedTeam,iSelectedClass);

	ICustomizeHelper* pCust=GetINSVGUIHelper( )->GetCustomizeHelper( );

	int iCount=pCust->GetCustomizeCount( );

	m_pModelPanel->m_pEntity->ClearAttachementRequests( );

	m_pModel=modelinfo->GetModel(pCust->GetModelIndex( ));
	m_pModelPanel->ResetModel(m_pModel);

	for (int i=-1;i<iCount;i++)
	{
		const char* pszAttach=pCust->GetCustomizeAttachment(i);
		if (pszAttach)
		{
			int iAttach=m_pModelPanel->m_pEntity->LookupAttachment(pszAttach);
			if (iAttach>=0)
				m_pModelPanel->m_pEntity->RequestAttachment(iAttach);
		}
	}

	Q_snprintf(m_szWeapon,MAX_PATH,"VGUI/armory/weapons/%s",pCust->GetCustomizeImage(-2));

	m_bResetModel=true;
}

void CArmory::PaintModel(CModelPanel* m_pModelPanel)
{
	if(!IsVisible( ))
		return;

	Assert(m_pModelPanel==this->m_pModelPanel);

	if (m_bResetModel)
	{
		m_bResetModel=false;
		m_bAlignButtons=true;
		m_pModelPanel->ResetModel(m_pModel);
	}

	if (m_bAlignButtons&&m_pModelPanel->m_pEntity&&m_pModelPanel->m_pEntity->IsModelReset( ))
	{
		m_bAlignButtons=false;
		AlignButtons( );	
	}

	ICustomizeHelper* pCust=GetINSVGUIHelper( )->GetCustomizeHelper( );

	m_pModelPanel->DrawModel(m_pModel,pCust->GetSelectedRealBody( ),pCust->GetSelectedRealSkin( ));
}

typedef struct button_pos_s {
	ImageButton *pLButton,*pRButton;
	int iY;
	button_pos_s(ImageButton* _pLButton,ImageButton* _pRButton,int _iY) : pLButton(_pLButton), pRButton(_pRButton), iY(_iY) {};
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

void CArmory::AlignButtons(void)
{
	m_apLSelectButtons.PurgeAndDeleteElements( );
	m_apRSelectButtons.PurgeAndDeleteElements( );
	//m_apSelectLabels.PurgeAndDeleteElements( );

	char szTmp[5] = { 's', 'b', 0, 0, 0 };

	ICustomizeHelper* pCust=GetINSVGUIHelper( )->GetCustomizeHelper( );

	int iCount=pCust->GetCustomizeCount( );

	CUtlVector<button_pos_t> m_aButtons;

	for (int i=-1;i<iCount;i++)
	{
		szTmp[3]=i+40;
		ImageButton *pLButton,*pRButton;

		szTmp[2]='l';
		pLButton=new ImageButton("VGUI/arrows/left_optional","VGUI/arrows/left_optional_hover","VGUI/arrows/left_optional","VGUI/arrows/left_cantclick",this,szTmp,this,szTmp);
		pLButton->SetBounds(0,0,VGUI_CUST_SELECTBUTTON_SIZE_X,VGUI_CUST_SELECTBUTTON_SIZE_Y);
		pLButton->SetScaledImageSize(IntegerScale(64),IntegerScale(64));
		pLButton->SetVisible(false);
		pLButton->SetEnabled(true);
		m_apLSelectButtons.AddToTail(pLButton);

		szTmp[2]='r';
		pRButton=new ImageButton("VGUI/arrows/right_optional","VGUI/arrows/right_optional_hover","VGUI/arrows/right_optional","VGUI/arrows/right_cantclick",this,szTmp,this,szTmp);
		pRButton->SetBounds(0,0,VGUI_CUST_SELECTBUTTON_SIZE_X,VGUI_CUST_SELECTBUTTON_SIZE_Y);
		pRButton->SetScaledImageSize(IntegerScale(64),IntegerScale(64));
		pRButton->SetVisible(false);
		pRButton->SetEnabled(true);
		m_apRSelectButtons.AddToTail(pRButton);

		//m_apSelectLabels.AddToTail(new Label(this,"",pCust->GetCustomizeName(i)));

		const char* pszAttach=pCust->GetCustomizeAttachment(i);
		if (pszAttach)
		{
			int y,top;
			m_pModelPanel->GetPos(y,top);
			y=m_pModelPanel->GetAttachmentY(pszAttach);
			if (y>=0)
			{
				y+=top;
				m_aButtons.AddToTail(button_pos_t(pLButton,pRButton,y));
				pLButton->SetVisible(true);
				pRButton->SetVisible(true);
			}
			else
			{
				pLButton->SetVisible(false);
				pRButton->SetVisible(false);
			}
		}
		else
		{
			pLButton->SetVisible(false);
			pRButton->SetVisible(false);
		}
	}

	m_pLSelectButton->SetVisible(true);
	m_pRSelectButton->SetVisible(true);

	m_aButtons.Sort(ButtonSort);

	iCount=m_aButtons.Count( );
	int yLast,xL,xR;
	int x,y,w,h;
	m_pModelPanel->GetBounds(x,y,w,h);
	xL=x-VGUI_CUST_SELECTBUTTON_SIZE_X;
	xR=x+w;
	yLast=y-VGUI_CUST_SELECTBUTTON_SIZE_Y;
	for (int i=0;i<iCount;i++)
	{
		button_pos_t& b=m_aButtons[i];
		yLast=max(b.iY,yLast+VGUI_CUST_SELECTBUTTON_SIZE_Y);
		//m_apSelectLabels[b.iButton]->SetBounds(xL,yLast,xR+VGUI_CUST_SELECTBUTTON_SIZE-20,VGUI_CUST_SELECTBUTTON_SIZE);
		//m_apSelectLabels[b.iButton]->SetZPos(1000);
		b.pLButton->SetPos(xL,yLast);
		b.pLButton->SetZPos(1001);
		b.pRButton->SetPos(xR,yLast);
		b.pRButton->SetZPos(1001);
	}
}

void CArmory::OnCommand( const char *pszCommand )
{
	//Handle button clicks

	if( FStrEq( pszCommand, "deploy" ) )
	{
		// close the panel
		ShowPanel( false );

		EncodedSquadData_t pSquadData =  SquadData_t( g_selectionData.m_iSquadID, g_selectionData.m_iClassID ).GetEncodedData( );

		GetINSVGUIHelper( )->JoinFull( g_selectionData.m_iTeamID, pSquadData );

		// join the server!
		g_pGameUIPanel->JoinServer( );
	}
	else if(Q_strncmp(pszCommand,"sb",2)==0)
	{
		if (pszCommand[2]=='l'||pszCommand[2]=='r')
		{
			ICustomizeHelper* pCust=GetINSVGUIHelper( )->GetCustomizeHelper( );

			bool bDir=pszCommand[2]=='r';
			int iInd=((int)pszCommand[3])-40;
			int iCount=pCust->GetCustomizeOptionCount(iInd);
			int iCur=pCust->GetSelectedCustomizeOption(iInd);
			iCur+=bDir?1:-1;
			if (iCur<0)
				iCur=iCount-1;
			if (iCur>=iCount)
				iCur=0;
			pCust->SetSelectedCustomizeOption(iInd,iCur);
			if (iInd==-2)
			{
				Q_snprintf(m_szWeapon,MAX_PATH,"VGUI/armory/weapons/%s",pCust->GetCustomizeImage(-2));
				UseTexture(m_iWeapon,m_szWeapon,true);
			}
		}
	}
	else if(Q_strncmp(pszCommand,"button",6)==0)
	{
		g_selectionData.m_iClassID = pszCommand[6] - '1';
		UpdateSquadButtons();
	}
}

void CArmory::Paint( )
{
	BaseClass::Paint( );

	UseTexture(m_iWeapon,m_szWeapon);
	int x,y,w,h;
	m_pWeaponPanel->GetBounds(x,y,w,h);
	w+=x;
	h+=y;
	vgui::surface( )->DrawTexturedSubRect(x, y, w, h, 0.0f, 0.0f, 399.0f/512.0f, 147.0f/256.0f );
}

void CArmory::UpdateSquadButtons()
{
	IGameResources *gr = GameResources( );

	if( !gr )
		return;

	C_PlayTeam *pTeam = GetGlobalPlayTeam( g_selectionData.m_iTeamID );

	if( !pTeam )
		return;

	C_INSSquad *pSquad = pTeam->GetSquad( g_selectionData.m_iSquadID );

	if( !pSquad )
		return;

	int idx,iPlayer=GetLocalPlayerIndex();

	for( int j = 0; j < MAX_SQUAD_SLOTS; j++ )
	{
		idx=pSquad->GetPlayerID(j);
		bool bSelectable = iPlayer==idx||idx==-1;

		CPlayerClass *pClass = pSquad->GetClass( j );
		m_pButtons[j]->SetClassName( pClass ? pClass->GetName( ) : "None" );
		if(bSelectable)
		{
			if (j==g_selectionData.m_iClassID)
			{
				m_pButtons[j]->SetState( ClassButton::CB_STATE_SELECTED, gr->GetPlayerName( iPlayer ) );
			}
			else
			{
				m_pButtons[j]->SetState( ClassButton::CB_STATE_FREE );
			}
		}
		else
		{
			m_pButtons[j]->SetState( ClassButton::CB_STATE_TAKEN, gr->GetPlayerName( idx ) );
		}
	}
}
