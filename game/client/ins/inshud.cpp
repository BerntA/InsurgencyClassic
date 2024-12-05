//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include "inshud.h"
#include "ins_gamerules.h"
#include "play_team_shared.h"
#include "ins_squad_shared.h"
#include "c_playerresource.h"
#include "hud_macros.h"
#include "hlscolor.h"
#include "basic_colors.h"
#include "ins_player_shared.h"
#include <cl_dll/iviewport.h>
#include "view.h"
#include "in_buttons.h"
#include "iinput.h"
#include "pain_helper.h"
#include "imc_config.h"
#include "datacache/imdlcache.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
void CC_ScrollUp( void )
{
	GetINSHUDHelper( )->SendScrollUpdate( INSHUD_SCROLLUP );
}

ConCommand scrollup( "scrollup", CC_ScrollUp );

void CC_ScrollDown( void )
{
	GetINSHUDHelper( )->SendScrollUpdate( INSHUD_SCROLLDOWN );
}

ConCommand scrolldown( "scrolldown", CC_ScrollDown );

//=========================================================
//=========================================================
#define DECLATE_SLOTNUMBER( value ) \
	void CC_slot##value( void ) { \
	GetINSHUDHelper( )->SendNumberUpdate( value ); } \
	ConCommand slot##value( "slot" #value, CC_slot##value );

DECLATE_SLOTNUMBER( 0 );
DECLATE_SLOTNUMBER( 1 );
DECLATE_SLOTNUMBER( 2 );
DECLATE_SLOTNUMBER( 3 );
DECLATE_SLOTNUMBER( 4 );
DECLATE_SLOTNUMBER( 5 );
DECLATE_SLOTNUMBER( 6 );
DECLATE_SLOTNUMBER( 7 );
DECLATE_SLOTNUMBER( 8 );
DECLATE_SLOTNUMBER( 9 );

//=========================================================
//=========================================================
bool IINSHUDElement::ShouldDraw( void )
{
	// TODO: finish
	return true;
}

//=========================================================
//=========================================================
CINSHUDElementHelper *CINSHUDElementHelper::m_pHelpers = NULL;

CINSHUDElementHelper::CINSHUDElementHelper( CreateElement_t Helper )
{
	if( m_pHelpers == NULL )
	{
		m_pNext = m_pHelpers;
		m_pHelpers = this;
	}
	else
	{
		CINSHUDElementHelper *pPrev = m_pHelpers;
		CINSHUDElementHelper *pCurrent = m_pHelpers->m_pNext;

		while( pCurrent != NULL )
		{
			pPrev = pCurrent;
			pCurrent = pCurrent->m_pNext;
		}

		pPrev->m_pNext = this;
		m_pNext = pCurrent;
	}

	Assert( Helper );
	m_Helper = Helper;
}

//=========================================================
//=========================================================
void CINSHUDElementHelper::CreateAllElements( void )
{
	CINSHUDElementHelper *pHelper = m_pHelpers;

	while( pHelper )
	{
		CreateElement_t Helper = pHelper->m_Helper;

		IINSHUDElement *pNewElement = ( Helper )( );
		Assert( pNewElement );

		if( pNewElement )
			GetINSHUDHelper( )->AddHUDElement( pNewElement );

		pHelper = pHelper->GetNext( );
	}
}

