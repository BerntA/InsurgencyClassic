//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for Insurgency
//
// $NoKeywords: $
//====================================================================================//	

#include "cbase.h"
#include "ins_player.h"
#include "viewport_panel_names.h"
#include "ins_gamerules.h"
#include "play_team_shared.h"
#include "ins_obj.h"
#include "player_resource.h"
#include "view_team.h"
#include "ins_spawnprotection_shared.h"
#include "ins_mantlezone_shared.h"
#include "team_lookup.h"
#include "voice_gamemgr.h"
#include "weapon_ballistic_base.h"
#include "ins_squad_shared.h"
#include "engine/ienginesound.h"
#include "pain_helper_shared.h"
#include "equipment_helpers.h"
#include "weapondef.h"
#include "imc_config.h"
#include "ins_touch.h"
#include "ins_recipientfilter.h"
#include "ins_utils.h"
#include "voicemgr.h"
#include "soundenvelope.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CGamePlayerStats::CGamePlayerStats( )
{
	Reset( );
}

CGamePlayerStats &CGamePlayerStats::operator=( const CGamePlayerStats &Src )
{
	m_iGamePoints = Src.m_iGamePoints;
	m_iKillPoints = Src.m_iKillPoints;
	m_iKills = Src.m_iKills;
	m_iFriendlyKills = Src.m_iFriendlyKills;

	return *this;
}

void CGamePlayerStats::Reset( void )
{
	m_iGamePoints = m_iKillPoints = m_iKills = m_iFriendlyKills = 0;
}

void CGamePlayerStats::Increment( int iType, int iValue )
{
	switch( iType )
	{
		case PLAYERSTATS_GAMEPTS:
		{
			m_iGamePoints += iValue;
			return;
		}

		case PLAYERSTATS_KILLPTS:
		{
			m_iKillPoints += iValue;
			return;
		}

		case PLAYERSTATS_KILLS:
		{
			m_iKills += iValue;
			return;
		}

		case PLAYERSTATS_FKILLS:
		{
			m_iFriendlyKills += iValue;
			return;
		}

		case PLAYERSTATS_DEATHS:
		{
			m_iDeaths += iValue;
			return;
		}
	}
}

BEGIN_DATADESC( CINSPlayer )

	DEFINE_FIELD( m_iStamina, FIELD_INTEGER ),
    DEFINE_FIELD( m_iCurrentStance, FIELD_INTEGER ),
    DEFINE_FIELD( m_iLastStance, FIELD_INTEGER ),
	DEFINE_FIELD( m_flStanceTransMarker, FIELD_FLOAT ),

END_DATADESC( )

void SendProxy_CurrentObj( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CINSPlayer *pPlayer = ( CINSPlayer* )pStruct;

	int iValue = INVALID_OBJECTIVE;

	if( pPlayer )
	{
		CINSObjective *pObjective = pPlayer->GetCurrentObjective( );

		if( pObjective )
			iValue = pObjective->GetOrderID( );
	}

	pOut->m_Int = iValue + 1;
}

void SendProxy_AllowHeadTurn( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CINSPlayer *pPlayer = ( CINSPlayer* )pStruct;

	int iValue = 0;

	if( pPlayer )
		iValue = ( gpGlobals->curtime > pPlayer->GetHeadTurnThreshold( ) ) ? 1 : 0;

	pOut->m_Int = iValue;
}

void SendProxy_LeanType( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CINSPlayer *pPlayer = ( CINSPlayer* )pStruct;

	int iValue = PLEANING_NONE;

	if( pPlayer )
		iValue = pPlayer->INSAnim_LeaningType( );

	pOut->m_Int = iValue;
}

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS(player, CINSPlayer);

BEGIN_SEND_TABLE_NOBASE(CINSPlayer, DT_INSPlayerExclusive)

	SendPropDataTable( SENDINFO_DT(m_INSLocal), &REFERENCE_SEND_TABLE(DT_INSLocal) ),

	SendPropBool( SENDINFO(m_bCommander)),

	SendPropBool(SENDINFO(m_bSpawnedViewpoint)),
	SendPropInt("currentobj", 0, SIZEOF_IGNORE, OBJ_IDBITS, 0, SendProxy_CurrentObj),

	SendPropEHandle		( SENDINFO( m_hClippingEntity)),
	SendPropEHandle		( SENDINFO( m_hMantleEntity)),

	SendPropInt( SENDINFO( m_iStamina ), STAMINA_MAX_BITS, SPROP_UNSIGNED ),
	SendPropTime( SENDINFO( m_flStaminaUpdateThreshold ) ),

	SendPropTime	(SENDINFO(m_flStartBunnyHopTime)),
	SendPropFloat	(SENDINFO(m_flBunnyHopLength)),

	SendPropBool	(SENDINFO(m_bIsCustomized)),

	SendPropInt  ( SENDINFO( m_iDamageDecay ) ),
	SendPropTime(SENDINFO(m_flDamageDecayThreshold)),

	SendPropFloat( SENDINFO_VECTORELEM( m_angShakyHands, 0 ), 12, 0, -MAX_SHAKE_RADIUS, MAX_SHAKE_RADIUS ),
	SendPropFloat( SENDINFO_VECTORELEM( m_angShakyHands, 1 ), 12, 0, -MAX_SHAKE_XYSIZE, MAX_SHAKE_XYSIZE ),
	SendPropFloat( SENDINFO( m_flRadius ), 8, SPROP_NOSCALE, 0.0f, MAX_SHAKE_RADIUS ),
	SendPropFloat( SENDINFO( m_flRadiusDesired ), 8, SPROP_NOSCALE, 0.0f, MAX_SHAKE_RADIUS ),

	SendPropArray3(SENDINFO_ARRAY3(m_iCmdRegister), SendPropInt(SENDINFO_ARRAY(m_iCmdRegister))),

	SendPropBool	(SENDINFO(m_bTKPunished)),

	SendPropTime(SENDINFO(m_flNextStanceThreshold)),

	SendPropTime(SENDINFO(m_flViewTransitionLength)),
	SendPropTime(SENDINFO(m_flViewTransitionEnd)),
	SendPropInt(SENDINFO(m_iViewTransitionFrom), 7, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_iViewTransitionTarget), 7, SPROP_UNSIGNED),
	SendPropInt(SENDINFO(m_iViewOffsetBipod), 7),

END_SEND_TABLE()

IMPLEMENT_SERVERCLASS_ST( CINSPlayer, DT_INSPlayer )

	// exclude these
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),
	SendPropExclude( "DT_ServerAnimationData", "m_flCycle" ),	
	SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),

    // local data
	SendPropDataTable( "ins_localdata", 0, &REFERENCE_SEND_TABLE( DT_INSPlayerExclusive ), SendProxy_SendLocalDataTable ),

	// player flags
	SendPropInt( SENDINFO( m_iPlayerFlags ), FL_PLAYER_BITS, SPROP_UNSIGNED ),

    // stance data
    SendPropInt( SENDINFO( m_iCurrentStance ), 2, SPROP_UNSIGNED ),
    SendPropIntWithMinusOneFlag( SENDINFO( m_iLastStance ), 2 ),
	SendPropIntWithMinusOneFlag( SENDINFO( m_iOldStance ), 2 ),
    SendPropFloat( SENDINFO( m_flStanceTransMarker ), 0, SPROP_NOSCALE ),

    // leaning
	SendPropInt( SENDINFO( m_iLeanType ), 2, SPROP_UNSIGNED ),

	// allow headturn
	SendPropInt( "allowht", 0, SIZEOF_IGNORE, 1, SPROP_UNSIGNED, SendProxy_AllowHeadTurn ),

	// ragdoll entity
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),


END_SEND_TABLE( )

//=========================================================
//=========================================================
RankBoundaries_t CINSPlayer::m_RankBoundaries;

//=========================================================
//=========================================================
CINSPlayer::CINSPlayer()
{
	// create animation system
    m_pPlayerAnimState = CreatePlayerAnimState(this, this, LEGANIM_9WAY, true);

    UseClientSideAnimation();

	// init everything
	m_iPlayerFlags = 0;

	ResetBandaging();

	ResetSquad();

	m_iTeamID = INVALID_TEAM;

	m_INSLocal.m_bAllowTeamChange = true;

	m_flLastFFAttack = 0.0f;

	m_pCurrentObjective = NULL;
	m_flObjExitTime = 0.0f;

	m_bSpawnedViewpoint = false;

	m_bFirstSquadChange = true;

	m_iVoiceType = PVOICETYPE_ALL;

	m_bCanShowDeathMenu = false;

	m_iStatsMemberID = MEMBERID_INVALID;

	m_pStaminaSound = NULL;

	ResetSprinting();
    ResetBreathing();

	ResetBunny();
	ResetDamageDecay();

	m_flTimeJoined = 0.0f;

	m_iRank = INVALID_RANK;
	m_bCommander = false;

	m_bChangedTeam = false;

	m_bDeadFade = false;

	m_bDeathIgnored = false;

	for(int i = 0; i < CMDREGISTER_COUNT; i++)
		SetCmdValue(i, 0);

	m_iLastSpawnType = PSPAWN_NONE;

	m_bAllowReinforcement = true;

	m_bHasPowerball = false;

	ResetOrders( );

	m_fNextSuicideTime = 0.0f;

	m_bTKPunished = false;

	m_iStatusType = INVALID_PSTATUSTYPE;
	m_iStatusID = INVALID_PSTATUSID;

	m_flHeadTurnThreshold = 0.0f;

	m_flNextStanceThreshold = 0.0f;

	m_flInvincibilityTheshold = 0.0f;

	ResetPlayerOrders( );

	ResetActions( );
	ResetHelp( );

	ClearLayoutCustomisation( );
	ClearClassPreference( );

	ResetViewTransition( );
}

//=========================================================
//=========================================================
CINSPlayer::~CINSPlayer( )
{
}

//=========================================================
//=========================================================
void CINSPlayer::UpdateOnRemove( void )
{
	// remove ragdoll
	CBaseEntity *pRagdoll = m_hRagdoll;

	if( pRagdoll )
	{
		UTIL_RemoveImmediate( pRagdoll );
		pRagdoll = NULL;
	}

	// remove from team etc
	ExecuteRemoveCommon( );

	// remove from viewpoint
	ViewpointRemove( );

	BaseClass::UpdateOnRemove( );
}

//=========================================================
//=========================================================
CINSPlayer *CINSPlayer::CreatePlayer( const char *className, edict_t *ed )
{
	CINSPlayer::s_PlayerEdict = ed;
	return ( CINSPlayer* )CreateEntityByName( className );
}

//=========================================================
//=========================================================
void CINSPlayer::PreThink(void)
{
	QAngle vOldAngles = GetLocalAngles( );
	QAngle vTempAngles = GetLocalAngles( );

	vTempAngles = EyeAngles( );

	if( vTempAngles[ PITCH ] > 180.0f )
		vTempAngles[ PITCH ] -= 360.0f;

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink( );

	if( m_lifeState < LIFE_DYING )
	{
		// handle various systems
		HandleWalking( );
		SimulateStamina( );
		SimulateDamageDecay( );
		SimulateBreathing( );
		HandleBandaging( );

		// handle obj capturing
		if( IsOutsideCapturing( ) && gpGlobals->curtime > m_flObjExitTime )
			ExitObjective( );

		// handle leaning
		HandleLeaning( );

		// reset forces etc
		m_vecTotalForce = m_vecTotalBulletForce = vec3_origin;

		// check invisibility
		if( m_flInvincibilityTheshold != 0.0f && gpGlobals->curtime > m_flInvincibilityTheshold )
			m_takedamage = DAMAGE_YES;
	}

	// restore old angles
	SetLocalAngles( vOldAngles );
}

//=========================================================
//=========================================================
void CINSPlayer::PostThink( void )
{
	BaseClass::PostThink( );

	// store the eye angles pitch so the client can compute its animation state correctly.
	QAngle angAngles = GetLocalAngles( );
	angAngles[ PITCH ] = 0;
	SetLocalAngles( angAngles );

    m_pPlayerAnimState->Update( m_angEyeAngles[ YAW ], m_angEyeAngles[ PITCH ] );
}

//=========================================================
//=========================================================
void CINSPlayer::UpdateCollisionBounds( void )
{
	switch( m_iCurrentStance )
	{
		case STANCE_STAND:
			SetCollisionBounds( VEC_HULL_MIN, VEC_HULL_MAX );
			break;

		case STANCE_CROUCH:
			SetCollisionBounds( VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX );
			break;

		case STANCE_PRONE:
			SetCollisionBounds( VEC_PRONE_HULL_MIN, VEC_PRONE_HULL_MAX );
			break;
	}
}

