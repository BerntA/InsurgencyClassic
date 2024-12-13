//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_ins_base.h"
#include "keyvalues.h"
#include "weapon_defines.h"
#include "ins_gamerules.h"
#include "ins_utils.h"
#include "in_buttons.h"
#include "debugoverlay_shared.h"
#include "imc_config.h"
#include "weapondef.h"

#ifdef CLIENT_DLL

#include "prediction.h"
#include "ins_viewmodel_shared.h"
#include "inshud.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
WeaponData_t g_WeaponData[ MAX_WEAPONS ];
WeaponDataCreate_Helper_t g_WeaponDataHelpers[ MAX_WEAPONS ];

//=========================================================
//=========================================================
DEFINE_STRING_LOOKUP_CONSTANTS( int, weaponids )

	ADD_LOOKUP( WEAPON_M67 )
	ADD_LOOKUP( WEAPON_M18 )
	ADD_LOOKUP( WEAPON_M16A4 )
	ADD_LOOKUP( WEAPON_M16M203 )
	ADD_LOOKUP( WEAPON_M4 )
	ADD_LOOKUP( WEAPON_M9 )
    ADD_LOOKUP( WEAPON_M14 )
	ADD_LOOKUP( WEAPON_M249 )
	ADD_LOOKUP( WEAPON_M1014 )
	ADD_LOOKUP( WEAPON_C4 )
	ADD_LOOKUP( WEAPON_KABAR )
	ADD_LOOKUP( WEAPON_RGD5 )
	ADD_LOOKUP( WEAPON_AK47 )
	ADD_LOOKUP( WEAPON_RPK )
	ADD_LOOKUP( WEAPON_RPG7 )
	ADD_LOOKUP( WEAPON_MAKAROV )
	ADD_LOOKUP( WEAPON_FNFAL )
	ADD_LOOKUP( WEAPON_SKS )
	ADD_LOOKUP( WEAPON_TOZ )
	ADD_LOOKUP( WEAPON_L42A1 )
	ADD_LOOKUP( WEAPON_BAYONET )

END_STRING_LOOKUP_CONSTANTS( )

//=========================================================
//=========================================================
DEFINE_STRING_LOOKUP_CONSTANTS( int, weapontypes )

	ADD_LOOKUP( WEAPONTYPE_PRIMARY )
	ADD_LOOKUP( WEAPONTYPE_SECONDARY )

END_STRING_LOOKUP_CONSTANTS( )

//=========================================================
//=========================================================
DEFINE_STRING_LOOKUP_CONSTANTS( int, tracertypes )

	ADD_LOOKUP( TRACERTYPE_NONE )
	ADD_LOOKUP( TRACERTYPE_DEFAULT )
	ADD_LOOKUP( TRACERTYPE_WATER )
	ADD_LOOKUP( TRACERTYPE_RED )
	ADD_LOOKUP( TRACERTYPE_GREEN )
	ADD_LOOKUP( TRACERTYPE_PLASMA )

END_STRING_LOOKUP_CONSTANTS( )

//=========================================================
//=========================================================
inline bool IsInvalidWeaponID( int iWeaponID )
{
	return ( ( iWeaponID >= MAX_WEAPONS ) || ( iWeaponID < 0 ) );
}

const char *WeaponIDToAlias( int iWeaponID )
{
	if( IsInvalidWeaponID( iWeaponID ) )
		return NULL;

	return g_WeaponData[ iWeaponID ].m_pszWeaponAlias;
}

const char *WeaponIDToName( int iWeaponID )
{
	if( IsInvalidWeaponID( iWeaponID ) )
		return NULL;

	return g_WeaponData[ iWeaponID ].m_pszWeaponName;
}

int WeaponNameToID( const char *pszWeapon )
{
	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		AssertMsg( g_WeaponData[ i ].m_pszWeaponName, "WeaponNameToID, NULL Weapon Name" );

		if( !g_WeaponData[ i ].m_pszWeaponName )
			continue;

		if( Q_strcmp( g_WeaponData[ i ].m_pszWeaponName, pszWeapon ) == 0 )
			return i;
	}

	return WEAPON_INVALID;
}

int WeaponIDToType( int iWeaponID )
{
	if( IsInvalidWeaponID( iWeaponID ) )
		return NULL;

	return g_WeaponData[ iWeaponID ].m_iWeaponType;
}

