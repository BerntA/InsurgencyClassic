//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "clientmode_ins.h"

#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "view_scene.h"

#include "ivmodemanager.h"
#include "panelmetaclassmgr.h"
#include "insviewport.h"
#include "rendertexture.h"
#include "svgtexture.h"
#include "imc_config.h"

#include "ins_player_shared.h"
#include "playercust.h"
#include "team_lookup.h"
#include "play_team_shared.h"
#include "hud_messages.h"
#include "spectatorgui.h"
#include "insvgui.h"
#include "inshud.h"
#include "ins_shared.h"
#include "rendertexture.h"
#include "hint_helper.h"

#include "dlight.h"
#include "iefx.h"
#include "model_types.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
ConVar default_fov( "default_fov", "75", FCVAR_CHEAT );

IClientMode *g_pClientMode = NULL;

//=========================================================
//=========================================================
class CINSModeManager : public IVModeManager
{
public:
	virtual void Init( void );
	virtual void SwitchMode( bool commander, bool force ) { }
	virtual void LevelInit( const char *newmap );
	virtual void LevelShutdown( void );
	virtual void ActivateMouse( bool isactive ) { }
};

static CINSModeManager g_ModeManager;
IVModeManager *modemanager = ( IVModeManager * )&g_ModeManager;

//=========================================================
//=========================================================
#define SCREEN_FILE "scripts/vgui_screens.txt"

//=========================================================
//=========================================================
void NameChangeCallback( ConVar *var, char const *pszOldString )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( pPlayer )
		pPlayer->NameChangeCallback( var, pszOldString );
}

//=========================================================
//=========================================================
void CINSModeManager::Init( void )
{
	g_pClientMode = GetClientModeNormal( );
	
	PanelMetaClassMgr( )->LoadMetaClassDefinitionFile( SCREEN_FILE );

	// init VGUI
	GetINSVGUIHelper( )->Init( );

	// init HUD
	GetINSHUDHelper( )->Init( );

	// hint hints
	g_HintHelper.Init( );

	// find 'cl_name'
	ConVar *cl_name = ( ConVar* )ConCommandBase::FindCommand( "name" );

	if( cl_name )
		cl_name->InstallChangeCallback( NameChangeCallback );
}

//=========================================================
//=========================================================
void CINSModeManager::LevelInit( const char *newmap )
{
	g_pClientMode->LevelInit( newmap );
}

//=========================================================
//=========================================================
void CINSModeManager::LevelShutdown( void )
{
	g_pClientMode->LevelShutdown( );
	GetINSHUDHelper( )->LevelShutdown( );
}

//=========================================================
//=========================================================
ClientModeINSNormal::ClientModeINSNormal( )
{
}

//=========================================================
//=========================================================
void ClientModeINSNormal::Init( void )
{
	BaseClass::Init( );

	SVGLoadLibrary( );

	CLoadIMCHelper::CreateAllElements( );

    GetZoomTexture( );
    // GetMoBlurTex0( );

	gameeventmanager->AddListener( this, "player_spawn", false );

	BaseClass::Init( );
}

//=========================================================
//=========================================================
void ClientModeINSNormal::InitViewport( void )
{
	m_pViewport = new INSViewport( );
	m_pViewport->Start( gameuifuncs, gameeventmanager );
}

//=========================================================
//=========================================================
void ClientModeINSNormal::LevelShutdown( void )
{
	BaseClass::LevelShutdown( );
}

//=========================================================
//=========================================================
bool ClientModeINSNormal::ShouldDrawViewModel( void )
{
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon( );
	return ( pWeapon ? pWeapon->ShouldDrawViewModel( ) : NULL );
}

//=========================================================
//=========================================================
void ClientModeINSNormal::PreRender( CViewSetup *pSetup )
{
	BaseClass::PreRender( pSetup );

	materials->BeginRenderTargetAllocation( );

	// GetMoBlurTex0( );
	GetZoomTexture( );
}

//=========================================================
//=========================================================
void ClientModeINSNormal::PostRender( void )
{
	BaseClass::PostRender( );

	GetINSHUDHelper( )->SendPostRender( );
}