//=========================================================
//=========================================================
void CINSPlayer::PlayerRunCommand( CUserCmd *ucmd, IMoveHelper *moveHelper )
{
	BaseClass::PlayerRunCommand( ucmd, moveHelper );
}

//=========================================================
//=========================================================
void CINSPlayer::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr )
{
	if( !m_takedamage )
		return;

	CTakeDamageInfo info = inputInfo;

	// prevent damage that the gamerules doesn't allow
	CBaseEntity *pAttacker = inputInfo.GetAttacker( );

	if( pAttacker && pAttacker->IsPlayer( ) )
	{
		CBaseEntity *pAttacker = inputInfo.GetAttacker( );

		if( !INSRules( )->PlayerAdjustDamage( this, pAttacker, info ) )
			return;

		// PNOTE: need a system that takes the inflictor stuff into account
		/*CINSPlayer *pPlayer = ToINSPlayer( pAttacker );

		if( pPlayer )
		{
			CBaseCombatWeapon *pActiveWeapon = pPlayer->GetActiveWeapon();

			if(pActiveWeapon)
				pPlayer->BumpWeaponStat(pActiveWeapon->GetWeaponID(), UTIL_WeaponStatisticType( ptr->hitgroup ) );
		}*/
	}

	// kill them dead if has met tolerance or scale by multiplyer
	HitGroupData_t &HitGroupData = m_CustomData.m_HitGroupData[ ptr->hitgroup ];

	if( HitGroupData.m_iHitTolerance == m_iHitCounts[ ptr->hitgroup ] )
		info.AddDamageType( DMG_INSTANT );
	else
		info.ScaleDamage( HitGroupData.m_flMultiplyer );

	BaseClass::TraceAttack( info, vecDir, ptr );
}

//=========================================================
//=========================================================
#define BULLET_IMPULSE_FACTOR 0.1f

int CINSPlayer::OnTakeDamage( const CTakeDamageInfo &info )
{
	if( info.GetDamageType( ) & DMG_BULLET )
		m_vecTotalBulletForce += info.GetDamageForce( ) * BULLET_IMPULSE_FACTOR;
	else
		m_vecTotalForce += info.GetDamageForce( );

	return BaseClass::OnTakeDamage( info );
}

//=========================================================
//=========================================================
int	CINSPlayer::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	// grab the vector of the incoming attack (pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit)
	Vector vecDir = vec3_origin;

	if( info.GetInflictor( ) )
	{
		vecDir = info.GetInflictor( )->WorldSpaceCenter( ) - Vector ( 0, 0, 10 ) - WorldSpaceCenter( );
		VectorNormalize( vecDir );
	}

	g_vecAttackDir = vecDir;

	CBaseEntity *pAttacker = info.GetAttacker( );

	if( !pAttacker )
		return 0;

	// do the damage
	if( m_takedamage != DAMAGE_EVENTS_ONLY )
	{
		int iDamage = info.GetDamage( );

		CINSPlayer *pAttackerPlayer = ( pAttacker->IsPlayer( ) ? ToINSPlayer( info.GetAttacker( ) ) : NULL );

		// take action according to who was hit
		if( pAttackerPlayer != NULL && pAttackerPlayer != this )
		{
			// send out FF message if on the same team
			if( OnSameTeam( this, pAttackerPlayer ) )
			{
				if( pAttackerPlayer->CanSendFFMessage( ) )
				{
					CTeamRecipientFilter Filter( pAttackerPlayer->GetTeamID( ), true );

					UserMessageBegin( Filter, "FFMsg" );

						WRITE_SHORT( pAttackerPlayer->entindex( ) );

					MessageEnd( );

					// don't resend straight away
					pAttackerPlayer->ExtendFFMessage( );

					// send out hint
					UTIL_SendHint( pAttackerPlayer, HINT_FHIT );
				}
			}
			else if( !suppresskillhint.GetBool( ) )
			{
				UTIL_SendHint( pAttackerPlayer, HINT_NKILLCONF );
			}

			// register the damage
			/*CBaseCombatWeapon *pInflictorWeapon = dynamic_cast<CBaseCombatWeapon*>(info.GetInflictor());

			if(pInflictorWeapon)
				IncrementWeaponStat(pInflictorWeapon->GetWeaponID(), WEAPONSTATS_DAMAGE, iDamage);*/
		}

		int iDamageType = info.GetDamageType( );

		// take all the players health of an "instant-kill"
		bool bForcedKill = ( iDamageType & DMG_INSTANT ) ? true : false;
		
		if( bForcedKill )
			iDamage = m_iHealth;

		// fiddle with stats (when valid attacker, not attacked himself and not developer damage)
		if( pAttackerPlayer && pAttackerPlayer != this && ( ( iDamageType & DMG_DEVELOPER ) == 0 ) )
		{
			if( bForcedKill )
			{
				AddDamageTaken( pAttackerPlayer, DAMAGEINFO_SERIOUS );
			}
			else
			{
				// make sure you cap the damage so that players don't end up with two
				// serious damagetaken's
				int iCappedDamage = iDamage;

				if( ( m_iHealth + iCappedDamage ) > GetMaxHealth( ) )
					iCappedDamage = m_iHealth - ( GetMaxHealth( ) - m_iHealth );

				// add the damage
				int iDamageInfoType = GetDamageInfoType( iCappedDamage );

				if( iDamageInfoType >= DAMAGEINFO_MODERATE )
					AddDamageTaken( pAttackerPlayer, iDamageInfoType );
			}
		}

		// when we're not in buddha mode, take health etc
		if( ( m_debugOverlays & OVERLAY_BUDDHA_MODE ) == 0 )
		{
			// take away health
			m_iHealth -= max( 0, iDamage );

			// add damage decay
			m_iDamageDecay += iDamage;
			m_iDamageDecay = min( m_iMaxHealth, iDamage );

			m_flDamageDecayThreshold = gpGlobals->curtime + DAMAGE_UPDATE_TIME;

			// punch the viewmodel

			// HACKHACK: can anyone say prediction?
			float flFraction = min( VMHIT_DAMAGELIMIT, iDamage ) / ( float )VMHIT_DAMAGELIMIT;

			Vector vecViewDir;
			AngleVectors( GetAbsAngles( ), &vecViewDir );

			if( DotProduct( vecDir, vecViewDir ) > 0 )
				flFraction *= -1.0f;

			ViewPunchReset( flFraction * 10.0f );
			ViewPunch( QAngle( flFraction * 10.0f, flFraction * 5.0f, 0.0f ) );

			// can't look for a while
			m_flHeadTurnThreshold = gpGlobals->curtime + PLAYER_HEADTIME_HIT;

			// send over radio
			if( m_iHealth > 0 && iDamageType & DMG_BULLET )
				SendAction( PACTION_HIT );
		}

		// send out event
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "player_hurt", true );

		if ( pEvent )
		{
			pEvent->SetInt( "userid", GetUserID( ) );

			if( pAttacker )
				pEvent->SetInt( "attacker", pAttacker->entindex( ) );
			else
				pEvent->SetInt( "attacker", 0 );

			pEvent->SetInt( "dmg_health", iDamage );
			pEvent->SetInt( "hitgroup", m_LastHitGroup );

			CBaseCombatWeapon *pInflictorWeapon = dynamic_cast< CBaseCombatWeapon* >( info.GetInflictor( ) );

			if( pInflictorWeapon )
				pEvent->SetString( "weapon", pInflictorWeapon->GetClassname( ) );
			else
				pEvent->SetString( "weapon", "unknown" );
				
			gameeventmanager->FireEvent( pEvent );
		}
	}

	// nudge the player slightly
	if( info.GetInflictor( ) && ( GetMoveType( ) == MOVETYPE_WALK ) && !pAttacker->IsSolidFlagSet( FSOLID_TRIGGER ) && !( GetPlayerFlags( ) & FL_PLAYER_BIPOD && m_iHealth > 0 ) )
	{
		bool bProned = IsProned( );
		bool bBulletDamage = ( info.GetDamageType( ) & DMG_BULLET ) ? true : false;

		if( !( bProned && bBulletDamage ) )
		{
			int iAdjustedDamage = info.GetDamage( );

			if( info.GetDamageType( ) & DMG_BLAST )
				iAdjustedDamage *= 1.35f;

			Vector vecSize = WorldAlignSize( );
			vecSize.z = min( vecSize.z, 20.0f );

			float flForce = iAdjustedDamage * ( ( vecSize.x * vecSize.y * vecSize.z ) / ( 32.0f * 32.0f * 72.0f ) ) * 4;

			flForce = min( flForce, 1000.0f );

			if( bProned )
				flForce = min( flForce, 20.0f );
			else if( bBulletDamage )
				flForce = min( flForce, 190.0f );

			ApplyAbsVelocityImpulse( g_vecAttackDir * -flForce );
		}
	}

	return 1;
}

//=========================================================
//=========================================================
void CINSPlayer::Event_Killed( const CTakeDamageInfo &info )
{
	Vector vecForce = m_vecTotalForce;

	// only apply the bullet force if we haven't recieved any other forces
	// and they're roughly going in the same direction as the the velocity
	if( m_vecTotalForce.Length( ) == 0.0f && ( m_vecTotalBulletForce.Length( ) == 0.0f || DotProduct( GetAbsVelocity( ), m_vecTotalBulletForce ) > 0.0f ) )
		vecForce += m_vecTotalBulletForce;

	// update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( vecForce );

	// create ragdoll
	CreateRagdollEntity( vecForce );

	// unmask stats
	UnmaskStats( );

	// don't allow a respawn yet
	m_bAllowReinforcement = false;

	// don't reset invis
	m_flInvincibilityTheshold = 0.0f;

	// destory exhausible etc
	CBaseCombatWeapon *pWeapon = GetActiveWeapon( );

	if( pWeapon && pWeapon->IsExhaustible( ) && !pWeapon->HasAmmo( ) )
		RemoveWeapon( pWeapon );

	// run rest of code
	BaseClass::Event_Killed( subinfo );

	// fade to black
	FadeToBlack( PLAYER_DEATHFADEOUT );

	CBaseEntity *pRagdoll = m_hRagdoll;

	// ensure when the final damage is dissolve, it does the dissolve effect
	CINSRagdoll *pINSRagdoll = dynamic_cast< CINSRagdoll* >( pRagdoll );

	if( pINSRagdoll )
	{
		if( info.GetDamageType( ) & DMG_DISSOLVE )
			pINSRagdoll->GetBaseAnimating( )->Dissolve( NULL, gpGlobals->curtime, ENTITY_DISSOLVE_NORMAL );

		pINSRagdoll->SetDroppedWeapon( GetActiveWeapon( ) );
	}

	// attempt a squad update
	AttemptSquadUpdate( );
}

//=========================================================
//=========================================================
void CINSPlayer::Event_Dying(const CTakeDamageInfo& info)
{
	BaseClass::Event_Dying(info);

	// stop stamina sound
	if( m_pStaminaSound )
	{
		CSoundEnvelopeController::GetController( ).SoundDestroy( m_pStaminaSound );
		m_pStaminaSound = NULL;
	}

	// PNOTE: other objective exits are handled by the physics
	if( IsOutsideCapturing( ) )
		ExitObjective( );

	// had to add this here due to health network reasons
	// -- but it sucks!
	SetObserverMode(OBS_MODE_DEATHCAM);
}

//=========================================================
//=========================================================
void CINSPlayer::UpdateClientData( void )
{
	CReliablePlayerRecipientFilter Filter( this );

	if( m_DmgTake > 0 || m_bitsDamageType != 0 )
	{
		m_DmgTake = clamp( m_DmgTake, 0, 255 );

		UserMessageBegin( Filter, "Damage" );

			WRITE_BYTE( m_DmgTake );
			WRITE_LONG( m_bitsDamageType );

			if( m_bitsDamageType & DMG_VALIDFORCE )
			{
				WRITE_FLOAT( m_DmgOrigin.x );
				WRITE_FLOAT( m_DmgOrigin.y );
				WRITE_FLOAT( m_DmgOrigin.z );
			}

		MessageEnd( );
	
		m_DmgTake = 0;
		m_bitsDamageType = 0;
	}

	BaseClass::UpdateClientData( );
}

