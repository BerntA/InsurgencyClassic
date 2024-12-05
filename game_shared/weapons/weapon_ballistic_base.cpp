//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_ballistic_base.h"
#include "in_buttons.h"
#include "keyvalues.h"
#include "ammodef.h"
#include "weapondef.h"
#include "debugoverlay_shared.h"
#include "clipdef.h"
#include "command_register.h"
#include "bulletdef.h"

#ifdef GAME_DLL

#include "ins_recipientfilter.h"

#else

#include "inshud.h"
#include "takedamageinfo.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
extern ConVar perfectshot;

//=========================================================
//=========================================================
DEFINE_STRING_LOOKUP_CONSTANTS( int, bulletformtypes )

	ADD_LOOKUP( BULLETFT_FLATNOSELEAD )
	ADD_LOOKUP( BULLETFT_ROUNDNOSELEAD )
	ADD_LOOKUP( BULLETFT_ROUNDNOSEJACKETED )
	ADD_LOOKUP( BULLETFT_SEMIPOINTEDSOFTPOINT )
	ADD_LOOKUP( BULLETFT_POINTEDSOFTPOINT )
	ADD_LOOKUP( BULLETFT_POINTEDFULLJACKET )
	ADD_LOOKUP( BULLETFT_POINTEDFULLJACKETBOATTAILED )

END_STRING_LOOKUP_CONSTANTS( )

//=========================================================
//=========================================================
BallisticRange_t::BallisticRange_t( )
{
	iMaxRange = 0;
	vecSpread.Init( );
}

//=========================================================
//=========================================================

// PNOTE: it's strange ... all values need to be adjusted by 25% faster
// otherwise the firetime is lower
#define CALCULATE_SECONDS_RPM( rpm ) ( ( 1.0f / ( rpm / 60.0f ) ) * 0.75f )

//=========================================================
//=========================================================
BallisticWeaponInfo_t::BallisticWeaponInfo_t( )
{
	iClipType = INVALID_CLIP;
	iMuzzleVelocity = 0;
	flCycleTime = 0.0f;
	flBurstCycleTime = 0.0f;
	iFiremodes = 0;
	iTracerType = TRACERTYPE_DEFAULT;
	flRecoilXMin = flRecoilXMax = flRecoilYMin = flRecoilYMax = 0.0f;
}

//=========================================================
//=========================================================
void BallisticWeaponInfo_t::Parse( KeyValues *pKeyValuesData, const char *pszWeaponName )
{
	BaseClass::Parse( pKeyValuesData, pszWeaponName );

	// find valid clips
	const char *pszClipType = pKeyValuesData->GetString( "clip_type" );

	if( pszClipType && *pszClipType != '/0' )
		LookupClipTypeID( pszClipType, iClipType );

	// find bullet data

	// ... adjusted
	iMuzzleVelocity = pKeyValuesData->GetInt( "vmuzzle", 800 );
	CalculateRange( pKeyValuesData, "erange", Range, RangeIronsights );

	// ... real-life
	iMuzzleVelocityRL = pKeyValuesData->GetInt( "r_vmuzzle", 800 );
	CalculateRange( pKeyValuesData, "r_erange", RangeRL, RangeIronsightsRL );

	// set cycle time
	flCycleTime = pKeyValuesData->GetFloat( "cycle_time", 600 );

	if( flCycleTime != 0.0f )
		flCycleTime = CALCULATE_SECONDS_RPM( flCycleTime );

	flBurstCycleTime = pKeyValuesData->GetFloat( "burst_cycle_time", 800 );

	if( flBurstCycleTime != 0.0f )
		flBurstCycleTime = CALCULATE_SECONDS_RPM( flBurstCycleTime );

	// firemodes
	iFiremodes = pKeyValuesData->GetFloat( "firemodes", 1 );

	// tracertype
	const char *pszTracerType = pKeyValuesData->GetString( "tracertype" );

	if( pszTracerType && *pszTracerType != '\0' )
		LookupTracerType( pszTracerType, iTracerType );

	// work out recoil
	flRecoilXMin = pKeyValuesData->GetFloat( "recoil_x_min", GetWeaponDef( )->flRecoilXMin );
	flRecoilXMax = pKeyValuesData->GetFloat( "recoil_x_max", GetWeaponDef( )->flRecoilXMax );
	flRecoilYMin = pKeyValuesData->GetFloat( "recoil_y_min", GetWeaponDef( )->flRecoilYMin );
	flRecoilYMax = pKeyValuesData->GetFloat( "recoil_y_max", GetWeaponDef( )->flRecoilYMax );

	flRecoilXMinBipod = pKeyValuesData->GetFloat( "recoil_x_min_bipod", GetWeaponDef()->flRecoilXMinBipod );
	flRecoilXMaxBipod = pKeyValuesData->GetFloat( "recoil_x_max_bipod", GetWeaponDef()->flRecoilXMaxBipod );
	flRecoilYMinBipod = pKeyValuesData->GetFloat( "recoil_y_min_bipod", GetWeaponDef()->flRecoilYMinBipod );
	flRecoilYMaxBipod = pKeyValuesData->GetFloat( "recoil_y_max_bipod", GetWeaponDef()->flRecoilYMaxBipod );

	float flRecoilScale = pKeyValuesData->GetFloat( "recoil_scale", 1.0f );

	flRecoilXMin *= flRecoilScale;
	flRecoilXMax *= flRecoilScale;
	flRecoilYMin *= flRecoilScale;
	flRecoilYMax *= flRecoilScale;
}