//=========================================================
//=========================================================
INSWeaponInfo_t::INSWeaponInfo_t( )
{
	iIronsightFOV = 0;
	flFreeaimDistanceFactor = 0.0f;
	flWeaponLagFactor = 0.0f;

	iViewmodelFOV = 0;
    iIronsightViewmodelFOV = 0;

	iScopeFOV = 0;
	bHasScope = false;

#ifdef CLIENT_DLL

	bPrecachedResources = false;

	szAmmoTex[ 0 ] = '\0';
	iAmmoTexID = 0;

#endif
}

void INSWeaponInfo_t::Parse( KeyValues *pKeyValuesData, const char *pszWeaponName )
{
	BaseClass::Parse( pKeyValuesData, pszWeaponName );

	iIronsightFOV = pKeyValuesData->GetInt( "ironsight_fov", 75 );
	flFreeaimDistanceFactor = pKeyValuesData->GetFloat( "freeaim_distance_scale", 1.0f );
	flWeaponLagFactor = pKeyValuesData->GetFloat( "weaponlag_scale", 1.0f );

	iViewmodelFOV = pKeyValuesData->GetInt( "viewmodel_fov", GetWeaponDef( )->iDefaultViewmodelFOV );
    iIronsightViewmodelFOV = pKeyValuesData->GetInt( "viewmodel_ironsight_fov", GetWeaponDef( )->iDefaultViewmodelIronsightFOV );

	iScopeFOV = pKeyValuesData->GetFloat( "scope_fov", 0 );
	bHasScope = ( iScopeFOV > 0 );

#ifdef CLIENT_DLL

	Q_strncpy( szAmmoTex, pKeyValuesData->GetString( "ammotex", "" ), MAX_WEAPON_STRING );

#endif
}

//=========================================================
//=========================================================
BEGIN_NETWORK_TABLE_NOBASE( CWeaponINSBase, DT_LocalINSWeaponData )

#ifdef GAME_DLL

	SendPropIntWithMinusOneFlag( SENDINFO( m_iPlayerSlot ), PLAYER_WEAPONSLOT_BITS ),
	SendPropInt( SENDINFO( m_iIdleState ), 2, SPROP_UNSIGNED ),

#else

	RecvPropIntWithMinusOneFlag( RECVINFO( m_iPlayerSlot ) ),
	RecvPropInt( RECVINFO( m_iIdleState ) ),

#endif

END_NETWORK_TABLE()

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponINSBase, DT_WeaponINSBase )

#ifdef GAME_DLL

extern void *SendProxy_SendLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID );

#endif

BEGIN_NETWORK_TABLE( CWeaponINSBase, DT_WeaponINSBase )

#ifdef GAME_DLL

	SendPropDataTable( "LocalINSWeaponData", 0, &REFERENCE_SEND_TABLE( DT_LocalINSWeaponData ), SendProxy_SendLocalWeaponDataTable ),

#else

	RecvPropDataTable( "LocalINSWeaponData", 0, 0, &REFERENCE_RECV_TABLE( DT_LocalINSWeaponData ) ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponINSBase )

	DEFINE_PRED_FIELD( m_iIdleState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA( )

#endif

//=========================================================
//=========================================================
CWeaponINSBase::CWeaponINSBase( )
{
	m_iIdleState = WIDLESTATE_NORMAL;

#ifdef CLIENT_DLL

	m_flFreeaimDistance = 0.0f;
	m_flHippedLagFactor = m_flIronsightLagFactor = 0.0f;

#endif
}

//=========================================================
//=========================================================
CINSPlayer *CWeaponINSBase::GetINSPlayerOwner( void ) const
{
	return ToINSPlayer( GetOwner( ) );
}

//=========================================================
//=========================================================
void CWeaponINSBase::Spawn( void )
{
	BaseClass::Spawn( );

	m_iPlayerSlot = WEAPONSLOT_INVALID;

#ifdef CLIENT_DLL

	m_flFreeaimDistance = GetWeaponDef( )->flFreeaimDistance * GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, flFreeaimDistanceFactor );

	float flWeaponLagFactor = GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, flWeaponLagFactor );
	m_flHippedLagFactor = WEAPONSWAY_SCALE_HIPPED * flWeaponLagFactor;
	m_flIronsightLagFactor = WEAPONSWAY_SCALE_IRONSIGHTS * flWeaponLagFactor;

#endif
}