//=========================================================
//=========================================================
#define DEFINE_HUDLISTENER( classname, objectname ) \
	CUtlVector< classname* > objectname; \
	classname::classname( ) { \
	objectname##.AddToTail( this ); } \
	classname::~##classname( ) { \
	objectname##.FindAndRemove( this ); }

DEFINE_HUDLISTENER( IINSMeshDraw, g_MeshDrawers );
DEFINE_HUDLISTENER( IINSOrderListener, g_OrderListeners );
DEFINE_HUDLISTENER( IINSTeamListener, g_TeamListeners );
DEFINE_HUDLISTENER( IINSDamageListener, g_DamageListeners );
DEFINE_HUDLISTENER( IINSObjListener, g_ObjListeners );
DEFINE_HUDLISTENER( IINSPostRenderListener, g_RenderListeners );
DEFINE_HUDLISTENER( IINSChatListener, g_ChatListeners );
DEFINE_HUDLISTENER( IINSFireMode, g_FMListeners );
DEFINE_HUDLISTENER( IINSPlayerDeath, g_DeathListeners );
DEFINE_HUDLISTENER( IINSReinforcement, g_ReinforcementListeners );

//=========================================================
//=========================================================
void IINSControlListener::ControlTakeFocus( void )
{
	GetINSHUDHelper( )->TakeControlFocus( this );
}

//=========================================================
//=========================================================
void IINSControlListener::ControlClose( void )
{
	DoControlClose( );

	GetINSHUDHelper( )->RemoveControlFocus( );
}

//=========================================================
//=========================================================
void __MsgFunc_Pain( bf_read &msg )
{
	GetINSHUDHelper( )->SendPainUpdate( msg );
}

//=========================================================
//=========================================================
void __MsgFunc_Damage( bf_read &msg )
{
	GetINSHUDHelper( )->SendDamageUpdate( msg );
}

//=========================================================
//=========================================================
void __MsgFunc_SayText( bf_read &msg )
{
	GetINSHUDHelper( )->SendChatUpdate( msg );
}

//=========================================================
//=========================================================
void __MsgFunc_FireMode( bf_read &msg )
{
	GetINSHUDHelper( )->SendFireModeUpdate( msg );
}

//=========================================================
//=========================================================
void __MsgFunc_ReinforceMsg( bf_read &msg )
{
	GetINSHUDHelper( )->SendReinforcementUpdate( msg );
}

//=========================================================
//=========================================================
CINSHUDHelper::CINSHUDHelper( )
{
	m_pUnitOrder = NULL;
	m_pWeaponInfo = NULL;
	m_pChatMessages = NULL;

	m_pDefaultController = m_pActiveController = NULL;

	m_pUnitOrderModel = NULL;
}

//=========================================================
//=========================================================
CINSHUDHelper *CINSHUDHelper::GetINSHUDHelper( void )
{
	static CINSHUDHelper *pHUDHelper = NULL;

	if( !pHUDHelper )
		pHUDHelper = new CINSHUDHelper;

	return pHUDHelper;
}

//=========================================================
//=========================================================
#define TARGET_LUMINANCE 0.9f

void CINSHUDHelper::Init( void )
{
	// hook messages
	HOOK_MESSAGE( Damage );
	HOOK_MESSAGE( Pain );
	HOOK_MESSAGE( SayText );
	HOOK_MESSAGE( FireMode );
	HOOK_MESSAGE( ReinforceMsg );

	// setup health colors
	for( int i = 0; i < HEALTHTYPE_COUNT; i++ )
	{
		HLSColor WColor( GetHealthColor( i ) );
		WColor.m_flLuminance = TARGET_LUMINANCE;

		m_HealthColors[ i ] = WColor.ConvertToRGB( );
	}

	// init elements
	CINSHUDElementHelper::CreateAllElements( );

	for( int i = 0; i < m_HUDElements.Count( ); i++ )
		m_HUDElements[ i ]->Init( );
}

//=========================================================
//=========================================================
void CINSHUDHelper::LevelShutdown( void )
{
	//m_HUDElements.PurgeAndDeleteElements( );
	m_pUnitOrderModel = NULL;
}

//=========================================================
//=========================================================
void CINSHUDHelper::ProcessInput( void )
{
	if( !m_pActiveController )
		return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( !pPlayer || ( gHUD.m_iKeyBits & IN_ATTACK ) == 0 )
		return;

	bool bControlActive = m_pActiveController->IsControlActive( );

	if( !pPlayer->IsInVGuiInputMode( ) && !bControlActive )
		return;

	::input->ClearInputButton( IN_ATTACK );

	IINSControlListener *pOldController = m_pActiveController;

	if( bControlActive )
		m_pActiveController->Selection( );
	
	if( m_pActiveController == pOldController )
		m_pActiveController->ControlClose( );
}

//=========================================================
//=========================================================
void CINSHUDHelper::RegisterUnitOrder( IINSUnitOrder *pElement )
{
	m_pUnitOrder = pElement;
}

//=========================================================
//=========================================================
void CINSHUDHelper::RegisterWeaponInfo( IINSWeaponInfo *pElement )
{
	m_pWeaponInfo = pElement;
}

//=========================================================
//=========================================================
void CINSHUDHelper::RegisterChatMessages( IINSChatMessages *pElement )
{
	m_pChatMessages = pElement;
}

//=========================================================
//=========================================================
void CINSHUDHelper::DrawHUDMesh( void )
{
	for( int i = 0; i < g_MeshDrawers.Count( ); i++ )
		g_MeshDrawers[ i ]->Draw( );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendObjOrderUpdate( void )
{
	for( int i = 0; i < g_OrderListeners.Count( ); i++ )
		g_OrderListeners[ i ]->ObjOrder( );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendUnitOrderUpdate( void )
{
	for( int i = 0; i < g_OrderListeners.Count( ); i++ )
		g_OrderListeners[ i ]->UnitOrder( );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendTeamUpdate( C_PlayTeam *pTeam )
{
	if( !pTeam )
		return;

	for( int i = 0; i < g_TeamListeners.Count( ); i++ )
		g_TeamListeners[ i ]->TeamUpdate( pTeam );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendDamageUpdate( bf_read &msg )
{
	int iAmount = msg.ReadByte( );
	int iBits = msg.ReadLong( );

	Vector vecFrom = vec3_origin;

	if( iBits & DMG_VALIDFORCE )
	{
		vecFrom.x = msg.ReadFloat( );
		vecFrom.y = msg.ReadFloat( );
		vecFrom.z = msg.ReadFloat( );
	}

	for( int i = 0; i < g_DamageListeners.Count( ); i++ )
		g_DamageListeners[ i ]->DamageTaken( iAmount, iBits, vecFrom );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendPainUpdate( bf_read &msg )
{
	g_PainHelper.CreatePain( msg.ReadByte( ) - 1 );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendObjUpdate( C_INSObjective *pObjective )
{
	for( int i = 0; i < g_ObjListeners.Count( ); i++ )
		g_ObjListeners[ i ]->ObjUpdate( pObjective );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendPostRender( void )
{
	for( int i = 0; i < g_RenderListeners.Count( ); i++ )
		g_RenderListeners[ i ]->PostRender( );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendChatUpdate( bf_read &msg )
{
	int iType, iSenderID;
	bool bThirdPerson = false;

	// collect information
	iType = msg.ReadByte( );
	iSenderID = 0;

	if( iType != SAYTYPE_SERVER )
	{
		bThirdPerson = msg.ReadOneBit( ) ? true : false;
		iSenderID = msg.ReadByte( );
	}

	char szString[ MAX_CHATMSG_LENGTH ];
	msg.ReadString( szString, sizeof( szString ) );

	const char *pszString = UTIL_CleanChatString( szString );

	// parse information
	if( !pszString )
		return;

	CColoredString ParsedString;
	UTIL_ParseChatMessage( ParsedString, iSenderID, iType, bThirdPerson, pszString );

	for( int i = 0; i < g_RenderListeners.Count( ); i++ )
		g_ChatListeners[ i ]->PrintChat( ParsedString, iType );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendFireModeUpdate( bf_read &msg )
{
	int iMode = msg.ReadByte( );

	for( int i = 0; i < g_FMListeners.Count( ); i++ )
		g_FMListeners[ i ]->FireMode( iMode );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendDeathUpdate( void )
{
	for( int i = 0; i < g_DeathListeners.Count( ); i++ )
		g_DeathListeners[ i ]->PlayerDeath( );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendReinforcementUpdate( bf_read &msg )
{
	int iRemaining = msg.ReadByte( );

	for( int i = 0; i < g_ReinforcementListeners.Count( ); i++ )
		g_ReinforcementListeners[ i ]->ReinforcementDeployed( iRemaining );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendEmergencyUpdate( void )
{
	for( int i = 0; i < g_ReinforcementListeners.Count( ); i++ )
		g_ReinforcementListeners[ i ]->EmergencyDeployment( );
}

//=========================================================
//=========================================================
void CINSHUDHelper::ReinforcementCall( void )
{
	engine->ServerCmd( GRCMD_CREINFORCE, true );
}

//=========================================================
//=========================================================
void CINSHUDHelper::ObjectiveOrder( int iObjID )
{
	char szCommand[ 32 ];
	sprintf( szCommand, "%s %i", GRCMD_OBJORDERS, iObjID );

	engine->ServerCmd( szCommand );
}

//=========================================================
//=========================================================
void CINSHUDHelper::UnitOrder( int iOrderType, const Vector &vecPosition )
{
	char szCommand[ 128 ];
	Q_snprintf( szCommand, sizeof( szCommand ), "%s %i %.2f %.2f %.2f", GRCMD_UNITORDERS, iOrderType, vecPosition.x, vecPosition.y, vecPosition.z );

	engine->ServerCmd( szCommand );
}

//=========================================================
//=========================================================
void CINSHUDHelper::PlayerOrder( int iOrderID )
{
	char szCommand[ 64 ];
	Q_snprintf( szCommand, sizeof( szCommand ), "%s %i", GRCMD_PLAYERORDERS, iOrderID );

	engine->ServerCmd( szCommand );
}

//=========================================================
//=========================================================
void CINSHUDHelper::PlayerOrderResponse( int iResponseID )
{
	char szCommand[ 64 ];
	Q_snprintf( szCommand, sizeof( szCommand ), "%s %i", PCMD_PORESPONSE, iResponseID );

	engine->ServerCmd( szCommand );
}

//=========================================================
//=========================================================
void CINSHUDHelper::StatusBroadcast( void )
{
	engine->ServerCmd( PCMD_STATUSBCAST );
}

//=========================================================
//=========================================================
void CINSHUDHelper::PlayerHelp( void )
{
	engine->ServerCmd( PCMD_NEEDSHELP );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendChat( const char *pszText )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	// copy text
	char szTextBuffer[ MAX_CHATMSG_LENGTH ];
    Q_strncpy( szTextBuffer, pszText, MAX_CHATMSG_LENGTH );

	// handle the string
	bool bThirdPerson = false;
	const char *pszHandledText = UTIL_HandleChatString( szTextBuffer, bThirdPerson );

	if( !pszHandledText )
		return;

	// ... command and type
	char szSendText[ MAX_CHATBUFFER_LENGTH ];
	Q_snprintf( szSendText, sizeof( szSendText ), "say2 %i %s", bThirdPerson ? 1 : 0, pszHandledText );

	// send to the server
	engine->ServerCmd( szSendText );
}

//=========================================================
//=========================================================
IINSWeaponInfo *CINSHUDHelper::WeaponInfo( void ) const
{
	Assert( m_pWeaponInfo );
	return m_pWeaponInfo;
}

//=========================================================
//=========================================================
IINSChatMessages *CINSHUDHelper::ChatMessages( void ) const
{
	Assert( m_pChatMessages );
	return m_pChatMessages;
}

//=========================================================
//=========================================================
void CINSHUDHelper::AddHUDElement( IINSHUDElement *pElement )
{
	m_HUDElements.AddToTail( pElement );
}

//=========================================================
//=========================================================
bool CINSHUDHelper::IsHUDElementHidden( int iFlags ) const
{
	// ensure local player
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if ( !pPlayer )
		return true;

	// don't show anything when we's not in running mode
	if( !INSRules( )->IsModeRunning( ) )
		return true;

	// never ever show in deadcam
	if( pPlayer->GetObserverMode( ) == OBS_MODE_DEATHCAM )
		return true;

	// test flags

	// ... test alive and dead
	if( pPlayer->IsRunningAround( ) )
	{
		if( iFlags & HIDEHUD_ALIVE )
			return true;
	}
	else
	{
		if( iFlags & HIDEHUD_DEAD )
			return true;
	}

	// ... not commander
	if( iFlags & HIDEHUD_NOTCOMMANDER && !pPlayer->IsCommander( ) )
		return true;

	// ... test roundcold
	if( iFlags & HIDEHUD_ROUNDCOLD && INSRules( )->IsRoundCold( ) )
		return true;

	// ensure HUD disappears when a VGUI panel has input
	IViewPortPanel *pActivePanel = gViewPortInterface->GetActivePanel( );

	if( pActivePanel && pActivePanel->IsVisible( ) )
		return true;

	return false;
}

//=========================================================
//=========================================================
void CINSHUDHelper::CreateTimer( int iSeconds, char *pszBuffer, int iLength )
{
	Q_snprintf( pszBuffer, iLength, "%02d:%02d", ( iSeconds / 60 ), ( iSeconds % 60 ) );
}

//=========================================================
//=========================================================
#define TIMER_REDTIME 5

int CINSHUDHelper::CreateRoundTimer( char *pszBuffer, int iLength )
{
	static const char *pszDefaultTimer = "00:00";

	if( !INSRules( )->IsModeRunning( ) )
		return ROTIMERTYPE_INVALID;

	if( INSRules( )->RunningMode( )->IsFrozen( ) )
	{
		int iRoundTime = RoundLeft( INSRules( )->RunningMode( )->GetFrozenTimeLength( ) - 1 );
		CreateTimer( iRoundTime, pszBuffer, iLength );

		return ROTIMERTYPE_RED;
	}

	if( INSRules( )->RunningMode( )->IsRoundExtended( ) )
	{
		Q_strncpy( pszBuffer, pszDefaultTimer, iLength );
		return ROTIMERTYPE_FLASH;
	}

	int iRoundLength = INSRules( )->RunningMode( )->GetRoundLength( );

	if( iRoundLength == ROUNDTIMER_INVALID )
	{
		Q_strncpy( pszBuffer, pszDefaultTimer, iLength );
		return ROTIMERTYPE_NONE;
	}

	int iRoundTime = RoundLeft( INSRules( )->RunningMode( )->GetRoundLength( ) );
	CreateTimer( iRoundTime, pszBuffer, iLength );

	return ( ( iRoundTime <= TIMER_REDTIME ) ? ROTIMERTYPE_RED : ROTIMERTYPE_NORMAL );
}

//=========================================================
//=========================================================
int CINSHUDHelper::RoundLeft( int iRoundLength )
{
	return max( iRoundLength - int( gpGlobals->curtime - INSRules( )->RunningMode( )->GetRoundStartTime( ) ), 0 );
}

//=========================================================
//=========================================================
int CINSHUDHelper::GetReinforcementTimer( int &iSeconds, bool bPrepareOffset, bool bIgnoreTK )
{
	if( !INSRules( )->IsModeRunning( ) )
		return ROTIMERTYPE_INVALID;

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );
	Assert( pPlayer );

	if( !pPlayer || !pPlayer->InSquad( ) )
		return RFTIMERTYPE_INVALID;

	C_PlayTeam *pTeam = pPlayer->GetPlayTeam( );
	int iSquadID = pPlayer->GetSquadID( );

	static const char *pszDefaultTimer = "-";

	if( !pTeam->IsUnlimitedWaves( ) && pTeam->GetReinforcementsLeft( iSquadID ) == 0 )
		return RFTIMERTYPE_NONE;

	float flStartReinforcementTime = pTeam->GetStartReinforcementTime( );

	if( flStartReinforcementTime == 0.0f )
		return RFTIMERTYPE_NOTSTARTED;

	if( !bIgnoreTK && pPlayer->IsTKPunished( ) )
		return RFTIMERTYPE_TKPUNISH;

	iSeconds = ( pTeam->GetTimeWave( ) - int( gpGlobals->curtime - flStartReinforcementTime ) );

	int iRetType = ( iSeconds < DEATH_ANIMATION_TIME ) ? RFTIMERTYPE_PREPARE : RFTIMERTYPE_COUNTDOWN;

	if( bPrepareOffset )
		iSeconds -= DEATH_ANIMATION_TIME;

	return iRetType;
}

//=========================================================
//=========================================================
bool CINSHUDHelper::CreateReinforcementCommanderTimer( char *pszBuffer, int iLength )
{
	static const char *pszDefaultTimer = "-";

	int iSeconds, iTimerRet;
	iSeconds = 0;
	iTimerRet = GetReinforcementTimer( iSeconds, false, true );

	if( iTimerRet == RFTIMERTYPE_INVALID )
		return false;
	else if( iTimerRet > RFTIMERTYPE_COUNTDOWN )
		Q_strncpy( pszBuffer, pszDefaultTimer, iLength );
	else
		CreateTimer( iSeconds, pszBuffer, iLength );

	return true;
}

//=========================================================
//=========================================================
bool CINSHUDHelper::CreateReinforcementVGUITimer( char *pszBuffer, int iLength )
{
	int iSeconds = 0;

	switch( GetReinforcementTimer( iSeconds, true, false ) )
	{
		case RFTIMERTYPE_PREPARE:
		{
			Q_strncpy( pszBuffer, "Prepare for Deployment", iLength );
			break;
		}

		case RFTIMERTYPE_COUNTDOWN:
		{
			char szTimerBuffer[ INSHUD_TIMER_LENGTH ];
			CreateTimer( iSeconds, szTimerBuffer, iLength );

			Q_snprintf( pszBuffer, iLength, "Reinforcements in %s", szTimerBuffer );

			break;
		}

		case RFTIMERTYPE_NOTSTARTED:
		{
			Q_strncpy( pszBuffer, "Waiting for Reinforcements", iLength );
			break;
		}

		case RFTIMERTYPE_NONE:
		{
			Q_strncpy( pszBuffer, "Reinforcements Depleted", iLength );
			break;
		}

		case RFTIMERTYPE_TKPUNISH:
		{
			Q_strncpy( pszBuffer, "Reinforcement Skipped for TK", iLength );
			break;
		}

		default:
		{
			return false;
		}
	}

	return true;
}

//=========================================================
//=========================================================
bool CINSHUDHelper::ReinforcementData( int &iLeft, int &iPool )
{
	C_PlayTeam *pTeam = GetLocalPlayTeam( );

	if( !pTeam )
		return false;

	if( pTeam->IsUnlimitedWaves( ) )
		return false;

	iLeft = pTeam->GetNumWaves( );
	iPool = pTeam->GetIMCTeamConfig( )->GetNumWaves( );

	return true;
}

//=========================================================
//=========================================================
bool CINSHUDHelper::EmergencyDeployments( int &iLeft )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer || !pPlayer->InSquad( ) )
		return false;

	C_PlayTeam *pTeam = pPlayer->GetPlayTeam( );

	if( !pTeam )
		return false;

	int iSquadID = pPlayer->GetSquadID( );
	Assert( iSquadID != INVALID_SQUAD );

	iLeft = pTeam->GetEmergencyReinforcementsLeft( iSquadID );

	return true;
}

//=========================================================
//=========================================================
bool CINSHUDHelper::SquadLife( int &iLeft, int &iPool )
{
	// find player and squad
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return false;

	C_INSSquad *pSquad = pPlayer->GetSquad( );

	if( !pSquad )
		return false;

	// count how many
	iLeft = iPool = 0;

	for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
	{
		int iEntityIndex = pSquad->GetPlayerID( i );

		if( iEntityIndex <= 0 )
			continue;

		iPool++;

		if( PlayerResource( )->IsAlive( iEntityIndex ) )
			iLeft++;
	}

	return true;
}

//=========================================================
//=========================================================
const Color &CINSHUDHelper::GetHealthColor( int iType ) const
{
	static Color HealthColors[ HEALTHTYPE_COUNT ] = {
		COLOR_GREEN,	// HEALTHTYPE_UNINJURED
		COLOR_GREEN,	// HEALTHTYPE_FINE
		COLOR_YELLOW,	// HEALTHTYPE_INJURED
		COLOR_RED		// HEALTHTYPE_SERIOUS
	};

	return HealthColors[ iType ];
}

//=========================================================
//=========================================================
const Color &CINSHUDHelper::GetBrightHealthColor( int iType ) const
{
	return m_HealthColors[ iType ];
}

//=========================================================
//=========================================================
void CINSHUDHelper::GetRotation( const Vector &vecDelta, float *flRotation )
{
	QAngle angPlayerAngles = MainViewAngles( );

	Vector vecForward, vecRight, vecUp;

	vecUp = Vector( 0, 0, 1 );

	AngleVectors( angPlayerAngles, &vecForward, NULL, NULL );
	vecForward.z = 0;

	VectorNormalize( vecForward );
	CrossProduct( vecUp, vecForward, vecRight );
	float flFront = DotProduct( vecDelta, vecForward );
	float flSide = DotProduct( vecDelta, vecRight );
	float flXPos = 360.0f * -flSide;
	float flYPos = 360.0f * -flFront;

	*flRotation = atan2( flXPos, flYPos ) + M_PI;
	*flRotation *= 180 / M_PI;
}

//=========================================================
//=========================================================
class C_UnitOrder : public C_BaseAnimating
{
public:
	void SpawnClientEntity( void )
	{
		ForceClientSideAnimationOn( );
	}

	void UpdateClientSideAnimation( void )
	{
		if( GetSequence( ) != ACT_INVALID )
			FrameAdvance( gpGlobals->frametime );
	}
};

//=========================================================
//=========================================================
void CINSHUDHelper::UnitOrder( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	const CUnitOrder *pUnitOrder = pPlayer->GetUnitOrders( );

	if( !pUnitOrder )
		return;

	if( m_pUnitOrderModel )
	{
		m_pUnitOrderModel->Release( );
		m_pUnitOrderModel = NULL;
	}

	if( !pUnitOrder->HasOrders( ) )
		return;
	
	MDLCACHE_CRITICAL_SECTION( );

	m_pUnitOrderModel = new C_UnitOrder( );
	m_pUnitOrderModel->InitializeAsClientEntity( CUnitOrder::ModelName( pUnitOrder->OrderType( ) ), RENDER_GROUP_OPAQUE_ENTITY );

	m_pUnitOrderModel->SetAbsOrigin( pUnitOrder->Position( ) );
}

//=========================================================
//=========================================================
void CINSHUDHelper::DamageTaken( int iAmount, int iBits, Vector &vecFrom )
{
	CPainType *pPain = NULL;

	if( iBits & DMG_FALL )
		pPain = g_PainHelper.CreatePain( PAINTYPE_PMINOR );
	else
		pPain = g_PainHelper.CreatePain( PAINTYPE_PMAJOR );

	if( pPain )
		pPain->SetParameter( "damage", iAmount );
}

//=========================================================
//=========================================================
void CINSHUDHelper::TakeControlFocus( IINSControlListener *pController )
{
	if( m_pActiveController && m_pActiveController != pController )
		m_pActiveController->ControlClose( );

	m_pActiveController = pController;
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendScrollUpdate( int iType )
{
	UpdateActiveController( );
	m_pActiveController->Scroll( iType );
}

//=========================================================
//=========================================================
void CINSHUDHelper::SendNumberUpdate( int iNumber )
{
	UpdateActiveController( );
	m_pActiveController->Number( iNumber );
}

//=========================================================
//=========================================================
void CINSHUDHelper::RemoveControlFocus( void )
{
	m_pActiveController = NULL;
}

//=========================================================
//=========================================================
void CINSHUDHelper::UpdateActiveController( void )
{
	if( !m_pActiveController || !m_pActiveController->IsControlActive( ) )
		m_pActiveController = m_pDefaultController;

	Assert( m_pActiveController );
}

//=========================================================
//=========================================================
void CINSHUDHelper::UnitOrderStart( void )
{
	m_pUnitOrder->StartOrder( );
}