//=========================================================
//=========================================================
void BallisticWeaponInfo_t::CalculateRange( KeyValues *pData, const char *pszDataName, BallisticRange_t &Range, BallisticRange_t &RangeIronsights )
{
	// work out effective range
	Range.iEffectiveRange = pData->GetInt( pszDataName, 350 ) * INCHS_PER_METER * GetWeaponDef( )->flAccuracyMod;
	RangeIronsights.iEffectiveRange = Range.iEffectiveRange * GetWeaponDef( )->flIronSightsAccuracyMod;

	// calculate spread for effective range
	CalculateEffectiveRange( Range );
	CalculateEffectiveRange( RangeIronsights );

	// work out max range (e.g. where the tracelines stop)
	Range.iMaxRange = min( Range.iEffectiveRange * GetWeaponDef( )->flMaxRangeMod, MAX_TRACE_LENGTH );
	RangeIronsights.iMaxRange = min( RangeIronsights.iEffectiveRange * GetWeaponDef( )->flMaxRangeMod, MAX_TRACE_LENGTH );
}

//=========================================================
//=========================================================
void BallisticWeaponInfo_t::CalculateEffectiveRange( BallisticRange_t &Range )
{
	float flSpreadAngle = RAD2DEG( atan( 1.0f / ( Range.iEffectiveRange * GetWeaponDef( )->flAngleExaggeration ) ) );
	Range.vecSpread.Init( flSpreadAngle, flSpreadAngle, flSpreadAngle );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

extern void *SendProxy_SendLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID );

void SendProxy_ClipList(const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	CWeaponBallisticBase *pWeapon = ( CWeaponBallisticBase* )pStruct;
	pOut->m_Int = ( pWeapon ? pWeapon->GetClipList( ).Element( iElement ) : 0 );
}

int SendProxyArrayLength_ClipArray( const void *pStruct, int objectID )
{
	CWeaponBallisticBase *pWeapon = ( CWeaponBallisticBase* )pStruct;
	return ( pWeapon ? pWeapon->GetClipList( ).Count( ) : 0 );
}

#else

void RecvProxy_ClipList( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_WeaponBallisticBase *pWeapon = ( C_WeaponBallisticBase* )pStruct;

	if( pWeapon )
		pWeapon->GetClipList( ).Element( pData->m_iElement ) = pData->m_Value.m_Int;
}

void RecvProxyArrayLength_ClipArray( void *pStruct, int objectID, int currentArrayLength )
{
	C_WeaponBallisticBase *pWeapon = ( C_WeaponBallisticBase* )pStruct;

	if( pWeapon )
		pWeapon->GetClipList( ).SetSize( currentArrayLength );
}

#endif

//=========================================================
//=========================================================
BEGIN_NETWORK_TABLE_NOBASE( CWeaponBallisticBase, DT_LocalBallisticWeaponData )

#ifdef GAME_DLL

	SendPropBool( SENDINFO( m_bChamberedRound ) ),
	SendPropInt( SENDINFO( m_iClip ), 8, SPROP_UNSIGNED ),

	SendPropBool( SENDINFO( m_bHammerDown ) ),

	SendPropArray2( 
	   SendProxyArrayLength_ClipArray,
	   SendPropInt( "celement", 0, 4, MAX_CLIPSIZE_BITS, SPROP_UNSIGNED, SendProxy_ClipList ), 
	   MAX_CLIPS,
	   0, 
	   "carray" ),

	SendPropInt( SENDINFO( m_iActiveFiremode ), 2, SPROP_UNSIGNED ),
	
	SendPropTime( SENDINFO( m_flNextForcedPrimaryAttack ) ),
	SendPropInt( SENDINFO( m_iRemainingForcedBullets ), 2, SPROP_UNSIGNED ),