//=========================================================
//=========================================================
void CWeaponINSBase::Precache( void )
{
	BaseClass::Precache( );

#ifdef CLIENT_DLL

	bool &bPrecachedResources = GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, bPrecachedResources );

	if( !bPrecachedResources )
	{
		PrecacheResources( );

		bPrecachedResources = true;
	}

#endif
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

//=========================================================
//=========================================================
void CWeaponINSBase::PrecacheResources( void )
{
	LoadAmmoTex( GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, szAmmoTex ), GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, iAmmoTexID ) );
}

//=========================================================
//=========================================================
int CWeaponINSBase::GetAmmoTexID( void ) const
{
	return GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, iAmmoTexID );
}

//=========================================================
//=========================================================
void CWeaponINSBase::LoadAmmoTex( const char *pszAmmoTex, int &iAmmoTexID )
{
	iAmmoTexID = GetINSHUDHelper( )->WeaponInfo( )->CreateAmmoTexID( pszAmmoTex );
}

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CWeaponINSBase::Delete( void )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer )
		pPlayer->RemovedWeapon( this );

	BaseClass::Delete( );
}

#endif

//=========================================================
//=========================================================
bool CWeaponINSBase::CanDeploy( void )
{
	if( !BaseClass::CanDeploy( ) )
		return false;

	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer->IsLeaning( ) && !AllowLeaning( ) )
		return false;

	return true;
}

//=========================================================
//=========================================================
void CWeaponINSBase::HandleDeploy( void )
{
	BaseClass::HandleDeploy( );

	// perform cock when activity is ready
	if( GetActivity( ) == ACT_VM_READY )
		OnDeployReady( );

#ifdef GAME_DLL

	if( SendIronsightsHint( ) && CanUseIronsights( ) )
		UTIL_SendHint( GetINSPlayerOwner( ), HINT_IRONSIGHTS );

	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer )
	{
	    pPlayer->SetViewmodelFOV( GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, iViewmodelFOV ) );
        pPlayer->SetScopeFOV( GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, iScopeFOV ) );
	}

#else GAME_DLL

	GetINSHUDHelper( )->WeaponInfo( )->ShowAmmoInfo( );

#endif

	m_iIdleState = WIDLESTATE_NORMAL;
}

//=========================================================
//=========================================================
void CWeaponINSBase::OnDeployReady( void )
{
}

//=========================================================
//=========================================================
bool CWeaponINSBase::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	// remove ironsights
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer && pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
		SetIronsights( false, true );

	return BaseClass::Holster( pSwitchingTo );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CWeaponINSBase::RegisterShot( void )
{
	// update shots fired
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	// note it to the player
	pPlayer->NoteWeaponFired( );

	// register a shot fired
	pPlayer->BumpWeaponStat( GetWeaponID( ), WEAPONSTATS_SHOTS );
}

#endif

//=========================================================
//=========================================================
bool CWeaponINSBase::FindMuzzle( Vector &vecOrigin, Vector &vecForward, bool bForceEye )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if(!pPlayer)
		return false;

	Vector vecMuzzle;

	if( !bForceEye && !pPlayer->IsBot( ) && ShouldDrawViewModel( ) )
	{
		QAngle angMuzzle;
		pPlayer->GetMuzzle( vecMuzzle, angMuzzle );

		AngleVectors( angMuzzle, &vecForward );
		VectorMA( vecMuzzle, -25, vecForward, vecOrigin );
	}
	else
	{
		vecOrigin = pPlayer->Weapon_ShootPosition( );
		vecForward = pPlayer->Weapon_ShootDirection( );
	}

	return true;
}

//=========================================================
//=========================================================
bool CWeaponINSBase::CanReload( void )
{
	return ( m_iIdleState == WIDLESTATE_NORMAL );
}

//=========================================================
//=========================================================
void CWeaponINSBase::StartReload( void )
{
	BaseClass::StartReload( );

	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	// come out of ironsights
	if( pPlayer )
	{
		if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
			SetIronsights( false, true );

#ifdef GAME_DLL

		pPlayer->SendAction( PACTION_RELOADING );

#endif
	}

#ifdef CLIENT_DLL

	GetINSHUDHelper( )->WeaponInfo( )->ShowAmmoInfo( m_flNextPrimaryAttack - gpGlobals->curtime, true );

#endif

	DoAnimationEvent( PLAYERANIMEVENT_WEAP_RELOAD );
}