//=========================================================
//=========================================================
void CINSPlayer::FinishDeathThink( void )
{
	BaseClass::FinishDeathThink( );

	// fade out the black, when possible
	float flFadeOutLength = 0.0f;

	if( m_bChangedTeam )
		flFadeOutLength = FADEOUT_TEAMCHANGE_TIME;
	else if( !INSRules( )->ShouldForceDeathFade( ) && ( ( GetDeathMenuType( ) != DEATHINFOTYPE_SHOW_BLACK ) || m_bDeathIgnored ) )
		flFadeOutLength = PLAYER_DEATHFADEIN_QUICK;

	if( flFadeOutLength > 0.0f )
		FadeOutBlack( flFadeOutLength );

	// show squad selection panel if slain from team change
	if( m_bChangedTeam )
	{
		Spawn( );

		if( OnPlayTeam( ) )
			ShowViewPortPanel( PANEL_SQUADSELECT, true, NULL );
	}
	else
	{
		// show the death panel, when possible
		if( GetDeathMenuType( ) != DEATHINFOTYPE_NONE )
			ShowViewPortPanel( PANEL_DEATHINFO );

		// start observing
		CBaseEntity *pKillingEntity = m_LastKiller;

		if( pKillingEntity && pKillingEntity->IsPlayer( ) && pKillingEntity != this )
		{
			StartObserverMode( OBS_MODE_CHASE );
			SetObserverTarget( pKillingEntity );
		}
		else
		{
			m_hObserverTarget.Set( NULL );
			StartObserverMode( OBS_MODE_ROAMING );
		}
	}

	// we can now reinforce
	m_bAllowReinforcement = true;

	// tell the gamerules
	INSRules( )->PlayedFinishedDeathThink( this );
}

//=========================================================
//=========================================================
bool CINSPlayer::StartObserverMode( int iMode )
{
	if( iMode > OBS_MODE_NONE )
		ResetConcussionEffect( );

	StanceReset( );

	return BaseClass::StartObserverMode( iMode );
}

//=========================================================
//=========================================================
bool CINSPlayer::SetObserverMode(int iMode)
{
	if( iMode < OBS_MODE_NONE || iMode > OBS_MODE_ROAMING )
		return false;

	if( m_iObserverMode == iMode )
		return true;

	// check deadcam vars
	if( iMode > OBS_MODE_FIXED && OnPlayTeam( ) && INSRules( )->IsModeRunning( ) )
	{
		int iDeadCamMode = INSRules( )->RunningMode( )->GetDeadCamMode( );
		int iDeadCamTargets = INSRules( )->RunningMode( )->GetDeadCamTargets( );

		// correct the modes if we're restricted to our team/squad
		if( iDeadCamTargets != OBS_ALLOWTARGETS_ALL && iDeadCamMode == OBS_ALLOWTARGETS_ALL )
			iDeadCamMode = OBS_ALLOWMODE_INEYECHASE;

		switch( iDeadCamMode )
		{
			case OBS_ALLOWMODE_ALL:
			{
				break;									// no restrictions
			}

			case OBS_ALLOWMODE_INEYECHASE:
			{
				if( iMode == OBS_MODE_CHASE )
					break;								// only allow them to chase or in eye
			}

			case OBS_ALLOWMODE_INEYE:
			{
				iMode = OBS_MODE_IN_EYE;
				break;
			}

			case OBS_ALLOWMODE_NONE:
			{
				iMode = OBS_MODE_FIXED;					// don't allow anything
				break;
			}
		}
	}

	// remember mode if we were really spectating before
	if( m_iObserverMode > OBS_MODE_DEATHCAM )
		m_iObserverLastMode = m_iObserverMode;

	if( iMode == OBS_MODE_ROAMING && m_iObserverMode == OBS_MODE_DEATHCAM && m_hRagdoll )
	{
		CBaseEntity *pRagdoll = m_hRagdoll;

		// calculate back from the ragdoll
		Vector vecEnd = pRagdoll->GetAbsOrigin( );
		vecEnd.z += 128.0f;

		trace_t tr;
		UTIL_TraceLine( pRagdoll->GetAbsOrigin( ), vecEnd, MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &tr );

		Vector vecDesiredOrigin;

		if( tr.fraction < 1.0 )
		{
			// a few units down
			Vector vecCeilingEnd;

			vecCeilingEnd = tr.endpos;
			vecCeilingEnd.z -= 16.0f;

			vecDesiredOrigin = vecCeilingEnd;
		}
		else
		{
			vecDesiredOrigin = vecEnd;
		}

		SetAbsOrigin( vecDesiredOrigin );

		// now look down at the player
		QAngle angDesiredAngles;

		Vector vecDir = ( pRagdoll->GetAbsOrigin( ) - vecDesiredOrigin );
		VectorAngles( vecDir, angDesiredAngles );

		ForceSnapAngles( angDesiredAngles );
	}

	return BaseClass::SetObserverMode( iMode );
}

//=========================================================
//=========================================================
bool CINSPlayer::IsValidObserverTarget(CBaseEntity *pTarget)
{
	if(!BaseClass::IsValidObserverTarget(pTarget))
		return false;

	if(INSRules()->IsModeRunning() && INSRules()->RunningMode()->GetStatus(GAMERUNNING_ENDGAME) &&
		dynamic_cast<CINSObjective*>(pTarget) != NULL)
	{
		// when we're ending, if we're an objective ... allow it
		return true;
	}
	else if(!pTarget->IsPlayer())
	{
		// otherwise, just track players
		return false;
	}

	if( pTarget == this )
		return false; // We can't observe ourselves.

	CINSPlayer *pPlayer = ToINSPlayer(pTarget);

	if(pPlayer->IsEffectActive( EF_NODRAW)) // don't watch invisible players
		return false;

	if(pPlayer->m_lifeState == LIFE_RESPAWNABLE) // target is dead, waiting for respawn
		return false;

	if(pPlayer->m_lifeState == LIFE_DEAD || pPlayer->m_lifeState == LIFE_DYING)
	{
		if ((pPlayer->m_flDeathTime + DEATH_ANIMATION_TIME ) < gpGlobals->curtime)
			return false;	// allow watching until 3 seconds after death to see death animation
	}

	// check forcecamera settings for active players
	if(OnPlayTeam() && INSRules()->IsModeRunning())
	{
		int iDeadCamTargets = INSRules()->RunningMode()->GetDeadCamTargets();

		if(iDeadCamTargets == OBS_ALLOWTARGETS_TEAM || iDeadCamTargets == OBS_ALLOWTARGETS_SQUAD)
		{
			if(!OnSameTeam(this, pPlayer))
				return false;

			if(iDeadCamTargets == OBS_ALLOWTARGETS_SQUAD && !OnSameSquad(this, pPlayer))
				return false;
		}
	}

	return true;
}

//=========================================================
//=========================================================
void CINSPlayer::CheckObserverSettings(void)
{
	BaseClass::CheckObserverSettings( );

	// ensure that the var's haven't changed
	if(OnPlayTeam() && INSRules()->IsModeRunning())
	{
		int iDeadCamTargets = INSRules()->RunningMode()->GetDeadCamTargets();

		if(iDeadCamTargets == OBS_ALLOWTARGETS_TEAM || iDeadCamTargets == OBS_ALLOWTARGETS_SQUAD)
		{
			if(m_iObserverMode == OBS_MODE_ROAMING)
			{
				SetObserverMode(OBS_MODE_IN_EYE);
				return;
			}
		}
	}

	// check if our spectating target is still a valid one
	if (  m_iObserverMode == OBS_MODE_IN_EYE || m_iObserverMode == OBS_MODE_CHASE )
	{
		if ( !IsValidObserverTarget( m_hObserverTarget.Get() ) )
		{
			// our target is not valid, try to find new target
			CBaseEntity * target = FindNextObserverTarget( false );
			if ( target )
			{
				// switch to new valid target
				SetObserverTarget( target );	
			}
			else
			{
				// couldn't find new target, switch to temporary mode
				if ( !INSRules( )->IsModeRunning( ) || INSRules( )->RunningMode( )->GetDeadCamMode( ) == OBS_ALLOWMODE_ALL )
				{
					// let player roam around
					ForceObserverMode( OBS_MODE_ROAMING );
				}
				else
				{
					// fade the player out if there is an invalid target
					if( !m_bDeadFade )
					{
						FadeToBlack( PLAYER_FIXEDFADE );
						m_bDeadFade = true;
					}

					// fix player view right where it is
					ForceObserverMode( OBS_MODE_DEATHCAM );
					m_hObserverTarget.Set( NULL ); // no target to follow
				}
			}
		}

		CBasePlayer *target = ToBasePlayer( m_hObserverTarget.Get() );

		// for ineye mode we have to copy several data to see exactly the same 

		if ( target && m_iObserverMode == OBS_MODE_IN_EYE )
		{
			// Pongles [

			// NOTE: might need to copy m_iCurStance ?
			int flagMask =	FL_ONGROUND;

			int flags = target->GetFlags() & flagMask;

			if ( (GetFlags() & flagMask) != flags )
			{
				flags |= GetFlags() & (~flagMask); // keep other flags
				ClearFlags();
				AddFlag( flags );
			}

			// Pongles ]

			if ( target->GetViewOffset() != GetViewOffset()	)
			{
				SetViewOffset( target->GetViewOffset() );
			}
		}
	}

	// fade out when needed
	if( m_bDeadFade && m_iObserverMode != OBS_MODE_DEATHCAM && m_iObserverMode != OBS_MODE_FIXED )
	{
		FadeOutBlack( PLAYER_FIXEDFADE );
		m_bDeadFade = false;
	}
}

//=========================================================
//=========================================================
bool CINSPlayer::ClientCommand(const CCommand& args)
{
	const char* pszCommand = args[0];

	if ( stricmp( pszCommand, "skin" ) == 0 )
	{
		if (args.ArgC()>=2)
		{
			int iSkin=strtol(args[1],NULL,16); //base 16, see notes in playercust.h
			modelcustomization_t& MdlCust=GetModelCustomization(this);
			if (MdlCust.aSkins.Count()>iSkin)
			{
				if (!(MdlCust.aSkins[iSkin].iFlags&CUSTOMIZE_FL_ITEMDEPENDENT))
				{
					m_nSkin=MdlCust.aSkins[iSkin].iSkin;
					m_bIsCustomized=true;
				}
				else
				{
					engine->ClientPrintf(edict(),"Invalid skin (CUSTOMIZE_FL_ITEMDEPENDENT)!\n");
				}
			}
			else
			{
				engine->ClientPrintf(edict(),"Invalid skin (overflow)!\n");
			}
		}
		else
		{
			engine->ClientPrintf(edict(),"Usage: \n   skin <iSkin>\n");
		}
		return true;
	}
	else if ( stricmp( pszCommand, "body" ) == 0 )
	{
		if (args.ArgC()>=3)
		{
			int iBody=strtol(args[1],NULL,16); //base 16, see notes in playercust.h
			int iModel=strtol(args[2],NULL,16); //base 16, see notes in playercust.h
			modelcustomization_t& MdlCust=GetModelCustomization(this);
			if (MdlCust.aBodygroups.Count()>iBody)
			{
				if (MdlCust.aBodygroups[iBody].aSubmodels.Count()>iModel)
				{
					if (!*MdlCust.aBodygroups[iBody].aSubmodels[iModel].pszItemcode)
					{
						SetBodygroup(MdlCust.aBodygroups[iBody].iGroup,MdlCust.aBodygroups[iBody].aSubmodels[iModel].iGroupValue);
						m_bIsCustomized=true;
					}
					else
					{
						engine->ClientPrintf(edict(),"Invalid submodel (pszItemcode)!\n");
					}
				}
				else
				{
					engine->ClientPrintf(edict(),"Invalid submodel (overflow)!\n");
				}
			}
			else
			{
				engine->ClientPrintf(edict(),"Invalid bodygroup (overflow)!\n");
			}
		}
		else
		{
			engine->ClientPrintf(edict(),"Usage: \n   body <iBodygroup> <iSubmodel>\n");
		}
		return true;
	}
	else if( stricmp( pszCommand, PCMD_INITALSPAWN ) == 0 )
	{
		if( GetTeamID( ) != TEAM_UNASSIGNED )
			return true;

#ifndef STATS_PROTECTION

		FadeOutBlack( INITIAL_FADEIN );

#endif

		INSRules( )->PlayerInitialSpawn( this );

		return true;
	}
	else if( FStrEq( pszCommand, PCMD_PORESPONSE ) )
	{
		// PCMD_PORESPONSE "type"
		if( args.ArgC() != 2 )
			return true;

		PlayerOrderResponse( atoi( args[1] ) );
	}
	else if( FStrEq( pszCommand, PCMD_FINISHDI ) )
	{
		if( !INSRules( )->IsModeRunning( ) )
			return true;

		// if we haven't been told to show the death info, the client
		// must have sent it ... so just ignore it
		if( !m_bCanShowDeathMenu )
			return true;

		// don't want to fade again
		m_bCanShowDeathMenu = false;

		// fade back in when needed
		if( GetDeathMenuType( ) == DEATHINFOTYPE_SHOW_BLACK && !INSRules( )->ShouldForceDeathFade( ) )
			FadeOutBlack( PLAYER_DEATHFADEIN );

		return true;
	}
	else if( FStrEq( pszCommand, PCMD_STATUSBCAST ) )
	{
		KeyValues *pData = new KeyValues( "voicedata" );
		pData->SetInt( "type", m_iStatusType );
		pData->SetInt( "id", m_iStatusID );

		UTIL_SendVoice( VOICEGRP_PLAYER_STATUS, this, pData );

		return true;
	}
	else if( FStrEq( pszCommand, PCMD_NEEDSHELP ) )
	{
		SetNeedsHelp( );

		return true;
	}
	else if ( FStrEq( pszCommand, PCMD_VOICECYCLE ) )
	{
		extern ConVar sv_alltalk;

		// ensure we can switch to squad
		if( m_iVoiceType == PVOICETYPE_SQUAD )
		{
			m_iVoiceType = PVOICETYPE_ALL;

			// if we can alltalk, exit it now, otherwise rotate again
			if(sv_alltalk.GetBool( ) )
				return true;
		}

		m_iVoiceType++;

		return true;
	}
	else if( stricmp( pszCommand, "cmdr" ) == 0 )
	{
		if( args.ArgC() != 3 )
			return false;

		UTIL_UpdateCommandRegister( this, atoi( args[1] ), atoi( args[2] ) );
		return true;
	}
	else if( stricmp( pszCommand, "markme" ) == 0)
	{
		if( !IsDeveloper( ) )
			return false;

		m_nRenderFX = ( ( m_nRenderFX == kRenderFxNone ) ? kRenderFxHologram : kRenderFxNone );
		return true;
	}
	
	return BaseClass::ClientCommand(args);
}