#else

	RecvPropBool( RECVINFO( m_bChamberedRound ) ),
	RecvPropInt( RECVINFO( m_iClip ) ),

	RecvPropBool( RECVINFO( m_bHammerDown ) ),

	RecvPropArray2( 
		RecvProxyArrayLength_ClipArray,
		RecvPropIntWithMinusOneFlag( "celement", 0, SIZEOF_IGNORE, RecvProxy_ClipList ), 
		MAX_CLIPS,
		0, 
		"carray" ),
	
	RecvPropInt( RECVINFO( m_iActiveFiremode ) ),

	RecvPropTime( RECVINFO( m_flNextForcedPrimaryAttack ) ),
	RecvPropInt( RECVINFO( m_iRemainingForcedBullets ) ),
	
#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBallisticBase, DT_WeaponBallisticBase )

BEGIN_NETWORK_TABLE( CWeaponBallisticBase, DT_WeaponBallisticBase )

#ifdef GAME_DLL

	SendPropDataTable( "LocalBallisticWeaponData", 0, &REFERENCE_SEND_TABLE( DT_LocalBallisticWeaponData ), SendProxy_SendLocalWeaponDataTable ),

#else

	RecvPropDataTable( "LocalBallisticWeaponData", 0, 0, &REFERENCE_RECV_TABLE( DT_LocalBallisticWeaponData ) ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

// TODO: redo this
BEGIN_PREDICTION_DATA( CWeaponBallisticBase )

	DEFINE_PRED_FIELD( m_bChamberedRound, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iClip, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD( m_bHammerDown, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD( m_iActiveFiremode, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),

	DEFINE_PRED_FIELD( m_iRemainingForcedBullets, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD_TOL( m_flNextForcedPrimaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),	

END_PREDICTION_DATA( )

#endif

//=========================================================
//=========================================================
CWeaponBallisticBase::CWeaponBallisticBase( )
{
	m_bChamberedRound = false;
	m_iClip = 0;

	m_bHammerDown = true;

	ResetFiremodes( );
	m_flNextFiremodeChange = 0.0f;

#ifdef GAME_DLL

	m_bFirstDeploy = true;

#endif

	m_flNextForcedPrimaryAttack = 0.0f;
	m_iRemainingForcedBullets = 0;

	m_iNumShotsFired = 0;
	m_flLastAttackTime = 0.0f;
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::Spawn( void )
{
	BaseClass::Spawn( );

	// make the current clip full
	m_iClip = GetClipSize( );

	// set the firemodes
	int iFiremodes = ForcedFiremode( );

	if( iFiremodes == INVALID_FIREMODE )
		iFiremodes = GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, iFiremodes );

	SetFiremodes( iFiremodes );
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::CanDeploy( void )
{
	return ( BaseClass::CanDeploy( ) && IsValidClip( ) );
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::HandleDeploy( void )
{
	BaseClass::HandleDeploy( );

#ifdef GAME_DLL

	if( m_bFirstDeploy )
	{
		CINSPlayer *pPlayer = GetINSPlayerOwner( );

		if( pPlayer )
		{
			int iPerferredFiremodeValue = pPlayer->GetCmdValue( CMDREGISTER_PFIREMODE );

			if( iPerferredFiremodeValue != 0 )
			{
				int iPerferredFiremode = iPerferredFiremodeValue - 1;
					
				if( IsValidFiremode( iPerferredFiremode ) )
					m_iActiveFiremode = iPerferredFiremode;
			}
		}
	}

	m_bFirstDeploy = false;

#endif
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::OnDeployReady( void )
{
	Cock( );
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::ItemPostFrame( void )
{
	CBasePlayer *pPlayer = GetOwner( );

	if( !pPlayer )
		return;

	// mask out IN_ATTACK for when forcing primary attack
	bool bForcedPrimaryAttack = ( m_flNextForcedPrimaryAttack != 0.0f );

	if( bForcedPrimaryAttack )
	{
		pPlayer->m_afButtonPressed &= ~IN_ATTACK;
		pPlayer->m_nButtons &= ~IN_ATTACK;
		pPlayer->m_afButtonReleased &= ~IN_ATTACK;
	}

	// call the base itempostframe first
	BaseClass::ItemPostFrame( );

	// try and force an attack
	if( bForcedPrimaryAttack )
	{
		bool bResetBurst = false;

		if( m_iRemainingForcedBullets > 0 )
		{
			if( gpGlobals->curtime >= m_flNextForcedPrimaryAttack )
			{
				if( HandlePrimaryAttack( ) )
				{
					PrimaryAttack( );

					m_iRemainingForcedBullets--;
					m_flNextForcedPrimaryAttack = gpGlobals->curtime + GetBurstCycleTime( );
				}
				else
				{
					bResetBurst = true;
				}
			}
		}
		else
		{
			bResetBurst = true;
		}

		if( bResetBurst )
		{
			m_iRemainingForcedBullets = 0;
			m_flNextForcedPrimaryAttack = 0.0f;
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
		}
	}

	// check firemodes
	if( pPlayer->m_afButtonPressed & IN_FIREMODE )
		CycleFiremodes( );
}

//=========================================================
//=========================================================
Activity CWeaponBallisticBase::GetDrawActivity( void )
{
	// cock it when it hasn't been and the clip is empty
	if( ShouldCock( ) )
		return ACT_VM_READY;

	return BaseClass::GetDrawActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponBallisticBase::GetDryFireActivity( void ) const
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer && pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
		return ACT_VM_ISHOOTDRY;
	else  
		return ACT_VM_DRYFIRE;
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::ShouldEmptyAnimate( void ) const
{
	return IsEmpty( );
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

model_t *CWeaponBallisticBase::GetShellModel( void ) const
{
	const CBulletData &BulletData = CBulletDef::GetBulletData( GetBulletType( ) );
	return BulletData.m_pShellModel;
}

#endif

//=========================================================
//=========================================================
bool CWeaponBallisticBase::IsEmptyAttack( void )
{
	return IsEmpty( );
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::EmptyAttack( void )
{
	if( !m_bHammerDown )
	{
		WeaponSound( EMPTY );

		m_bHammerDown = true;
	}

	SendWeaponAnim( GetDryFireActivity( ) );

	m_flNextEmptyAttack = gpGlobals->curtime + SequenceDuration( ) + 0.1f;
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::CanPrimaryAttack( void )
{
	if( !BaseClass::CanPrimaryAttack( ) )
		return false;

	// allow when already firing
	if( m_flNextForcedPrimaryAttack != 0.0f )
		return true;

	// always allow when fullauto
	if( m_iActiveFiremode == FIREMODE_FULLAUTO )
		return true;

	if( gpGlobals->curtime - m_flLastAttackTime < GetCycleTime( ) )
		return false;

	CBasePlayer *pPlayer = GetOwner( );
	return ( pPlayer && ( pPlayer->m_afButtonPressed & IN_ATTACK ) != 0 );
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::StoreLastPrimaryAttack( void )
{
	return ( m_iActiveFiremode != FIREMODE_FULLAUTO );
}

//=========================================================
//=========================================================
#define TRACERS_FREQ 3

void CWeaponBallisticBase::PrimaryAttack( void )
{
	static int iShotsFired = 0;

	CBasePlayer *pPlayer = GetOwner( );

	if( !pPlayer )
		return;

	bool bBurstFire = ( m_iActiveFiremode == FIREMODE_3RNDBURST );

	float flCycleTime = bBurstFire ? GetBurstCycleTime( ) : GetCycleTime( );

	if( bBurstFire )
		m_iNumShotsFired = BULLETS_3RNDBURST - m_iRemainingForcedBullets;
	else
		m_iNumShotsFired = 0;

	bool bStartedBurst = false;

	if( bBurstFire && m_flNextForcedPrimaryAttack == 0.0f )
	{
		m_flNextForcedPrimaryAttack = gpGlobals->curtime + GetBurstCycleTime( );

		if( m_iRemainingForcedBullets == 0 )
			m_iRemainingForcedBullets = BULLETS_3RNDBURST - 1;

		bStartedBurst = true;
	}

	bool bDoEffects = true;

#ifdef CLIENT_DLL

	if( !DoClientEffects( ) )
		bDoEffects = false;

#endif

	// make a sound
	WeaponSound( SHOT_SINGLE, ( m_iActiveFiremode == FIREMODE_FULLAUTO ) ? m_flNextPrimaryAttack : 0.0f );

	// do effects
	bool bUnperfectShot = true;

#ifdef TESTING

	bUnperfectShot = ( perfectshot.GetInt( ) != 2 );

#endif


	if( bDoEffects && bUnperfectShot )
	{
		// add some recoil
		CreateRecoil( bStartedBurst );

		// PNOTE: muzzle flashes is handled in fx_ins_weaponfx
	}

	// find positions
	Vector vecOrigin, vecForward;
	FindMuzzle( vecOrigin, vecForward, false );

	// fire the bullet
	int iTracerType = ( ShowTracers( ) && ( ( iShotsFired++ % TRACERS_FREQ ) == 0 ) ) ? GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, iTracerType ) : TRACERTYPE_DEFAULT;
	UTIL_FireBullets( this, vecOrigin, vecForward, iTracerType );

#ifdef GAME_DLL

	RegisterShot( );

#endif

	m_flNextPrimaryAttack = gpGlobals->curtime + flCycleTime;
	m_flLastAttackTime = gpGlobals->curtime;

	if( m_flNextTertiaryAttack <= gpGlobals->curtime && bUnperfectShot )
		SendWeaponAnim( GetPrimaryAttackActivity( ) );

	if( !bBurstFire || ( bBurstFire && bStartedBurst ) )
		DoAnimationEvent( PLAYERANIMEVENT_WEAP_FIRE1 );

	// strip round
	StripRound( );

#ifdef ENABLE_BPATTACK_DEBUG

	Msg( "Clip %i\n", m_iClip + ( m_bChamberedRound ? 1 : 0 ) );

#endif
}

//=========================================================
//=========================================================
Activity CWeaponBallisticBase::GetPrimaryAttackActivity( void ) const
{
	CINSPlayer *pOwner = GetINSPlayerOwner( );

	if( pOwner )
	{
		if( IsLastBullet( ) )
		{
			if( pOwner->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
				return ACT_VM_ISHOOT_LAST;
			else
				return ACT_VM_SHOOTLAST;
		}

		if( ( pOwner->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) == 0 )
		{
			if( m_iNumShotsFired < 1 )
				return GetPrimaryAttackActivityRecoil( ACT_VM_PRIMARYATTACK, 0 );

			if( m_iNumShotsFired < 2 )
				return GetPrimaryAttackActivityRecoil( ACT_VM_PRIMARYATTACK, 1 );

			if( m_iNumShotsFired < 3 )
				return GetPrimaryAttackActivityRecoil( ACT_VM_PRIMARYATTACK, 2 );

			return GetPrimaryAttackActivityRecoil( ACT_VM_PRIMARYATTACK, 3 );
		}
	}

	return BaseClass::GetPrimaryAttackActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponBallisticBase::GetPrimaryAttackActivityRecoil( Activity BaseAct, int iRecoilState ) const
{
	if( BaseAct != ACT_VM_PRIMARYATTACK)
		return ACT_VM_PRIMARYATTACK;

	switch( iRecoilState )
	{
		case 0:
			return ACT_VM_PRIMARYATTACK;

		case 1:
			return ACT_VM_RECOIL1;

		case 2:
			return ACT_VM_RECOIL2;
	}

	return ACT_VM_RECOIL2;
}

//=========================================================
//=========================================================
CBaseEntity *CWeaponBallisticBase::GetAttacker( void )
{
	return GetOwner( );
}

//=========================================================
//=========================================================
CBaseEntity *CWeaponBallisticBase::GetInflictor( void )
{
	return this;
}

//=========================================================
//=========================================================
int CWeaponBallisticBase::GetBulletType( void ) const
{
	Clip_t *pClip = GetClipData( );
	return ( pClip ? pClip->m_iBulletType : BULLET_556NATO );
}

//=========================================================
//=========================================================
int CWeaponBallisticBase::GetMuzzleVelocity( void ) const
{
	return GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, iMuzzleVelocity );
}

//=========================================================
//=========================================================
Vector CWeaponBallisticBase::GetSpread( void ) const
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return vec3_origin;

	if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
		GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, Range.vecSpread );
	
	return GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, RangeIronsights.vecSpread );
}

//=========================================================
//=========================================================
int CWeaponBallisticBase::GetRange( void ) const
{
	if( IgnoreRange( ) )
		return MAX_TRACE_LENGTH;

	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return MAX_TRACE_LENGTH;

	if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
		return GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, Range.iMaxRange );

    return GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, RangeIronsights.iMaxRange );
}

//=========================================================
//=========================================================
float CWeaponBallisticBase::GetCycleTime( void )
{
	return GET_WEAPON_DATA_CUSTOM(BallisticWeaponInfo_t, flCycleTime);
}

//=========================================================
//=========================================================
float CWeaponBallisticBase::GetBurstCycleTime( void ) const
{
	return GET_WEAPON_DATA_CUSTOM(BallisticWeaponInfo_t, flBurstCycleTime);
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::CreateRecoil( bool bStartedBurst )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer == NULL )
		return;

	// set the seed
	RandomSeed( CBaseEntity::GetPredictionRandomSeed( ) & 255 );

	// reduce factor for each forced bullet
	float flFactor = 1.0f;

	if( !bStartedBurst && m_iRemainingForcedBullets > 0 )
		flFactor = m_iRemainingForcedBullets * 0.25f;

	// apply the recoil
	bool bBipod = ( pPlayer->GetPlayerFlags( ) & FL_PLAYER_BIPOD ) ? true : false;
	bool bIronsights = ( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) ? true : false;

	QAngle ViewPunch = GetRecoil( bBipod ) * flFactor;

	if( bBipod )
	{
		// do the view punch
		pPlayer->ViewPunch( ViewPunch * GetWeaponDef( )->flRecoilModBipodPunch );

		// do the recoil view punch
		QAngle RecoilViewPunch;
		RecoilViewPunch.x = random->RandomFloat( -1.0f, 1.0f );
		RecoilViewPunch.y = random->RandomFloat( -1.0f, 1.0f );
		RecoilViewPunch.z = 0;

		pPlayer->RecoilViewPunch( RecoilViewPunch * GetWeaponDef( )->flRecoilModBipodDecayLength );
		pPlayer->RecoilViewPunchReset( GetWeaponDef( )->iRecoilModBipodDecayReset );
	}
	else
	{
		/*if( player->GetFlags() & FL_IRONSIGHTS )
		{
		CWeaponINSBase *pWeapon = ToINSWeapon( player->GetActiveWeapon( ) );

		for( int i = 0; i < 2; i++ )
		{
		vec_t &vecNewPart = ( i == 0 ? v_angle.y : v_angle.x );

		if( !pWeapon->ValidISViewAngles( v_angle, vecNewPart, i ) )
		vecNewPart = ( i == 0 ? mv->m_vecAngles.y : mv->m_vecAngles.x );
		}
		}*/

		// do push up recoil
		pPlayer->SnapEyeAngles( ViewPunch * ( bIronsights ? GetWeaponDef( )->flRecoilModPushIronsights : GetWeaponDef( )->flRecoilModPush ), FIXANGLE_RELATIVE );

		// do the recoil view punch
		pPlayer->RecoilViewPunch( ViewPunch * ( bIronsights ? GetWeaponDef( )->flRecoilModDecayIronsights : GetWeaponDef( )->flRecoilModDecay ) );
	}
}

//=========================================================
//=========================================================
QAngle CWeaponBallisticBase::GetRecoil( bool bBipod )
{
	float flRecoilXMin, flRecoilXMax, flRecoilYMin, flRecoilYMax;

	if( bBipod )
	{
		flRecoilXMin = GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, flRecoilXMinBipod );
		flRecoilXMax = GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, flRecoilXMaxBipod );
		flRecoilYMin = GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, flRecoilYMinBipod );
		flRecoilYMax = GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, flRecoilYMaxBipod );
	}
	else
	{
		flRecoilXMin = GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, flRecoilXMin );
		flRecoilXMax = GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, flRecoilXMax );
		flRecoilYMin = GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, flRecoilYMin );
		flRecoilYMax = GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, flRecoilYMax );
	}

	QAngle Recoil;
	Recoil.x = random->RandomFloat( flRecoilXMin, flRecoilXMax );
	Recoil.y = random->RandomFloat( flRecoilYMin, flRecoilYMax );
	Recoil.z = 0.0f;

	return ( Recoil * GetRecoilMod( ) );
}