//=========================================================
//=========================================================
void CWeaponINSBase::DoAnimationEvent(int iEvent)
{
	CINSPlayer *pPlayer = GetINSPlayerOwner();

	if(pPlayer)
		pPlayer->DoAnimationEvent((PlayerAnimEvent_e)iEvent);
}

//=========================================================
//=========================================================
void CWeaponINSBase::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame( );

	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	// ensure valid player and in ironsights
	if( !pPlayer || ( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) == 0 )
		return;

	// if a player is crawling (with ironsights) then bring
	// them out of ironsights
	if( pPlayer->IsCrawling( ) )
		SetIronsights( false, false );
	
	// release ironsights when button is released when using holding		
	if( pPlayer->GetCmdValue( CMDREGISTER_IRONSIGHTHOLD ) != 0 && pPlayer->m_afButtonReleased & IN_SPECIAL2 )
		ToggleIronsights( );

	// release ironsights when in down (can't move at all when in crawl)
	if( m_iIdleState == WIDLESTATE_DOWN )
		SetIronsights( false, false );
}

//=========================================================
//=========================================================
bool CWeaponINSBase::CanAttack( void )
{
	if( m_iIdleState != WIDLESTATE_NORMAL )
		return false;

	return BaseClass::CanAttack( );
}

//=========================================================
//=========================================================
bool CWeaponINSBase::CanTertiaryAttack( void )
{
	if( !BaseClass::CanTertiaryAttack( ) )
		return false;

	if( CanUseIronsights( ) )
	{
		CINSPlayer *pPlayer = GetINSPlayerOwner( );

		if( !pPlayer )
			return false;

		if( pPlayer->GetCmdValue( CMDREGISTER_IRONSIGHTHOLD ) )
		{
			if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
				return false;
		}
		else
		{
			if( ( pPlayer->m_afButtonPressed & IN_SPECIAL2 ) == 0 )
				return false;
		}
	}

	return true;
}

//=========================================================
//=========================================================
void CWeaponINSBase::TertiaryAttack( void )
{
	if( !CanUseIronsights( ) )
		return;

	ToggleIronsights( );
}

//=========================================================
//=========================================================
void CWeaponINSBase::ToggleIronsights( void ) 
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );
	SetIronsights( !( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ), false );
}

//=========================================================
//=========================================================
void CWeaponINSBase::SetIronsights( bool bState, bool bForce )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	if( bState && m_iIdleState != WIDLESTATE_NORMAL )
		return;

	bool bIronsights = ( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) ? true : false;

	if( bState && !bIronsights )
	{
		SetIronsightsState( true );
	}
	else if( !bState && bIronsights )
	{
		SetIronsightsState( false );
	}
	else
	{
		return;
	}

	// modify the view when crouched
	if( pPlayer->IsCrouched( ) )
		pPlayer->SetViewTransition( pPlayer->GetCurrentViewOffset( ), 0.1f );

	// send the idle anim and set time
	if( !bForce )
	{
		SendWeaponAnim( GetWeaponIdleActivity( ) );

		m_flNextTertiaryAttack = gpGlobals->curtime + SequenceDuration( );
	}
}

//=========================================================
//=========================================================
void CWeaponINSBase::SetIronsightsState( bool bState )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	pPlayer->SetIronsightsState( bState );

	if( bState )
	{
#ifdef GAME_DLL

		SetPlayerIronsightFOV( true, 0.3f );

#endif

#ifdef CLIENT_DLL

		pPlayer->SetFreeAimAngles( vec3_angle );

#endif

		pPlayer->ViewPunch( QAngle( random->RandomInt( -1, -4 ), random->RandomInt( -1, -4 ), random->RandomInt( -1, 1 ) ) );
	}
	else
	{
#ifdef GAME_DLL

		SetPlayerIronsightFOV( false, 0.1f );

#endif
	}
}

//=========================================================
//=========================================================
bool CWeaponINSBase::CanUseIronsights( void )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );
	return ( pPlayer && m_iIdleState == WIDLESTATE_NORMAL );
}

//=========================================================
//=========================================================
#define VM_LENGTHCHECK_FRACTION 2.85f

#ifdef _DEBUG

ConVar showvminteraction( "sv_showvminteraction", "1" );

#endif