//=========================================================
//=========================================================
void CINSPlayer::Precache(void)
{
	BaseClass::Precache();

	PrecacheScriptSound("HL2Player.PickupWeapon");

	PrecacheScriptSound( "INSPlayer.ProneFrom" );
	PrecacheScriptSound( "INSPlayer.ProneTo" );
	PrecacheScriptSound( "INSPlayer.Sprinting" );
}

//=========================================================
//=========================================================
#define STATS_WAIT_LOGIN 2.5f

void CINSPlayer::Spawn( void )
{
	// forget about the ragdoll
	m_hRagdoll = NULL;

	// when not observer, remove all items
	if( m_iLastSpawnType == PSPAWN_NONE )
		RemoveAllItems( );

	// get spawntype
	int iSpawnType = INSRules( )->PlayerSpawnType( this );
	m_iLastSpawnType = iSpawnType;

	// set the player model
	bool bValidModel = false;

	if( OnPlayTeam( ) && iSpawnType == PSPAWN_NONE )
	{
		CPlayerModelData *pModelData = GetPlayerModelData( );
		Assert( pModelData );

		if( pModelData )
		{
			SetModel( pModelData->GetPath( ) );
			bValidModel = true;
		}
		else
		{
			AssertMsg( false, "CINSPlayer::Spawn, Invalid Model" );
		}
	}

	if( !bValidModel )
		SetModel( GetTeam( )->GetModel( ) );

	// ensure valid layers
	m_pPlayerAnimState->EnsureValidLayers( );

	// let the baseclass spawn
	BaseClass::Spawn( );

	// handle spawn-type
	if( iSpawnType == PSPAWN_NONE )
	{
		// get spawnspot
		CBaseEntity *pSpawnEntity = INSRules( )->GetPlayerSpawnSpot( this );
		Assert( pSpawnEntity );

		if( pSpawnEntity )
		{
			SetEntityPosition( pSpawnEntity );

			// ensure the right entity
			CSpawnPoint *pSpawnPoint = dynamic_cast< CSpawnPoint* >( pSpawnEntity );

			if( pSpawnPoint )
			{
				// set stance
				m_iCurrentStance = pSpawnPoint->GetStance( );

				// set invincibility
				CINSObjective *pObjParent = pSpawnPoint->GetParent( );

				if( pObjParent )
				{
					int iInvincibilityTime = pObjParent->GetInvincibilityTime( );

					if( iInvincibilityTime > 0 )
					{
						m_takedamage = DAMAGE_NO;
						m_flInvincibilityTheshold = gpGlobals->curtime + iInvincibilityTime;
					}
				}
			}
		}
	}

	// handle observer
	if( iSpawnType == PSPAWN_OBSERVER )
	{
		StartObserverMode( OBS_MODE_ROAMING );

		CBaseEntity *pSpawnSpot = INSRules( )->GetViewpoint( );

		if( pSpawnSpot )
			SetEntityPosition( pSpawnSpot );
	}
	else
	{
		StopObserverMode( );
	}

	// handle viewpoint
	if( iSpawnType == PSPAWN_VIEWPOINT )
	{
		SpawnDead( );

		if( !m_bSpawnedViewpoint )
			ViewpointAdd( );
	}
	else
	{
		ViewpointRemove( );
	}

	// don't interpolate next frame
	RemoveEffects( EF_NOINTERP );

	// they're on the ground
	AddFlag( FL_ONGROUND );

	// reset vars
	m_bChangedTeam = false;
	m_bChangedSquad = false;

	ResetOrders( );
	ResetStatus( );
	
	m_NextSquadData.Reset( );

	m_bDeadFade = false;
	m_bDeathIgnored = false;

	m_flNextPickupSound = 0.0f;

	ResetSprinting( );
	ResetSprinting( );
	ResetDamageDecay( );
	ResetBreathing( );
	ResetViewTransition( );

	ResetActions( );
	ResetHelp( );

	ResetConcussionEffect( );

	if( m_flTimeJoined == 0.0f )
		m_flTimeJoined = gpGlobals->curtime;

	m_hClippingEntity	= NULL;
	m_hMantleEntity		= NULL;

	ResetBunny();

	ResetBandaging();

	m_flLastFFAttack = 0.0f;

	m_DamageTaken.RemoveAll();

	m_bCanShowDeathMenu = false;

	m_bAllowReinforcement = true;

	ShowViewPortPanel( PANEL_DEATHINFO, false );

	m_bTKPunished = false;

	// PNOTE: this is due to lack of end-game dialog etc

	// clear any screenfade (unless ending or changing team)
	/*if( INSRules( )->IsModeRunning( ) && INSRules( )->RunningMode( )->GetStatus( GAMERUNNING_ENDGAME ) )
	{
		FadeToBlack( 0.0f );
	}
	else
	{*/
		color32 nothing = { 0, 0, 0, 255 };
		UTIL_ScreenFade( this, nothing, 0, 0, FFADE_IN | FFADE_PURGE );
	/*}*/

	ClearPain( );

	m_flHeadTurnThreshold = 0.0f;

	// Makes sure the Objective we're in has us deRegistered!
	ResetCurrentObj( );
	CBaseEntity::PhysicsRemoveTouchedList( this );

	// tell the gamerules
	if( iSpawnType == PSPAWN_NONE )
		INSRules( )->PlayerSpawn( this );

	// spawn an event
	IGameEvent *pEvent = gameeventmanager->CreateEvent( "player_spawn", true );

	if ( pEvent )
	{
		pEvent->SetInt( "userid", GetUserID( ) );
		pEvent->SetInt( "team", GetTeamID( ) );
		pEvent->SetBool( "dead", !IsRunningAround( ) );
		gameeventmanager->FireEvent( pEvent );
	}

	//deathz0rz [

	// TODO: this needs tidying

	if ( !m_bIsCustomized && bValidModel )
	{
		modelcustomization_t& MdlCust=GetModelCustomization(this);
		if (MdlCust.bLoaded)
		{
			static CUtlVector<int> aiApplicable;
			int i,j;
			aiApplicable.RemoveAll();
			for (i=0;i<MdlCust.aSkins.Count();i++)
			{
				if (MdlCust.aSkins[i].bLoaded&&MdlCust.aSkins[i].iFlags&CUSTOMIZE_FL_DEFAULT)
				{
					aiApplicable.AddToTail(i);
				}
			}
			skincustomization_t &Skin=MdlCust.aSkins[aiApplicable[random->RandomInt(0,aiApplicable.Count()-1)]];
			m_nSkin=Skin.iSkin;
			for (j=0;j<MdlCust.aBodygroups.Count();j++)
			{
				if (!MdlCust.aBodygroups[j].bLoaded)
					continue;
				aiApplicable.RemoveAll();
				for (i=0;i<MdlCust.aBodygroups[j].aSubmodels.Count();i++)
				{
					if (MdlCust.aBodygroups[j].aSubmodels[i].bLoaded&&MdlCust.aBodygroups[j].aSubmodels[i].iFlags&CUSTOMIZE_FL_DEFAULT)
					{
						aiApplicable.AddToTail(MdlCust.aBodygroups[j].aSubmodels[i].iGroupValue);
					}
				}
				if( aiApplicable.Count() > 1 )
					SetBodygroup(MdlCust.aBodygroups[j].iGroup,aiApplicable[random->RandomInt(0,aiApplicable.Count()-1)]);
			}
			m_bIsCustomized=true;
		}
		else
		{
			Warning("(CINSPlayer::Spawn) Player model loaded, but modelcustomization_t is not initialized\n");
		}
	}

	// deathz0rz ]
}

//=========================================================
//=========================================================
void CINSPlayer::InitialSpawn( void )
{
	BaseClass::InitialSpawn( );
	ChangeTeam( TEAM_UNASSIGNED );
}

//=========================================================
//=========================================================
void CINSPlayer::SpawnDead(void)
{
	SetMoveType(MOVETYPE_NONE);
	EnableControl(false);
	AddSolidFlags(FSOLID_NOT_SOLID);
	m_takedamage = DAMAGE_NO;
	AddEffects(EF_NODRAW);
}

//=========================================================
//=========================================================
void CINSPlayer::SpawnReinforcement( void )
{
	Spawn( );

	EnableTeamChange( );
}

//=========================================================
//=========================================================
bool CINSPlayer::BumpWeapon( CBaseCombatWeapon *pWeapon, bool bCheckVisible )
{
	CBasePlayer *pOwner = pWeapon->GetOwner( );

	// don't let weapons with owners or players in vechiles pick weapons up
	if (pOwner)
		return false;

	// don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( bCheckVisible && !pWeapon->FVisible( this, MASK_SOLID ) )
		return false;

	// if I already have it just take the ammo
	if( Weapon_OwnsThisType( pWeapon->GetWeaponID( ) ) )
	{
		bool bTakenAmmo = Weapon_EquipAmmoOnly( pWeapon );

		if( bTakenAmmo )
			UTIL_Remove( pWeapon );

		return true;
	}

	// try and swap it a weapon of the same type
	int iWeaponType = pWeapon->GetWeaponType( );

	if( iWeaponType == WEAPONTYPE_PRIMARY || iWeaponType == WEAPONTYPE_SECONDARY )
	{
		CBaseCombatWeapon *pActiveWeapon = GetActiveWeapon( );

		if( pActiveWeapon && pActiveWeapon->GetWeaponType( ) == pWeapon->GetWeaponType( ) )
		{
			if( !Weapon_CanDrop( pActiveWeapon ) )
				return false;

			Weapon_Drop( pActiveWeapon, true, true, NULL );
		}
	}

	// otherwise take the weapon
	if( !Weapon_CanUse( pWeapon ) )
		return false;

	pWeapon->AddSolidFlags( FSOLID_NOT_SOLID );
	pWeapon->AddEffects( EF_NODRAW );

	Weapon_Equip( pWeapon );

	// try and switch to the weapon if its the "best"
	if( !m_bDisableAutoSwitch )
		SwitchToNextBestWeapon( NULL );

	return true;
}