//=========================================================
//=========================================================
float CWeaponBallisticBase::GetRecoilMod( void ) const
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return 1.0f;

	bool bMoving = pPlayer->IsMoving( );
	bool bIronsights = ( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) ? true : false;

	float flMod = 1.0f;

	if( pPlayer->IsCrouched( ) )
	{
		if( bMoving )
		{
			if( bIronsights )
				flMod = GetWeaponDef( )->flRecoilModCrouchRunningIronsights;
			else
				flMod = GetWeaponDef( )->flRecoilModCrouchRunning;
		}
		else
		{
			if( bIronsights )
				flMod = GetWeaponDef( )->flRecoilModCrouchIronsights;
			else
				flMod = GetWeaponDef( )->flRecoilModCrouch;
		}
	}
	else if( pPlayer->IsProned( ) )
	{
		if( bIronsights )
			flMod = GetWeaponDef( )->flRecoilModProneIronsights;
		else
			flMod = GetWeaponDef( )->flRecoilModProne;
	}
	else
	{
		if( bMoving )
		{
			if( bIronsights )
				flMod = GetWeaponDef( )->flRecoilModRunningIronsights;
			else
				flMod = GetWeaponDef( )->flRecoilModRunning;
		}
		else if( bIronsights )
		{
			flMod = GetWeaponDef( )->flRecoilModIronsights;
		}
	}
	
	int iDamageDecay = pPlayer->GetDamageDecay( );

	if( iDamageDecay > 0 )
		flMod *= 1.0f + ( min( 0.25f, ( iDamageDecay / 100.0f ) ) * 1.5f );

	float flDuration = ( m_fFireDuration > 0.25f ) ? 0.25f : m_fFireDuration;
	float flKickPerc = flDuration / 0.25f;

	flKickPerc = max( flKickPerc, 0.5f );

	return ( flMod * flKickPerc );
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::SetFiremodes( int iFiremodes )
{
	ResetFiremodes( );

	// lots of
	int iFiremodeCount = 0;

	for( int i = 0; i < FIREMODE_COUNT; i++ )
	{
		if( iFiremodes & ( 1 << i ) )
		{
			m_bFiremodes[ i ] = true;
			iFiremodeCount++;
		}
	}

	if( iFiremodeCount == 0 )
	{
		Warning( "ERROR: %s has Invalid Firemodes Definition\n", GetName( ) );

		m_iActiveFiremode = FIREMODE_SEMI;

		return;
	}

	// store if this is
	m_bHasMultipleFiremodes = ( iFiremodeCount > 1 );

	for( int i = 0; i < FIREMODE_COUNT; i++ )
	{
		if( IsValidFiremode( i ) )
			m_iActiveFiremode = i;
	}
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::ResetFiremodes( void )
{
	memset( m_bFiremodes, false, sizeof( m_bFiremodes ) );
	m_bHasMultipleFiremodes = false;

	m_iActiveFiremode = 0;
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::CycleFiremodes( void )
{
	// ensure we can actually scroll and time is on our side
	if( !m_bHasMultipleFiremodes || m_flNextFiremodeChange >= gpGlobals->curtime || m_flNextPrimaryAttack >= gpGlobals->curtime )
		return;

	// iterate through all positions
	int iIteration, iFindMode, iNewMode;
	iIteration = 0;
	iFindMode = m_iActiveFiremode;
	iNewMode = INVALID_FIREMODE;

	while( iIteration < FIREMODE_COUNT )
	{
		iFindMode++;

		if( iFindMode >= FIREMODE_COUNT )
			iFindMode = 0;

		if( IsValidFiremode( iFindMode ) )
		{
			iNewMode = iFindMode;
			break;
		}

		iIteration++;
	}

	// quit out if invalid
	if( iNewMode == INVALID_FIREMODE )
	{
		m_flNextFiremodeChange = gpGlobals->curtime + 1.0f;
		return;
	}

	// set the new mode
	m_iActiveFiremode = iNewMode;

#ifdef GAME_DLL

	// alert the client
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer )
	{
		CReliablePlayerRecipientFilter filter( pPlayer );

		UserMessageBegin( filter, "FireMode" );

			WRITE_BYTE( iNewMode );

		MessageEnd( );
	}

#endif

	// play animations and set time
	if( SendWeaponAnim( GetFireModeActivity( ) ) )
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = gpGlobals->curtime + SequenceDuration( );
	else
		m_flNextFiremodeChange = gpGlobals->curtime + 0.3f;
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::IsValidFiremode( int iFiremode ) const
{
	return m_bFiremodes[ iFiremode ];
}

//=========================================================
//=========================================================
Activity CWeaponBallisticBase::GetFireModeActivity( void ) const
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer )
	{
		int iPlayerFlags = pPlayer->GetPlayerFlags( );

		if( iPlayerFlags & FL_PLAYER_BIPOD )
		{
			if( iPlayerFlags & FL_PLAYER_IRONSIGHTS )
				return ACT_VM_DIFIREMODE;
			else
				return ACT_VM_DFIREMODE;
		}
		else
		{
			if( iPlayerFlags & FL_PLAYER_IRONSIGHTS )
				return ACT_VM_IFIREMODE;
		}
	}

	return ACT_VM_FIREMODE;
}