void CWeaponINSBase::CalcViewmodelInteraction( float &flFraction, Vector *pForward )
{
	flFraction = 1.0f;

	if( !AllowViewmodelInteraction( ) )
		return;

	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	float flVMLength, flVMWidth;
	CalcViewmodelInteraction( flVMLength, flVMWidth );

	Vector vecOrigin, vecForward, vecEnd;

	FindMuzzle( vecOrigin, vecForward, LowerViewmodelInteraction( ) );
	vecEnd = vecOrigin + ( vecForward * flVMLength * VM_LENGTHCHECK_FRACTION );

	if( pForward )
		*pForward = vecForward;

#ifdef _DEBUG

	if( showvminteraction.GetBool( ) )
	{
	#ifdef GAME_DLL

		NDebugOverlay::Line( vecOrigin, vecEnd, 0, 0, 255, true, 0.1f );

	#else

		debugoverlay->AddLineOverlay( vecOrigin, vecEnd, 255, 0, 0, true, 0.1f );

	#endif
	}

#endif

	trace_t tr;
	UTIL_TraceHull( vecOrigin, vecEnd, 
		Vector( -flVMWidth, -flVMWidth, -flVMWidth ),
		Vector( flVMWidth, flVMWidth, flVMWidth ),
		MASK_SOLID, this, COLLISION_GROUP_DEBRIS, &tr );

	flFraction = tr.fraction;
}

//=========================================================
//=========================================================
void CWeaponINSBase::CalcViewmodelInteraction( Vector &vecOrigin, QAngle &angAngles )
{
	if( LowerViewmodelInteraction( ) )
		return;

	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	float flFraction;
	Vector vecForward;

	CalcViewmodelInteraction( flFraction, &vecForward );

	if( flFraction != 1.0f )
	{
		float flInteractionFractionLimit = 1.0f;

		if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
		{
			if( GetActivity( ) != ACT_TRANSITION )
				return;

			if( GetSequence( ) != SelectWeightedSequence( GetIronsightInActivity( ) ) )
				return;

			CBaseViewModel *pVM = GetOwnerViewModel( );
	
			if( !pVM )
				return;

			flInteractionFractionLimit = 1.0f - pVM->GetCycle( );
		}

		flFraction = min( flFraction, flInteractionFractionLimit );

		float flVMLength, flVMWidth;
		CalcViewmodelInteraction( flVMLength, flVMWidth );

		float flChange = ( flVMLength * ( 1.0f - flFraction ) );
		vecOrigin -= ( vecForward * flChange ) + ( Vector( 0, 0, 1 ) * ( flChange * 0.25f ) );
	}
}

//=========================================================
//=========================================================
#define VM_DEFAULT_LENGTH 7.0f
#define VM_DEFAULT_HALFWIDTH 2.0f

void CWeaponINSBase::CalcViewmodelInteraction( float &flLength, float &flHalfWidth )
{
	flLength = VM_DEFAULT_LENGTH;
	flHalfWidth = VM_DEFAULT_HALFWIDTH;
}

//=========================================================
//=========================================================
Activity CWeaponINSBase::GetIronsightInActivity(void) const
{
	if( UseEmptyAnimations( ) )
		return ACT_VM_IIN_EMPTY;

	return ACT_VM_IIN;
}

//=========================================================
//=========================================================
Activity CWeaponINSBase::GetIronsightOutActivity(void) const
{
	if( UseEmptyAnimations( ) )
		return ACT_VM_IOUT_EMPTY;

	return ACT_VM_IOUT;
}