//=========================================================
//=========================================================
bool CINSPlayer::Weapon_CanUse(CBaseCombatWeapon *pWeapon)
{
	int iWeaponSlot = Weapon_GetEmptySlot(pWeapon->GetWeaponType());

	if(iWeaponSlot == WEAPONSLOT_INVALID)
		return false;

	return (GetWeapon(iWeaponSlot) == NULL);
}

//=========================================================
//=========================================================
int CINSPlayer::Weapon_GetEmptySlot( int iWeaponType ) const
{
	switch( iWeaponType )
	{
		case WEAPONTYPE_PRIMARY:
		{
			return WEAPONSLOT_PRIMARY;
		}

		case WEAPONTYPE_SECONDARY:
		{
			return WEAPONSLOT_SECONDARY;
		}

		case WEAPONTYPE_MELEE:
		{
			return WEAPONSLOT_MELEE;
		}

		case WEAPONTYPE_EQUIPMENT:
		{
			for( int i = WEAPONSLOT_EQUIPMENT1; i <= WEAPONSLOT_EQUIPMENT2; i++ )
			{
				if( !GetWeapon( i ) )
					return i;
			}
		}
	}

	return WEAPONSLOT_INVALID;
}

//=========================================================
//=========================================================
bool CINSPlayer::Weapon_CanDrop( CBaseCombatWeapon *pWeapon ) const
{
	if( !BaseClass::Weapon_CanDrop( pWeapon ) )
		return false;

	if( GetPlayerFlags( ) & FL_PLAYER_BIPOD )
		return false;

	CPlayTeam *pTeam = GetPlayTeam( );

	if( pTeam && pWeapon->GetWeaponID( ) == pTeam->GetTeamLookup( )->GetDefaultWeapon( ) )
		return false;

	return true;
}

//=========================================================
//=========================================================
void CINSPlayer::Weapon_Equip( CBaseCombatWeapon *pWeapon )
{
	int iWeaponSlot = Weapon_GetEmptySlot( pWeapon->GetWeaponType( ) );

	// set the weapon to its slot
	m_hMyWeapons.Set( iWeaponSlot, pWeapon );
	ToINSWeapon( pWeapon )->SetPlayerSlot( iWeaponSlot );

	// tell the weapon its been equiped
	pWeapon->Equip( this );

	// update weight stuff
	m_flWeight += pWeapon->GetWeight( );
	UpdateWeightFactor( );

	// pass the lighting origin over to the weapon if we have one
	pWeapon->SetLightingOriginRelative( GetLightingOriginRelative( ) );
}

//=========================================================
//=========================================================
bool CINSPlayer::Weapon_EquipAmmoOnly(CBaseCombatWeapon *pWeapon)
{
	//return pWeapon->GiveAmmo(this, true);
	return false;
}

//=========================================================
//=========================================================
void CINSPlayer::RemovedWeapon(CBaseCombatWeapon *pWeapon)
{
	m_flWeight -= pWeapon->GetWeight();

	UpdateWeightFactor();
}

//=========================================================
//=========================================================
void CINSPlayer::RemoveAllItems( void )
{
	BaseClass::RemoveAllItems( );

	m_iBandadgeCount = 0;
}

//=========================================================
//=========================================================
void CINSPlayer::RemoveAllWeapons(void)
{
	BaseClass::RemoveAllWeapons();

	m_flWeight = 0;
	UpdateWeightFactor();
}

//=========================================================
//=========================================================
void CINSPlayer::CreateRagdollEntity( const Vector &vecForce )
{
#ifdef TESTING

	extern ConVar noragdolls;

	if( noragdolls.GetBool( ) )
		return;

#endif

	if( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// if we already have a ragdoll, don't make another one
	CINSRagdoll *pRagdoll = dynamic_cast< CINSRagdoll* >( m_hRagdoll.Get( ) );

	if( !pRagdoll )
		pRagdoll = dynamic_cast< CINSRagdoll* >( CreateEntityByName( "ins_ragdoll" ) );

	if( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin( );
		pRagdoll->m_vecRagdollVelocity = vec3_origin;
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = vecForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin( ) );
		pRagdoll->CopyAnimationDataFrom( this );

		pRagdoll->Spawn( );
	}

	m_hRagdoll = pRagdoll;
}

//=========================================================
//=========================================================
void CINSPlayer::CommitSuicide( void )
{
	CommitSuicide( false );
}

//=========================================================
//=========================================================
void CINSPlayer::CommitSuicide( bool bForce )
{
	CommitSuicide( PDEATHTYPE_SELF, bForce );
}

//=========================================================
//=========================================================
void CINSPlayer::CommitSuicide( int iType, bool bForce )
{
	if( !IsAlive( ) )
		return;

	// prevent suiciding too often
	if( !bForce )
	{
		if( m_fNextSuicideTime > gpGlobals->curtime )
			return;

		// don't let them suicide for 5 seconds after suiciding
		m_fNextSuicideTime = gpGlobals->curtime + 5.0f;  
	}

	// have the player kill themself
	int iDamageFlags = DMG_GENERIC | DMG_REMOVENORAGDOLL | DMG_PREVENT_PHYSICS_FORCE;

	if( iType == PDEATHTYPE_SOULSTOLEN )
		iDamageFlags |= ( DMG_DISSOLVE | DMG_DEVELOPER );
	else if( iType == PDEATHTYPE_TELEFRAG )
		iDamageFlags |= DMG_TELEFRAG;

	CTakeDamageInfo KilledDamage( this, this, 0, iDamageFlags );

	// don't need to set forced since ontakedamage isn't called
	Event_Killed( KilledDamage );
	Event_Dying(KilledDamage);
}

//=========================================================
//=========================================================
CTeam *CINSPlayer::GetTeam( void ) const
{
	Assert( IsValidTeam( m_iTeamID ) );
	return GetGlobalTeam( m_iTeamID );
}

//=========================================================
//=========================================================
bool CINSPlayer::OnPlayTeam(void) const
{
	return IsPlayTeam(GetTeamID());
}

//=========================================================
//=========================================================
void CINSPlayer::ChangeTeam( int iTeamID )
{
	Assert( IsValidTeam( iTeamID ) );

	// ensure its not the old team again
	int iOldTeam = GetTeamID( );

	if( iTeamID == iOldTeam )
		return;

	// remove from current obj
	ExitObjective( );

	// remove from current team
	RemoveFromTeam( );

	// add him to the new team
	CTeam *pNewTeam = GetGlobalTeam( iTeamID );

	if( !pNewTeam )
		return;

	pNewTeam->AddPlayer( this );
	m_iTeamID = iTeamID;

	// spawn right away if spectator and in a viewpoint
	if( iTeamID == TEAM_SPECTATOR && InViewpoint( ) )
		Spawn( );

	// send it off now!
	g_pPlayerResource->UpdatePlayerTeamID(entindex(), m_iTeamID);

	// send the message
	if( !IsBot( ) && iOldTeam != INVALID_TEAM )
	{
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "player_team", true );

		if( !pEvent )
			return;

		pEvent->SetInt( "userid", GetUserID( ) );
		pEvent->SetInt( "team", iTeamID );
		pEvent->SetInt( "oldteam", iOldTeam );
		gameeventmanager->FireEvent( pEvent );
	}
}

//=========================================================
//=========================================================
void CINSPlayer::RemoveFromTeam( void )
{
	if( GetTeamID( ) == INVALID_TEAM )
		return;

	// remove from the old team
	CTeam *pOldTeam = GetTeam( );
	Assert( pOldTeam );

	if( pOldTeam )
		pOldTeam->RemovePlayer( this );

	// set new team
	m_iTeamID = INVALID_TEAM;

	// objectives - update!
	CINSObjective::UpdateAllRequiredPlayers( );
}

//=========================================================
//=========================================================
bool CINSPlayer::ChangeSquad( const SquadData_t &SquadData, bool bWhenDie )
{
	CPlayTeam *pTeam = GetPlayTeam( );
	Assert( pTeam );

	if( !pTeam )
		return false;

	if( bWhenDie )
	{
		// set future squad data
		m_NextSquadData = SquadData;

		// echo to the player
		const CINSSquad *pSquad = pTeam->GetSquad( SquadData.GetSquadID( ) );
		Assert( pSquad );

		if( pSquad )
		{
			CPlayerClass *pClass = pSquad->GetClass( SquadData.GetSlotID( ) );
			Assert( pClass );

			if( pClass )
			{
				char szRespawnMsg[ 128 ];
				Q_snprintf( szRespawnMsg, sizeof( szRespawnMsg ), "You will respawn in %s as a %s", pSquad->GetName( ), pClass->GetName( ) );

				ClientPrint( this, HUD_PRINTTALK, szRespawnMsg );
			}
		}

		return true;
	}
	else
	{
		return ChangeSquad( SquadData );
	}
}

//=========================================================
//=========================================================
bool CINSPlayer::ChangeSquad( const SquadData_t &SquadData )
{
	CPlayTeam *pTeam = GetPlayTeam( );
	Assert( pTeam );

	if( !pTeam )
		return true;

	// remove from current slot
	RemoveSquad( );

	// add to new squad
	CINSSquad *pNewSquad = pTeam->GetSquad( SquadData );

	pNewSquad->AddPlayer( this, SquadData );

	// send the message
	IGameEvent *pEvent = gameeventmanager->CreateEvent( "player_squad", true );

	if( pEvent )
	{
		pEvent->SetInt( "userid", entindex( ) );
		pEvent->SetInt( "squad", SquadData.GetEncodedData( ) );
		pEvent->SetInt( "oldsquad", m_SquadData.GetEncodedData( ) );
		gameeventmanager->FireEvent( pEvent );
	}

	// set data
	m_SquadData = SquadData;

	// mark them as having had their first squad change
	m_bFirstSquadChange = false;

	// don't let him do it again (until next time he dies)
	m_INSLocal.m_bAllowTeamChange = false;

	// update rank
	UpdateRank( );

	return true;
}

//=========================================================
//=========================================================
bool CINSPlayer::RemoveSquad( void )
{
	CINSSquad *pCurrentSquad = GetSquad( );

	if( pCurrentSquad )
		pCurrentSquad->RemovePlayer( this );

	return true;
}

//=========================================================
//=========================================================
void CINSPlayer::ResetSquad(void)
{
	m_SquadData.Reset();
	m_bCommander = false;
}

//=========================================================
//=========================================================
bool CINSPlayer::IsValidSquad(void) const
{
	return m_SquadData.IsValid();
}

//=========================================================
//=========================================================
EncodedSquadData_t CINSPlayer::GetEncodedSquadData(void)
{
	return m_SquadData.GetEncodedData( );
}

//=========================================================
//=========================================================
CINSSquad *CINSPlayer::GetSquad( void ) const
{
	CPlayTeam *pTeam = GetPlayTeam( );
	return ( pTeam ? pTeam->GetSquad( GetSquadID( ) ) : NULL );
}

//=========================================================
//=========================================================
void CINSPlayer::AttemptSquadUpdate( void )
{
	// don't bother when invalid
	if( !m_NextSquadData.IsValid( ) )
		return;

	char szMessageToSend[ 128 ];

	// attempt to change squad
	EncodedSquadData_t EncodedSquadData = m_NextSquadData.GetEncodedData( );

	if( INSRules( )->SetupPlayerSquad( this, true, &EncodedSquadData, false ) )
	{
		CINSSquad *pSquad = GetSquad( );
		CPlayerClass *pClass = GetClass( );

		if( pSquad && pClass )
			Q_snprintf( szMessageToSend, sizeof( szMessageToSend ), "You are now in %s as a %s", pSquad->GetName( ), pClass->GetName( ) );
	}
	else
	{
		Q_strncpy( szMessageToSend, "You were unable to change your squad", sizeof( szMessageToSend ) );
	}

	// reset
	m_NextSquadData.Reset( );

	// send the message
	ClientPrint( this, HUD_PRINTTALK, szMessageToSend );
}

//=========================================================
//=========================================================
void CINSPlayer::UpdateRank( void )
{
	/*if( IsCommander( ) )
	{
		m_iRank = RANK_COMMANDER;
	}
	else
	{*/
		m_iRank = RANK_PRIVATE;

		for( int i = 0; i < RANK_COUNT; i++ )
		{
			if( m_iMorale > m_RankBoundaries[ i ] )
				continue;

			m_iRank = i;
			break;
		}

		if (OnPlayTeam())
			GetPlayTeam()->UpdatePlayerRanks();
	//}
}

//=========================================================
//=========================================================
void CINSPlayer::ResetHelp( void )
{
	m_bNeedsHelp = false;
	m_flHelpTimeout = 0.0f;
}