//=========================================================
//=========================================================
void ClientModeINSNormal::PostRenderVGui()
{
	/*Vector vecOrigin;

	CBaseHandle hEnt;
	CBaseEntity* pEnt;

	vecOrigin.Init();

	for (hEnt=g_pEntityList->FirstHandle();hEnt!=g_pEntityList->InvalidHandle();hEnt=g_pEntityList->NextHandle(hEnt))
	{
		pEnt=(CBaseEntity*)g_pEntityList->LookupEntity(hEnt);
		if (FStrEq(pEnt->GetClassName(),"C_InfoCustomizeView"))
		{
			vecOrigin=pEnt->GetAbsOrigin();
			break;
		}
	}
	
	// Make a light so the model is well lit.
	dlight_t *dl = effects->CL_AllocDlight ( LIGHT_INDEX_TE_DYNAMIC + 1 );

	dl->flags = DLIGHT_NO_WORLD_ILLUMINATION;
	dl->origin=vecOrigin;
	dl->die  = gpGlobals->curtime + 0.0001f; // Go away immediately so it doesn't light the world too.
	dl->color.r = dl->color.g = dl->color.b = 250;
	dl->radius = 400;

	CViewSetup view;
	view.x = g_aiSecondViewportSize[0];
	view.y = g_aiSecondViewportSize[1];
	view.width = g_aiSecondViewportSize[2];
	view.height = g_aiSecondViewportSize[3];

	view.m_bOrtho = false;
	view.fov = 54;

	view.origin=vecOrigin;

	view.angles.Init();
	view.m_vUnreflectedOrigin = view.origin;
	view.zNear = VIEW_NEARZ;
	view.zFar = 1000;

	Frustum dummyFrustum;
	render->Push3DView( view, 0, false, NULL, dummyFrustum );

	C_BaseAnimating* pAnimating;
	for (hEnt=ClientEntityList().FirstHandle();hEnt!=ClientEntityList().InvalidHandle();hEnt=ClientEntityList().NextHandle(hEnt))
	{
		pAnimating=(C_BaseAnimating*)ClientEntityList().GetBaseEntityFromHandle(hEnt);
		if (modelinfo->GetModelType(pAnimating->GetModel())==mod_studio) //check c_baseentity.cpp, ln 1586. if you dont do it, youre probably fucked here anyway
			if (pAnimating->ShouldDrawPostVGUI())
				pAnimating->DrawModelPostVGUI(STUDIO_RENDER|(pAnimating->IsTransparent()?STUDIO_TRANSPARENCY:0));
	}

	render->PopView( dummyFrustum );*/
}

//=========================================================
//=========================================================
void ClientModeINSNormal::ProcessInput( bool bActive )
{
	BaseClass::ProcessInput( bActive );

	if( bActive )
		GetINSHUDHelper( )->ProcessInput( );
}

//=========================================================
//=========================================================
int ClientModeINSNormal::KeyInput( int iDown, int iKeyNum, const char *pszCurrentBinding )
{
	if( BaseClass::KeyInput( iDown, iKeyNum, pszCurrentBinding ) == 0 )
		return 0;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	// if ingame spectator mode, intercept key event here
	if( pPlayer && pPlayer->GetObserverMode( ) > OBS_MODE_DEATHCAM ) 
	{
		if( iDown && pszCurrentBinding && !g_pSpectatorGUI->HandleInput( pszCurrentBinding ) )
			return 0;
	}

	return 1;
}

//=========================================================
//=========================================================
CON_COMMAND( say_start, "Bring up Chatting Input Line" )
{
	ClientModeShared *pMode = ( ClientModeShared* )GetClientModeNormal();

	if( pMode )
		pMode->StartMessageMode( 0 );
}

CON_COMMAND( say_start2, "Bring up Chatting Input Line (Team)" )
{
	ClientModeShared *pMode = ( ClientModeShared* )GetClientModeNormal( );

	if( pMode )
		pMode->StartMessageMode( SAYTYPE_TEAM );
}

CON_COMMAND( say_start3, "Bring up Chatting Input Line (Squad)" )
{
	ClientModeShared *pMode = ( ClientModeShared* )GetClientModeNormal( );

	if( pMode )
		pMode->StartMessageMode( SAYTYPE_SQUAD );
}

//=========================================================
//=========================================================
void ClientModeINSNormal::StartMessageMode( int iMessageModeType )
{
	Assert( g_pPlayerChat );
	g_pPlayerChat->StartMessageMode( iMessageModeType );
}

//=========================================================
//=========================================================
Panel *ClientModeINSNormal::GetMessagePanel( void )
{
	Assert( g_pPlayerChat );
	return g_pPlayerChat->GetInputPanel( );
}

//=========================================================
//=========================================================
void ClientModeINSNormal::FireGameEvent( IGameEvent *pEvent )
{
	BaseClass::FireGameEvent( pEvent );

	const char *pszEventName = pEvent->GetName( );

	if( Q_strcmp( "player_spawn", pszEventName ) == 0 )
	{
		// PNOTE: this is very broken when the player changes his
		// squad because it doesn't have the class information
		// in the client yet and causes the wrong model to 
		// be returned

		if( pEvent->GetBool( "dead" ) )
			return;

		C_INSPlayer *pPlayer = ToINSPlayer( USERID2PLAYER( pEvent->GetInt( "userid" ) ) );

		if ( !pPlayer || !pPlayer->IsLocalPlayer( ) )
			return;

		if( !pPlayer->IsCustomized( ) )
			return;

		modelcustomization_t &MdlCust = GetModelCustomization( pPlayer );

		if( MdlCust.bLoaded )
		{
			int a, b, c;
			LoadModelCustomization( MdlCust, true, a, b, c );
		}
		else
		{
			Warning( "Player Model Loaded, but Customization is not Initialized\n" );
		}
	}
}

//=========================================================
//=========================================================
INSViewport *ClientModeINSNormal::GetINSViewport( void )
{
	return ( INSViewport* )m_pViewport;
}

//=========================================================
//=========================================================
ClientModeINSNormal g_ClientModeNormal;

IClientMode *GetClientModeNormal( void )
{
	return &g_ClientModeNormal;
}

//=========================================================
//=========================================================
ClientModeINSNormal *GetClientModeINSNormal( void )
{
	Assert( dynamic_cast< ClientModeINSNormal* >( GetClientModeNormal() ) );
	return static_cast< ClientModeINSNormal* >( GetClientModeNormal() );
}