//=========================================================
//=========================================================
Activity CWeaponINSBase::GetWeaponIdleActivity( void ) const
{
	switch( m_iIdleState )
	{
		case WIDLESTATE_DOWN:
			return GetDownActivity( );

		case WIDLESTATE_CRAWL:
			return GetCrawlActivity( );
	}

	return GetNormalActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponINSBase::GetPrimaryAttackActivity( void ) const
{
	CINSPlayer *pOwner = GetINSPlayerOwner( );

	if( pOwner && pOwner->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
		return ACT_VM_ISHOOT;
	
	return ACT_VM_PRIMARYATTACK;		
}

//=========================================================
//=========================================================
#define	INS_BOB_CYCLE_MIN	1.0f
#define	INS_BOB_CYCLE_MAX	0.45f
#define	INS_BOB				0.002f
#define	INS_BOB_UP			0.5f

void CWeaponINSBase::CalcViewmodelBob( void )
{
	float flCycle;

	CINSPlayer *player = GetINSPlayerOwner( );
	//Assert( player );

	//NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it

	if ( ( !gpGlobals->frametime ) || ( player == NULL ) )
		return;

	if( player->GetMoveType( ) != MOVETYPE_WALK )
		return;

	if( player->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS || player->IsProned( ) )
	{
		m_flLateralBob  = 0.0f;
		m_flVerticalBob = 0.0f;
		return;
	}

	float bob_offset = 1.0f;

	//FIXME: This maximum speed value must come from the server.
	//		 MaxSpeed() is not sufficient for dealing with sprinting - jdw

	//Find the speed of the player
	float speed = player->GetLocalVelocity().Length2D();

	if( player->IsSprinting( ) )
		bob_offset = 1.2f;
	else
		bob_offset = RemapVal( speed, 0, PLAYER_MAXSPEED_WALK, 0.2f, 0.4f );

	m_flBobTime += ( gpGlobals->curtime - m_flLastBobTime ) * bob_offset;
	m_flLastBobTime = gpGlobals->curtime;

	//Calculate the vertical bob
	flCycle = m_flBobTime - (int)(m_flBobTime/INS_BOB_CYCLE_MAX)*INS_BOB_CYCLE_MAX;
	flCycle /= INS_BOB_CYCLE_MAX;

	if ( flCycle < INS_BOB_UP )
	{
		flCycle = M_PI * flCycle / INS_BOB_UP;
	}
	else
	{
		flCycle = M_PI + M_PI*(flCycle-INS_BOB_UP)/(1.0 - INS_BOB_UP);
	}

	m_flVerticalBob = speed*0.005f;
	m_flVerticalBob = m_flVerticalBob*0.3 + m_flVerticalBob*0.7*sin(flCycle);

	m_flVerticalBob = clamp( m_flVerticalBob, -7.0f, 4.0f );

	//Calculate the lateral bob
	flCycle = m_flBobTime - (int)(m_flBobTime/INS_BOB_CYCLE_MAX*2)*INS_BOB_CYCLE_MAX*2;
	flCycle /= INS_BOB_CYCLE_MAX*2;

	if ( flCycle < INS_BOB_UP )
	{
		flCycle = M_PI * flCycle / INS_BOB_UP;
	}
	else
	{
		flCycle = M_PI + M_PI*(flCycle-INS_BOB_UP)/(1.0 - INS_BOB_UP);
	}

	m_flLateralBob = speed*0.005f;
	m_flLateralBob = m_flLateralBob*0.3 + m_flLateralBob*0.7*sin(flCycle);
	m_flLateralBob = clamp( m_flLateralBob, -7.0f, 4.0f );
}

//=========================================================
//=========================================================
void CWeaponINSBase::UpdateIdleState( void )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	// save the old state
	int iOldState = m_iIdleState;

	// set the new state
	m_iIdleState = WIDLESTATE_NORMAL;

	if( pPlayer )
	{
		if( pPlayer->IsCrawling( ) || ( pPlayer->InStanceTransition( ) && ( pPlayer->GetCurrentStance( ) == STANCE_PRONE || pPlayer->GetLastStance( ) == STANCE_PRONE ) ) )
			m_iIdleState = WIDLESTATE_CRAWL;
		else if( ShouldHideWeapon( ) )
			m_iIdleState = WIDLESTATE_DOWN;
	}

	// compare and reset idle animation if different
	if( m_iIdleState != iOldState )
		SetWeaponIdleTime( 0.0f );
}

//=========================================================
//=========================================================
bool CWeaponINSBase::ShouldHideWeapon( void )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return false;

	if( pPlayer->IsReloading( ) )
		return false;

	// if we're sprinting
	if( pPlayer->IsSprinting( ) )
		return true;

	// we're on a ladder
	if( pPlayer->GetMoveType( ) == MOVETYPE_LADDER )
		return true;

	// test water and jump attack
	if( TestWaterAttack( ) || TestJumpAttack( ) )
		return true;

	// test if we're close a wall
	if( LowerViewmodelInteraction( ) )
	{
		float flFraction;
		CalcViewmodelInteraction( flFraction, NULL );

		if( flFraction != 1.0f )
			return true;
	}

	return false;
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CWeaponINSBase::SetPlayerIronsightFOV( bool bState, float flTime )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer )
    {
		pPlayer->SetFOV( bState ? GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, iIronsightFOV ) : 0, flTime );
        pPlayer->SetViewmodelFOV( bState ? GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, iIronsightViewmodelFOV ) : GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, iViewmodelFOV ), flTime );
    }
}