//=========================================================
//=========================================================
#define HELP_TIMEOUT 3.0f

void CINSPlayer::SetNeedsHelp( void )
{
	if( m_bNeedsHelp && m_flHelpTimeout >= gpGlobals->curtime )
		return;

	// don't send help again for a while
	m_bNeedsHelp = true;
	m_flHelpTimeout = gpGlobals->curtime + HELP_TIMEOUT;

	// send voice
	KeyValues *pData = new KeyValues( "voicedata" );

	const char *pszArea = UTIL_FindPlaceName( GetAbsOrigin( ) );

	if( pszArea )
		pData->SetString( "area", pszArea );

	UTIL_SendVoice( VOICEGRP_PLAYER_HELP, this, pData );
}

//=========================================================
//=========================================================
void CINSPlayer::ResetActions( void )
{
	memset( &m_flActionThreshold, 0.0f, sizeof( m_flActionThreshold ) );
}

//=========================================================
//=========================================================
#define PLAYER_ACTION_CHECKRADIUS 144

void CINSPlayer::SendAction( int iType )
{
	static float flActionLengths[ PACTION_COUNT ] = {
		10.0f, // PACTION_RELOADING 
		15.0f, // PACTION_HIT
		10.0f, // PACTION_OUTOFAMMO
		0.0f,  // PACTION_FRAGOUT
		8.0f   // PACTION_MANDOWN
	};

	// ensure valid type
	if( iType < 0 || iType >= PACTION_COUNT )
	{
		Assert( false );
		return;
	}

	// ensure they're not spamming it
	if( m_flActionThreshold[ iType ] >= gpGlobals->curtime )
		return;

	// ensure team-mates are around to hear it
	CBaseEntity *pEntity = NULL;
	bool bTeamAround = false;

	for( CEntitySphereQuery sphere( GetAbsOrigin( ), PLAYER_ACTION_CHECKRADIUS );
		( !bTeamAround && ( pEntity = sphere.GetCurrentEntity( ) ) != NULL );
		sphere.NextEntity( ) )
	{
		if( !pEntity->IsPlayer( ) || pEntity == this  )
			continue;

		CINSPlayer *pPlayer = ToINSPlayer( pEntity );

		// ... must be on the same team
		if( pPlayer->GetTeamID( ) != m_iTeamID )
			continue;

		// ... must be alive
		if( pPlayer->IsAlive( ) && !pPlayer->IsObserver( ) )
			continue;

		bTeamAround = true;

		break;
	}

	// don't send if there aren't any team-mates around to hear it
	if( !bTeamAround )
		return;

	// send out voice
	KeyValues *pData = new KeyValues( "voicedata" );
	pData->SetInt( "action", iType );

	UTIL_SendVoice( VOICEGRP_PLAYER_ACTION, this, pData );

	// add lengths
	float flActionLength = flActionLengths[ iType ];

	if( flActionLength > 0.0f )
		m_flActionThreshold[ iType ] = gpGlobals->curtime + flActionLength;
}

//=========================================================
//=========================================================
void CINSPlayer::ResetStatus( void )
{
	int iType, iStatusID;
	iType = INVALID_PSTATUSTYPE;
	iStatusID = INVALID_PSTATUSID;

	for( int i = 0; i < PSTATUSTYPE_COUNT; i++ )
	{
		int iCurrentStatusID = ValidStatus( i );

		if( iCurrentStatusID != INVALID_PSTATUSID )
		{
			iType = i;
			iStatusID = iCurrentStatusID;
		}
	}

	if( iType != INVALID_PSTATUSTYPE )
		SetStatus( iType, iStatusID );
}

//=========================================================
//=========================================================
void CINSPlayer::ResetStatus( int iType )
{
	// don't bother reseting when the current status type
	// is greater than the current type
	if( m_iStatusType > iType )
		return;

	// find a new one
	ResetStatus( );
}

//=========================================================
//=========================================================
void CINSPlayer::UpdateStatus( int iType, int iID )
{
	// ensure of a higher order
	if( iType < m_iStatusType )
		return;

	// find a status ID
	if( iID == INVALID_PSTATUSID )
	{
		iID = ValidStatus( iType );

		if( iID == INVALID_PSTATUSID )
		{
			Assert( false );
			return;
		}
	}

	// set the status
	SetStatus( iType, iID );
}

//=========================================================
//=========================================================
void CINSPlayer::SetStatus( int iType, int iID )
{
	// store data
	m_iStatusType = iType;
	m_iStatusID = iID;

	// send update when running around and the round isn't cold
	if( OnPlayTeam( ) && !INSRules( )->IsRoundCold( ) )
	{
		CReliablePlayerRecipientFilter Filter( this ); 

		UserMessageBegin( Filter, "PlayerStatus" );

			WRITE_BYTE( iType );
			WRITE_BYTE( iID );

		MessageEnd( );
	}
}

//=========================================================
//=========================================================
int CINSPlayer::ValidStatus( int iType ) const
{
	switch( iType )
	{
		case PSTATUSTYPE_ORDER_OBJ:
		{
			// PNOTE: 'ORDERTYPE_OBJ_NONE' is the default player status

			const CObjOrder *pObjOrder = GetObjOrders( );

			if( !pObjOrder )
				return ORDERTYPE_OBJ_NONE;

			return UTIL_CaculateObjType( this, pObjOrder->Objective( ) );
		}

		case PSTATUSTYPE_ORDER_UNIT:
		{
			const CUnitOrder *pUnitOrder = GetUnitOrders( );

			if( !pUnitOrder )
				return INVALID_PSTATUSID;

			return pUnitOrder->OrderType( );
		}

		case PSTATUSTYPE_ORDER_PLAYER:
		{
			if( m_iPlayerOrderID == INVALID_PORDER )
				return INVALID_PSTATUSID;

			return m_iPlayerOrderID;
		}

		case PSTATUSTYPE_OBJECTIVE:
		{
			CINSObjective *pObjective = GetCurrentObjective( );

			if( !pObjective )
				return INVALID_PSTATUSID;

			return ( m_iTeamID == pObjective->GetCapturedTeam( ) ) ? PSTATUS_OBJ_DEFENDING : PSTATUS_OBJ_CAPTURING;
		}
	}

	return INVALID_PSTATUSID;
}

//=========================================================
//=========================================================
void CINSPlayer::LeftCurrentObj(void)
{
	m_flObjExitTime = gpGlobals->curtime + OBJ_OUTSIDECAPTIME;
}

//=========================================================
//=========================================================
bool CINSPlayer::InObjective( void ) const
{
	return ( m_pCurrentObjective != NULL );
}

//=========================================================
//=========================================================
void CINSPlayer::SetCurrentObjective( CINSObjective *pObjective )
{
	Assert( pObjective );
	m_pCurrentObjective = pObjective;

	UpdateStatus( PSTATUSTYPE_OBJECTIVE, INVALID_PSTATUSID );
}

//=========================================================
//=========================================================
void CINSPlayer::ResetCurrentObj( void )
{
	m_pCurrentObjective = NULL;
	ResetObjExitTime( );

	ResetStatus( PSTATUSTYPE_OBJECTIVE );
}

//=========================================================
//=========================================================
void CINSPlayer::ResetObjExitTime( void )
{
	m_flObjExitTime = 0.0f;
}

//=========================================================
//=========================================================
void CINSPlayer::ExitObjective(void)
{
	if( !m_pCurrentObjective )
		return;

	m_pCurrentObjective->PlayerExit( this );

	ResetCurrentObj( );
}

//=========================================================
//=========================================================
void CINSPlayer::CustomiseLayout( int iLayoutType, int iID )
{
	/*if( iLayoutType < 0 || iLayoutType > LAYOUTCUSTOM_COUNT )
		return;

	CPlayerClass *pPlayerClass = GetClass( );

	if( !pPlayerClass )
		return;

	CPlayerInventoryBlueprint &Blueprint = pPlayerClass->GetBlueprint( );

	if(iLayoutType == LAYOUTCUSTOM_PRIMARYWEAPON || iLayoutType == LAYOUTCUSTOM_SECONDARYWEAPON)
	{
		CUtlVector<CPlayerInventoryItem*> &Weapons = pItemGroup->m_Weapons[iLayoutType];

		if(!Weapons.IsValidIndex(iID))
			return;

		m_LayoutCustomisation[iLayoutType] = iID;
	}*/
}

//=========================================================
//=========================================================
void CINSPlayer::ClearLayoutCustomisation(void)
{
	for( int i = 0; i < PLAYER_CLASSPREFERENCE_COUNT; i++ )
		m_LayoutCustomisation[ i ] = 0;
}

//=========================================================
//=========================================================
int CINSPlayer::GetLayoutCustomisation(int iLayoutType) const
{
	return m_LayoutCustomisation[iLayoutType];
}

//=========================================================
//=========================================================
void CINSPlayer::ClearClassPreference( void )
{
	for( int i = 0; i < PLAYER_CLASSPREFERENCE_COUNT; i++ )
		m_ClassPreferences[ i ] = INVALID_CLASS;
}

//=========================================================
//=========================================================
int CINSPlayer::GetClassPreference( int iID ) const
{
	return m_ClassPreferences[ iID ];
}

//=========================================================
//=========================================================
void CINSPlayer::ResetViewTransition( void )
{
	m_flViewTransitionLength = 0.0f;
	m_flViewTransitionEnd = 0.0f;
	m_iViewTransitionFrom = 0;
	m_iViewTransitionTarget = 0;
	m_iViewOffsetBipod = 0;
}

//=========================================================
//=========================================================
void CINSPlayer::SetupEndgameCamera(CBaseEntity *pWinningObject)
{
	if(!pWinningObject)
		return;

	SetObserverMode(OBS_MODE_CHASE);
	SetObserverTarget(pWinningObject);
}

//=========================================================
//=========================================================
#define UPPER_WEIGHT_LIMIT 15

void CINSPlayer::UpdateWeightFactor(void)
{
	CTeamLookup *pTeam = GetTeamLookup();

	float flValue = 1.0f;

	if(pTeam)
		flValue = (0.6f + (0.4f * (1.0f - clamp(((m_flWeight / UPPER_WEIGHT_LIMIT) * (1 / pTeam->GetWeightOffset())), 0.0f, 1.0f))));

	m_INSLocal.m_flWeightFactor = flValue;
}

//=========================================================
//=========================================================
void CINSPlayer::ResetDamageDecay(void)
{
	m_iDamageDecay = 0;
	m_flDamageDecayThreshold = 0.0f;
}

//=========================================================
//=========================================================
void CINSPlayer::ResetBunny(void)
{
	m_flStartBunnyHopTime = m_flBunnyHopLength = 0.0f;
}

//=========================================================
//=========================================================
CBaseCombatWeapon *CINSPlayer::CreateWeapon( int iWeaponID )
{
	const char *pszName = WeaponIDToName( iWeaponID );

	// ensure it's a valid name
	if( !pszName )
		return NULL;

	// if they already own this type then don't create one
	if(Weapon_OwnsThisType( iWeaponID ) )
		return NULL;

	// create and asetup
	CBaseEntity *pEntity = CreateEntityByName( pszName );

	if( pEntity == NULL )
	{
		Msg( "CINSPlayer::CreateWeapon, Spawned a NULL Entity!\n" );
		return NULL;
	}

	// ... set origin and spawn
	CBaseCombatWeapon *pWeaponEntity = ( CBaseCombatWeapon* )CreateEntityByName( pszName );

	pWeaponEntity->SetLocalOrigin( GetLocalOrigin( ) );
	DispatchSpawn( pWeaponEntity );

	// ... hit it against the player so that they own it
	if( !BumpWeapon( pWeaponEntity, false ) )
	{
		UTIL_Remove( pWeaponEntity );
		return NULL;
	}

	return pWeaponEntity;
}

//=========================================================
//=========================================================
bool CINSPlayer::AddWeapon( int iWeaponID, int iClipCount, int iAmmoCount )
{
	CBaseCombatWeapon *pWeapon = CreateWeapon( iWeaponID );

	if( !pWeapon )
		return false;

	if( iClipCount > 0 )
		pWeapon->GiveClip( iClipCount );

	if( iAmmoCount > 0 )
		pWeapon->GiveAmmo( iAmmoCount );

	return true;
}