//=========================================================
//=========================================================
int CWeaponBallisticBase::ForcedFiremode( void ) const
{
	return INVALID_FIREMODE;
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::CanReload( void )
{
	if( !BaseClass::CanReload( ) )
		return false;

	return !IsClipFull( );
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::FinishReload( void )
{
	BaseClass::FinishReload( );

	// ensure the owner is valid and a player
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	// remove clip
	if( !HasClips( ) )
	{
	#ifdef _DEBUG

	#ifdef GAME_DLL

		static const char *pszDLLName = "Server";

	#else

		static const char *pszDLLName = "Client";

	#endif

		char szAssertMsg[ 256 ];
		Q_snprintf( szAssertMsg, sizeof( szAssertMsg ), "(%s) Reload has Finished, but Next Clip is Invalid", pszDLLName );
			
		AssertMsg( false, szAssertMsg );

	#endif

		return;
	}

	// add current clip
	int iClipLeft = TakeClip( );

	if( !IsClipEmpty( ) )
		m_Clips.AddToTail( m_iClip );

	// setup new clip
	m_iClip = iClipLeft;

	// cock if needed
	Cock( );
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::IsEmptyReload( void )
{
	return !HasClips( );
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::HandleEmptyReload( void )
{
#ifdef CLIENT_DLL

	GetINSHUDHelper( )->WeaponInfo( )->ShowAmmoInfo( );

#endif
}

//=========================================================
//=========================================================
int CWeaponBallisticBase::GetClipType( void ) const
{
	return GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, iClipType );
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::IsValidClip( void ) const
{
	return ( GetClipType( ) != INVALID_CLIP );
}

//=========================================================
//=========================================================
Clip_t *CWeaponBallisticBase::GetClipData( void ) const
{
	int iClipType = GetClipType( );
	return ( ( iClipType != INVALID_CLIP ) ? &GetClipDef( )->m_ClipData[ iClipType ] : NULL );
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::HasClips( void ) const
{
	return ( m_Clips.Count( ) > 0 );
}

//=========================================================
//=========================================================
int CWeaponBallisticBase::GetClipCount( void ) const
{
	return m_Clips.Count( );
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::IsClipFull( void )
{
	return ( m_iClip >= GetClipSize( ) );
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::IsClipEmpty( void ) const
{
	return ( m_iClip <= 0 );
}

//=========================================================
//=========================================================
int CWeaponBallisticBase::GetClipSize( void ) const
{
	Clip_t *pClipData = GetClipData( );
	return ( pClipData ? pClipData->m_iBullets : 0 );
}

//=========================================================
//=========================================================
int CWeaponBallisticBase::TakeClip( void )
{
	int iClipLeft = m_Clips[ 0 ];
	m_Clips.Remove( 0 );

	return iClipLeft;
}

//=========================================================
//=========================================================
int CWeaponBallisticBase::NextClip( void ) const
{
	if( !HasClips( ) )
		return 0;

	return m_Clips[ 0 ];
}

//=========================================================
//=========================================================
int CWeaponBallisticBase::GetShellType( void ) const
{
	return INS_SHELL_RIFLE;
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::HasAmmo( void ) const
{
	return ( !IsEmpty( ) || HasClips( ) );
}

//=========================================================
//=========================================================
int CWeaponBallisticBase::GetAmmoCount( void ) const
{
	return GetClipCount( );
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::GiveClip( int iCount )
{
	int iClipSize = GetClipSize( );

	if( iClipSize == 0 )
		return;

	for( int i = 0; i < iCount; i++ )
		m_Clips.AddToTail( iClipSize );
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::ShouldCock( void ) const
{
	return ( !m_bChamberedRound && !IsClipEmpty( ) );
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::Cock( void )
{
	if( !ShouldCock( ) )
		return;

	m_bChamberedRound = true;
	m_bHammerDown = false;

	StripRound( );
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::IsEmpty( void ) const
{
	return ( !m_bChamberedRound && IsClipEmpty( ) );
}

//=========================================================
//=========================================================
bool CWeaponBallisticBase::IsLastBullet( void ) const
{
	return ( m_bChamberedRound && IsClipEmpty( ) );
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::StripRound( void )
{
	if( !IsClipEmpty( ) )
		m_iClip--;
	else if( m_bChamberedRound )
		m_bChamberedRound = false;
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::ForceReady( void )
{
	Cock( );

	BaseClass::ForceReady( );
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

//=========================================================
//=========================================================
CON_COMMAND( ins_weaponstats, "Shows the Stats of the Current Weapon" )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	CWeaponBallisticBase *pWeapon = dynamic_cast< CWeaponBallisticBase* >( pPlayer->GetActiveINSWeapon( ) );

	if( pWeapon )
		pWeapon->PrintStats( );	
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::PrintStats( void )
{
	Msg( "------------------------------------------------\n" );
	Msg( "Weapon Stats: %s\n", GetName( ) );
	Msg( "------------------------------------------------\n" );
	Msg( "Muzzle Velocity: %i / %i\n", GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, iMuzzleVelocity ), GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, iMuzzleVelocityRL ) );
	Msg( "Effective Range: %i / %i\n", GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, Range.iEffectiveRange ), GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, RangeRL.iEffectiveRange ) );
	Msg( "Max Range: %i / %i\n", GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, Range.iMaxRange ), GET_WEAPON_DATA_CUSTOM( BallisticWeaponInfo_t, RangeRL.iMaxRange ) );
	PrintDamageStats( 0 );
	PrintDamageStats( 5 );
	PrintDamageStats( 25 );
	PrintDamageStats( 50 );
	PrintDamageStats( 100 );
	PrintDamageStats( 300 );
	Msg( "------------------------------------------------\n" );
}

//=========================================================
//=========================================================
void CWeaponBallisticBase::PrintDamageStats( int iRange )
{
	int iRangeInches = RoundFloatToInt( iRange * INCHS_PER_METER );

	CTakeDamageInfo tempInfo;
	UTIL_BulletImpact( GetBulletType( ), GetMuzzleVelocity( ), ( float )iRangeInches, &tempInfo );

	Msg( "Damage at %im: %f\n", iRange, tempInfo.GetDamage( ) );
}

#endif