#else

//=========================================================
//=========================================================
bool CWeaponINSBase::DoClientEffects( void ) const
{
	return ( prediction->InPrediction( ) && prediction->IsFirstTimePredicted( ) );
}

//=========================================================
//=========================================================
bool CWeaponINSBase::AllowFreeaim( void ) const
{
	// RELEASEHACK: freeaim is disabled in ironsights for first release
	CINSPlayer *pPlayer = GetINSPlayerOwner( );
	return ( pPlayer && ( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) == 0 );
}

//=========================================================
//=========================================================
float CWeaponINSBase::GetFreeaimDistance( void ) const
{
	return m_flFreeaimDistance;
}

//=========================================================
//=========================================================
float CWeaponINSBase::GetWeaponLagFactor( bool bIronsights )
{
	return ( bIronsights ? m_flIronsightLagFactor : m_flHippedLagFactor );
}

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CWeaponINSBase::HasScope( void ) const
{
	return GET_WEAPON_DATA_CUSTOM( INSWeaponInfo_t, bHasScope );
}

#endif

//=========================================================
//=========================================================
int CWeaponINSBase::GetActiveFiremode( void ) const
{
	return INVALID_FIREMODE;
}

//=========================================================
//=========================================================
void CWeaponINSBase::ForceReady( void )
{
	if( GetIdealActivity( ) == ACT_VM_READY || GetIdealActivity( ) == ACT_VM_READY_M203 )
		SendWeaponAnim( GetDrawActivity( ) );
}

//=========================================================
//=========================================================
Activity CWeaponINSBase::GetDownActivity( void ) const
{
	if( UseEmptyAnimations( ) )
		return ACT_VM_DOWN_EMPTY;

	return ACT_VM_DOWN;
}

//=========================================================
//=========================================================
Activity CWeaponINSBase::GetCrawlActivity( void ) const
{
	if( UseEmptyAnimations( ) )
		return ACT_VM_CRAWL_EMPTY;

	return ACT_VM_CRAWL;
}

//=========================================================
//=========================================================
Activity CWeaponINSBase::GetNormalActivity( void ) const
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer )
	{
		bool bIronsights = ( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) ? true : false;

		if( UseEmptyAnimations( ) )
		{
			if( bIronsights )
				return ACT_VM_IIDLE_EMPTY;

			return ACT_VM_IDLE_EMPTY;
		}
		else if( bIronsights )
		{
			return ACT_VM_IIDLE;
		}
	}

	return BaseClass::GetWeaponIdleActivity( );
}

//=========================================================
//=========================================================
void CWeaponINSBase::OnDrop( void )
{
	if( IsActiveWeapon( ) )
		SetIronsights( false, true );
}

//=========================================================
//=========================================================
bool CWeaponINSBase::TestJumpAttack( void ) const
{
	if( IMCConfig( )->HasCustomGravity( ) )
		return false;

	if( AllowJumpAttack( ) )
		return false;

	CINSPlayer *pPlayer = GetINSPlayerOwner( );
	return ( pPlayer && pPlayer->IsJumping( ) );
}

//=========================================================
//=========================================================
bool CWeaponINSBase::AllowPlayerStance( int iFromStance, int iToStance )
{
	return true;
}

//=========================================================
//=========================================================
bool CWeaponINSBase::AllowPlayerMovement( void ) const
{
	if( IsReloading( ) )
	{
		CINSPlayer *pPlayer = GetINSPlayerOwner( );

		if( pPlayer && pPlayer->IsProned( ) )
			return false;
	}

	return true;
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

//=========================================================
//=========================================================
CBasePlayer *CWeaponINSBase::GetScorer( void ) const
{
	return GetOwner( );
}

//=========================================================
//=========================================================
int CWeaponINSBase::GetInflictorType( void ) const
{
	return INFLICTORTYPE_WEAPON;
}

//=========================================================
//=========================================================
int CWeaponINSBase::GetInflictorID( void ) const
{
	return GetWeaponID( );
}

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

C_WeaponINSBase *GetINSActiveWeapon( void )
{
	return ToINSWeapon( GetActiveWeapon( ) );
}

#endif