//=========================================================
//=========================================================
bool CINSPlayer::AddEquipment( int iItemID, int iCount )
{
	// PNOTE: remember that the storage is handled by the derivative not
	// the base, so this is actually quite clean

	// handle bandage
	if( iItemID == ITEM_BANDAGE )
	{
		// TODO: different max amount for medics
		if( m_iBandadgeCount >= PLAYER_BANDAGE_MAX )
			return false;

		m_iBandadgeCount++;

		return true;
	}

	// translate item to weapon
	int iWeaponID = WEAPON_INVALID;

	switch( iItemID )
	{
		case ITEM_M67:
		{
			iWeaponID = WEAPON_M67;
			break;
		}

		case ITEM_M18:
		{
			iWeaponID = WEAPON_M18;
			break;
		}

		case ITEM_RGD5:
		{
			iWeaponID = WEAPON_RGD5;
			break;
		}

		case ITEM_C4:
		{
			iWeaponID = WEAPON_C4;
			break;
		}
	}

	if( iWeaponID == WEAPON_INVALID )
		return false;

	// try to see if we already have one
	CBaseCombatWeapon *pWeapon = FindEquipment( iWeaponID );

	// if not, create it
	if( !pWeapon )
	{
		pWeapon = CreateWeapon( iWeaponID );

		if( !pWeapon )
		{
			AssertMsg( false, "CINSPlayer::AddEquipment, need more weaponslots for equipment!!\n" );
			return false;
		}
	}

	pWeapon->GiveAmmo( iCount );

	return true;
}

//=========================================================
//=========================================================
CBaseCombatWeapon *CINSPlayer::FindEquipment( int iWeaponID )
{
	for( int i = WEAPONSLOT_EQUIPMENT1; i <= WEAPONSLOT_EQUIPMENT2; i++ )
	{
		CBaseCombatWeapon *pWeapon = GetWeapon( i );

		if( pWeapon && pWeapon->GetWeaponID( ) == iWeaponID )
			return pWeapon;
	}

	return NULL;
}

//=========================================================
//=========================================================
void CINSPlayer::UpdateMorale( void )
{
	m_iMorale = INSRules( )->PlayerCalculateMorale( m_MaskedStats );

	// update rank
	UpdateRank( );
}

//=========================================================
//=========================================================
void CINSPlayer::StatsLogin( int iMemberID )
{
	m_iStatsMemberID = iMemberID;
}

//=========================================================
//=========================================================
void CINSPlayer::StatsLogout( void )
{
	int iTimePlayed = RoundFloatToInt( ( gpGlobals->curtime - m_flTimeJoined ) / 60.0f );
	IncrementStat( PLAYERSTATS_MINUTES, iTimePlayed );
}

//=========================================================
//=========================================================
void CINSPlayer::ResetStats( void )
{
	m_Stats.Reset( );
	m_MaskedStats.Reset( );
}

//=========================================================
//=========================================================
void CINSPlayer::UnmaskStats( void )
{
	// copy over the live stats and then stamp it
	m_MaskedStats = m_Stats;

	// update morale
	UpdateMorale( );

	// update playerstate
	pl.frags = m_Stats.m_iKills;
	pl.deaths = m_Stats.m_iDeaths;
}

//=========================================================
//=========================================================
void CINSPlayer::IncrementStat( int iType, int iIncrement )
{
	// update stats
	m_AggregateStats.Increment( iType, iIncrement );
	m_Stats.Increment( iType, iIncrement );

	// don't update kills or deaths for masked stats
	if( iType != PLAYERSTATS_KILLPTS && iType != PLAYERSTATS_KILLS && iType != PLAYERSTATS_DEATHS )
		m_MaskedStats.Increment( iType, iIncrement );

	// update morale when gamepts or fkills
	if( iType == PLAYERSTATS_GAMEPTS || iType == PLAYERSTATS_FKILLS )
		UpdateMorale( );
}

//=========================================================
//=========================================================
void CINSPlayer::IncrementWeaponStat( int iWeaponID, int iType, int iIncrement )
{
	CWeaponStats &WeaponStats = m_AggregateStats.GetWeaponStats( iWeaponID );
	WeaponStats.Increment( iType, iIncrement );
}

//=========================================================
//=========================================================
const CGamePlayerStats &CINSPlayer::GetUpdateStats( void ) const
{
	return ( INSRules( )->PlayerLiveScores( ) ? m_Stats : m_MaskedStats );
}

//=========================================================
//=========================================================
void CINSPlayer::UpdateHealth( int iNewHealth )
{
	m_iHealth = clamp( iNewHealth, 0, GetMaxHealth( ) );
}

//=========================================================
//=========================================================
bool CINSPlayer::CanSendFFMessage(void) const
{
	return ( m_flLastFFAttack <= gpGlobals->curtime );
}

//=========================================================
//=========================================================
#define PLAYER_FF_TIME_GAP 0.5f

void CINSPlayer::ExtendFFMessage(void)
{
	m_flLastFFAttack = gpGlobals->curtime + PLAYER_FF_TIME_GAP;
}

//=========================================================
//=========================================================
void CINSPlayer::ViewpointAdd( void )
{
	INSRules( )->PlayerViewpointAdd( this );

	m_bSpawnedViewpoint = true;
}

//=========================================================
//=========================================================
void CINSPlayer::ViewpointRemove( void )
{
	if( !InViewpoint( ) )
		return;

	INSRules( )->PlayerViewpointRemove( this );

	m_bSpawnedViewpoint = false;
}

//=========================================================
//=========================================================
#define DAMAGETAKEN_HARDLIMIT 8

void CINSPlayer::AddDamageTaken( CINSPlayer *pAttacker, int iDamageType )
{
	Assert( pAttacker );

	// don't let it grow too large
	if( m_DamageTaken.Count( ) >= DAMAGETAKEN_HARDLIMIT )
		return;

	// add to tail
	int iIndex = m_DamageTaken.AddToTail( );

	if( !m_DamageTaken.IsValidIndex( iIndex ) )
		return;

	DamageTaken_t &DamageTaken = m_DamageTaken[ iIndex ];

	DamageTaken.m_iDamageType = iDamageType;
	DamageTaken.m_hPlayer = pAttacker;
	DamageTaken.m_iTeamID = pAttacker->GetTeamID( );
}

//=========================================================
//=========================================================
int CINSPlayer::GetDamageInfoType( int iDamage )
{
	if( iDamage >= 80 )
		return DAMAGEINFO_SERIOUS;
	else if( iDamage >= 35 )
		return DAMAGEINFO_MODERATE;

	return DAMAGEINFO_LIGHT;
}

//=========================================================
//=========================================================
const DamageTaken_t &CINSPlayer::GetDamageTaken( int iID )
{
	return m_DamageTaken[ iID ];
}

//=========================================================
//=========================================================
int CINSPlayer::GetDamageTakenCount( void )
{
	return m_DamageTaken.Count( );
}

//=========================================================
//=========================================================
void CINSPlayer::SendDeathInfo( int iType, CBaseEntity *pScorer, int iDistance, int iInflicatorType, int iInflicatorID, int iDamageType )
{
	// don't do anything when a bot
	if( IsBot( ) )
		return;

	// save the last killer
	m_LastKiller = pScorer;

	// now send it off
	int iScorerID = 0;
	
	if( pScorer )
		iScorerID = pScorer->entindex( );

	// ... build the message
	CSingleUserRecipientFilter filter( this );
	filter.MakeReliable( );

	UserMessageBegin( filter, "DeathInfo" );

		WRITE_BYTE( iType );
		WRITE_BYTE( iScorerID );

		if( iType == PDEATHTYPE_KIA || iType == PDEATHTYPE_FF )
		{
			CRunningMode *pRunning = INSRules( )->RunningMode( );

			if( pRunning && pRunning->IsDeathInfoFull( ) )
			{
				WRITE_WORD( iDistance );
				WRITE_BYTE( iInflicatorType + 1 );
				WRITE_BYTE( iInflicatorID + 1 );
			}
			else
			{
				WRITE_WORD( iDamageType );
			}
		}

	MessageEnd( );
}

//=========================================================
//=========================================================
/*#define MAX_USEBYTES 192
#define BYTE_SIZE sizeof(char)

void CINSPlayer::SetupDamageInfo(int iType, int iCurrentElement)
{
	if(IsBot() || iType == DAMAGEINFO_MSG_KILLER)
		return;

	CUtlVector<DamageInfo_t> &DamageInfo = (iType == DAMAGEINFO_MSG_GIVEN) ? m_DamageGiven : m_DamageTaken;
	int DamageInfoCount = DamageInfo.Count();

	if(DamageInfoCount == 0)
		return;

	// create filter
	CSingleUserRecipientFilter filter(this);
	filter.MakeReliable();

	// setup the message
	int iByteCount = 0;

	UserMessageBegin(filter, "DeathInfo");
		WRITE_BYTE(iType);

		for(int i = iCurrentElement; i < DamageInfoCount; i++)
		{
			DamageInfo_t &CurrentInfo = DamageInfo[i];
			int iUseBytes = 0;

			iUseBytes += BYTE_SIZE * 5; // 2 type's and 3 damage type's
			
			CBaseEntity *pPlayer = CurrentInfo.m_Player;

			if(pPlayer)
				iUseBytes += BYTE_SIZE;
			else
				iUseBytes += Q_strlen(CurrentInfo.m_szName) + 1;

			if((iByteCount + iUseBytes) <= MAX_USEBYTES)
			{
				CBaseEntity *pPlayer = CurrentInfo.m_Player;

				if(pPlayer)
				{
					WRITE_BYTE(DAMAGEINFO_PLAYER_ACTIVE);
					WRITE_BYTE(pPlayer->entindex());
				}
				else
				{
					WRITE_BYTE(DAMAGEINFO_PLAYER_INACTIVE);
					WRITE_STRING(CurrentInfo.m_szName);
				}

				WRITE_BYTE(CurrentInfo.m_iDamageType[DAMAGEINFO_DAMAGE_LIGHT]);
				WRITE_BYTE(CurrentInfo.m_iDamageType[DAMAGEINFO_DAMAGE_MODERATE]);
				WRITE_BYTE(CurrentInfo.m_iDamageType[DAMAGEINFO_DAMAGE_SERIOUS]);

				iCurrentElement++;
				iByteCount += iUseBytes;
			}
			else
			{
				WRITE_BYTE(DAMAGEINFO_LASTELEMENT);
				break;
			}

			if(iCurrentElement == DamageInfoCount)
				WRITE_BYTE(DAMAGEINFO_LASTELEMENT);
		}

	MessageEnd();

	if(iCurrentElement != DamageInfoCount)
		SetupDamageInfo(iType, iCurrentElement);
}*/

//=========================================================
//=========================================================
void CINSPlayer::SetClippingEntity(CSpawnProtection *pEntity)
{
#ifdef _DEBUG
	Msg("Clipping an Entity\n");
#endif

	m_hClippingEntity = pEntity;
	Assert(m_hClippingEntity != NULL);
}

//=========================================================
//=========================================================
void CINSPlayer::RemoveClippingEntity(void)
{
#ifdef _DEBUG
	Msg("Removing an Entity Clip\n");
#endif

	// HACKHACK
	m_hClippingEntity = NULL;
}


//=========================================================
//=========================================================
void CINSPlayer::SetMantalingEntity( CMantleZone *pEntity )
{
#ifdef _DEBUG
	Msg("Mantle an Entity\n");
#endif

	m_hMantleEntity = pEntity;
	Assert( m_hMantleEntity != NULL );
}

//=========================================================
//=========================================================
void CINSPlayer::RemoveMantalingEntity( void )
{
#ifdef _DEBUG
	Msg("Removing an Entity Mantle\n");
#endif

	// HACKHACK
	m_hMantleEntity = NULL;
}

//=========================================================
//=========================================================
void CINSPlayer::ClearPain(void)
{
	SendPain( PAINTYPE_RESET );
}

//=========================================================
//=========================================================
void CINSPlayer::SendPain( int iType )
{
	CSingleUserRecipientFilter user( this );
	user.MakeReliable( );

	UserMessageBegin( user, "Pain" );

		WRITE_BYTE( iType + 1 );

	MessageEnd( );
}

//=========================================================
//=========================================================
void CINSPlayer::ConcussionEffect( bool bMinor )
{
	// make the ears ring
	CReliablePlayerRecipientFilter Filter( this );

	int effect = bMinor ? 
		random->RandomInt( 32, 34 ) : 
		random->RandomInt( 35, 37 );

	enginesound->SetPlayerDSP( Filter, effect, false );

	// send pain only if minor (damagebits get sent over otherwise)
	if( bMinor )
		SendPain( PAINTYPE_CMINOR );
}

//=========================================================
//=========================================================
void CINSPlayer::ResetConcussionEffect( void )
{
	CReliablePlayerRecipientFilter user( this );
	enginesound->SetPlayerDSP( user, 0, false );
}

//=========================================================
//=========================================================
void CINSPlayer::SetEntityPosition( CBaseEntity *pPoint )
{
	SetLocalOrigin( UTIL_SpawnPositionOffset( pPoint ) );
	SetAbsVelocity( vec3_origin );
	SetLocalAngles( pPoint->GetLocalAngles( ) );

	m_Local.m_vecPunchAngle = vec3_angle;
	m_Local.m_vecPunchAngleVel = vec3_angle;
	m_Local.m_vecRecoilPunchAngle = vec3_angle;
	m_Local.m_vecRecoilPunchAngleVel = vec3_angle;

	SnapEyeAngles( pPoint->GetLocalAngles( ) );
}

//=========================================================
//=========================================================
void CINSPlayer::FadeToBlack(float flLength)
{
	color32 Black = {0, 0, 0, 255};
	UTIL_ScreenFade(this, Black, flLength, 0.0f, FFADE_OUT | FFADE_STAYOUT);
}

//=========================================================
//=========================================================
void CINSPlayer::FadeOutBlack(float flLength)
{
	color32 Black = {0, 0, 0, 255};
	UTIL_ScreenFade(this, Black, flLength, 0.0f, FFADE_IN | FFADE_PURGE);
}

//=========================================================
//=========================================================
void CINSPlayer::SetCanShowDeathMenu( void )
{
	if( GetDeathMenuType( ) == DEATHINFOTYPE_NONE )
		return;

	m_bCanShowDeathMenu = true;
}

//=========================================================
//=========================================================
int CINSPlayer::GetHealthType(void) const
{
	if( m_iHealth >= GetMaxHealth( ) )
		return HEALTHTYPE_UNINJURED;
	if( m_iHealth >= 75 )
		return HEALTHTYPE_FINE;
	else if( m_iHealth >= 25 )
		return HEALTHTYPE_INJURED;

	return HEALTHTYPE_SERIOUS;
}

//=========================================================
//=========================================================
void CINSPlayer::EnterCurrentArea( CINSTouch *pArea )
{
	const char *pszName = pArea->GetTitle( );
	Assert( pszName );

	if( pszName )
		Q_strncpy( m_szLastArea, pArea->GetTitle( ), MAX_PLACE_NAME_LENGTH );
}

//=========================================================
//=========================================================
void CINSPlayer::LeaveCurrentArea( void )
{
	// TODO: maybe a stack of IINSArea's
	Q_strncpy( m_szLastArea, "", MAX_PLACE_NAME_LENGTH );
}

//=========================================================
//=========================================================
void CINSPlayer::ResetOrders( void )
{
	m_iPlayerOrderID = INVALID_PORDER;
}

//=========================================================
//=========================================================
bool CINSPlayer::HasObjOrders( void ) const
{
	const CObjOrder *pObjOrder = GetObjOrders( );
	return ( pObjOrder && pObjOrder->HasOrders( ) );
}

//=========================================================
//=========================================================
const CObjOrder *CINSPlayer::GetObjOrders( void ) const
{
	CINSSquad *pSquad = GetSquad( );
	return ( ( pSquad && pSquad->HasObjOrders( ) ) ? &pSquad->GetObjOrders( ) : NULL );
}

//=========================================================
//=========================================================
void CINSPlayer::AssignedObjOrders( int iID )
{
	UpdateStatus( PSTATUSTYPE_ORDER_OBJ, iID );
}

//=========================================================
//=========================================================
bool CINSPlayer::HasUnitOrders( void ) const
{
	const CUnitOrder *pUnitOrder = GetUnitOrders( );
	return ( pUnitOrder != NULL );
}

//=========================================================
//=========================================================
const CUnitOrder *CINSPlayer::GetUnitOrders( void ) const
{
	CINSSquad *pSquad = GetSquad( );
	return ( ( pSquad && pSquad->HasUnitOrders( ) ) ? &pSquad->GetUnitOrders( ) : NULL );
}

//=========================================================
//=========================================================
void CINSPlayer::AssignedUnitOrders( int iID )
{
	UpdateStatus( PSTATUSTYPE_ORDER_UNIT, iID );
}

//=========================================================
//=========================================================
void CINSPlayer::ResetPlayerOrders( void )
{
	m_iPlayerOrderID = INVALID_PORDER;
}

//=========================================================
//=========================================================
void CINSPlayer::AssignPlayerOrders( int iID )
{
	Assert( UTIL_ValidPlayerOrder( iID ) );
	m_iPlayerOrderID = iID;

	UpdateStatus( PSTATUSTYPE_ORDER_PLAYER, iID );
}

//=========================================================
//=========================================================
void CINSPlayer::PlayerOrderResponse( int iType )
{
	// ensure valid response
	if( !UTIL_ValidPlayerOrderResponse( iType ) )
		return;

	// commanders cannot reply to their own commands
	if( IsCommander( ) )
		return;

	// ensure we have a valid order
	if( m_iPlayerOrderID == INVALID_PORDER )
		return;

	// send the command over voice
	KeyValues *pData = new KeyValues( "voicedata" );
	pData->SetInt( "order", m_iPlayerOrderID );
	pData->SetInt( "response", iType );
	UTIL_SendVoice( VOICEGRP_ORDER_PLAYER_RESPONSES, this, pData );

	// cancel the player command if they've finished it or
	// they can't do it
	if( iType != PORESPONSE_DOING )
		m_iPlayerOrderID = INVALID_PORDER;
}

//=========================================================
//=========================================================
void CINSPlayer::DamageEffect( float flDamage, int iDamageType )
{
	if( iDamageType & DMG_BULLET || iDamageType & DMG_BUCKSHOT )
	{
		EmitSound( "Flesh.BulletImpact" );

		EmitSound( "Player.MinorPain" );
	}
	else if( iDamageType & DMG_SLASH )
	{
		SpawnBlood( EyePosition( ), g_vecAttackDir, BloodColor( ), flDamage );

		if( GetDamageInfoType( flDamage ) > DAMAGEINFO_LIGHT )
			EmitSound( "Player.MegaPain" );
		else
			EmitSound( "Player.MajorPain" );
	}
	else if( iDamageType & DMG_PLASMA )
	{
		EmitSound( "Player.PlasmaDamage" );
	}
}

//=========================================================
//=========================================================
void CINSPlayer::ExecuteRemoveCommon( void )
{
	// remove him from his current obj
	ExitObjective( );

	// remove him from his current team
	RemoveFromTeam( );
}

//=========================================================
//=========================================================
int CINSPlayer::GetDeathMenuType( void ) const
{
	return GetCmdValue( CMDREGISTER_DEATHINFO );
}

//=========================================================
//=========================================================
void CINSPlayer::NoteWeaponFired( void )
{
	BaseClass::NoteWeaponFired( );

	m_flHeadTurnThreshold = gpGlobals->curtime + PLAYER_HEADTIME_SHOT;

	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CINSPlayer::MassVGUIHide( void )
{
	CSingleUserRecipientFilter filter( this );
	filter.MakeReliable( );

	UserMessageBegin( filter, "VGUIHide" );
	MessageEnd( );
}

//=========================================================
//=========================================================
bool CINSPlayer::SendHints( void ) const
{
	return GetCmdValue( CMDREGISTER_HIDEHINTS ) ? false : true;
}

//=========================================================
//=========================================================
void CINSPlayer::SendStatNotice( int iAmount, const char* pszType )
{
}

//=========================================================
//=========================================================
void CINSPlayer::ClientSettingsChanged( void )
{
}

//=========================================================
//=========================================================
void CINSPlayer::GivePowerball( void )
{
	m_nRenderFX = kRenderFxGlowShell;

	// TODO: switch to powerball!
	// Weapon_Switch( )

	m_bHasPowerball = true;
}

//=========================================================
//=========================================================
void CINSPlayer::StripPowerball( void )
{
	m_nRenderFX = kRenderFxNone;

	m_bHasPowerball = false;
}

CON_COMMAND( drop, "Drop your Current Weapon" )
{
	CINSPlayer *pPlayer = ToINSPlayer( UTIL_GetCommandClient( ) );

	if( pPlayer )
		pPlayer->Weapon_Drop( pPlayer->GetActiveWeapon( ), false, false, NULL );
}

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS(ins_ragdoll, CINSRagdoll);

BEGIN_DATADESC(CINSRagdoll)

	DEFINE_THINKFUNC(SUB_Vanish),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST_NOBASE( CINSRagdoll, DT_INSRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) ),
END_SEND_TABLE()

//=========================================================
//=========================================================
CINSRagdoll::CINSRagdoll( )
{
	m_iVanishTicks = 0;
}

//=========================================================
//=========================================================
extern ConVar ragdollkeep;

void CINSRagdoll::Spawn( void )
{
	INSRules( )->RagdollSpawn( this );

	m_flStartTime = gpGlobals->curtime;
}

//=========================================================
//=========================================================
void CINSRagdoll::Vanish(void)
{
	SetThink(&CINSRagdoll::SUB_Vanish);
	SetNextThink(gpGlobals->curtime);

	m_flStartTime = FLT_MAX;
}

//=========================================================
//=========================================================

// NOTE: could move this into CBaseEntity, but nothing else needs it

#define MAX_VANISH_TICKS 10

#define	MIN_CORPSE_FADE_DIST		256.0
#define	MAX_CORPSE_FADE_DIST		1500.0

void CINSRagdoll::SUB_Vanish(void)
{
	if(m_iVanishTicks > MAX_VANISH_TICKS)
	{
		SetThink(&CINSRagdoll::SUB_Remove);
		SetNextThink(gpGlobals->curtime);

		return;
	}

	// think again next second
	SetNextThink(gpGlobals->curtime + 1.0f);
		
	CBasePlayer *pPlayer;

	//Get all players
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		//Get the next client
		if ( ( pPlayer = UTIL_PlayerByIndex( i ) ) != NULL )
		{
			Vector corpseDir = (GetAbsOrigin() - pPlayer->WorldSpaceCenter() );

			float flDistSqr = corpseDir.LengthSqr();

			//If the player is close enough, don't fade out
			if ( flDistSqr < (MIN_CORPSE_FADE_DIST*MIN_CORPSE_FADE_DIST) )
			{
				m_iVanishTicks++;
				return;
			}

			// If the player's far enough away, we don't care about looking at it
			if ( flDistSqr < (MAX_CORPSE_FADE_DIST*MAX_CORPSE_FADE_DIST) )
			{
				VectorNormalize( corpseDir );

				Vector	plForward;
				pPlayer->EyeVectors( &plForward );

				float dot = plForward.Dot( corpseDir );

				if ( dot > 0.0f )
				{
					m_iVanishTicks++;
					return;
				}
			}
		}
	}

	// if we're here, then we can vanish safely
	if(m_DroppedWeapon)
		UTIL_Remove(m_DroppedWeapon);

	UTIL_Remove(this);
}

CON_COMMAND_F(kill, "Commit suicide", FCVAR_CHEAT)
{
	CINSPlayer* pPlayer = ToINSPlayer(UTIL_GetCommandClient());
	if (!pPlayer)
		return;

	if (!pPlayer->IsDeveloper() || (pPlayer->IsDeveloper() && args.ArgC() == 1))
	{
		pPlayer->CommitSuicide();
		return;
	}

	if (args.ArgC() < 2)
		return;

	if (args.ArgC() == 2)
	{
		int iPlayerID = atoi(args[1]);
		CINSPlayer* pPlayer = ToINSPlayer(UTIL_PlayerByIndex(iPlayerID));
		if (pPlayer)
			pPlayer->CommitSuicide(PDEATHTYPE_SOULSTOLEN, true);
	}
	else
	{
		int iTeamID = atoi(args[2]);

		if (!IsPlayTeam(iTeamID))
			return;

		CPlayTeam* pTeam = GetGlobalPlayTeam(iTeamID);
		if (!pTeam)
			return;

		for (int i = 0; i < pTeam->GetNumPlayers(); i++)
		{
			CINSPlayer* pTeamPlayer = ToINSPlayer(pTeam->GetPlayer(i));
			if (pTeamPlayer)
				pTeamPlayer->CommitSuicide(PDEATHTYPE_SOULSTOLEN, true);
		}
	}
}