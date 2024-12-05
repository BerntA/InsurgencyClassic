//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "base_playeranimstate.h"
#include "tier0/vprof.h"
#include "animation.h"
#include "studio.h"
#include "apparent_velocity_helper.h"
#include "utldict.h"
#include "ins_playeranimstate.h"
#include "ins_player_shared.h"

#ifdef CLIENT_DLL

#include "bone_setup.h"
#include "interpolatedvar.h"

#endif

//=========================================================
//=========================================================
#ifdef TESTING

ConVar ignoreplayerseq( "sv_ignoreplayerseq", "0", FCVAR_CHEAT | FCVAR_REPLICATED );

#endif

//=========================================================
//=========================================================
#ifdef TESTING

static ConVar s_varAnimSpeedWalk( "anim_walk_speed", "40", FCVAR_REPLICATED );
static ConVar s_varAnimSpeedRun( "anim_run_speed", "140", FCVAR_REPLICATED );
static ConVar s_varAnimSpeedSprint( "anim_sprint_speed", "200", FCVAR_REPLICATED );
static ConVar s_varAnimSpeedCrouchWalk( "anim_crouch_walk_speed", "40", FCVAR_REPLICATED );
static ConVar s_varAnimSpeedCrouchRun( "anim_crouch_run_speed", "40", FCVAR_REPLICATED );
static ConVar s_varAnimSpeedCrawl( "anim_crawl_speed", "35", FCVAR_REPLICATED );
static ConVar s_varAnimSpeedJump( "anim_jump_speed", "35", FCVAR_REPLICATED );

#define ANIM_TOPSPEED_WALK			( s_varAnimSpeedWalk.GetFloat( ) )
#define ANIM_TOPSPEED_RUN			( s_varAnimSpeedRun.GetFloat( ) )
#define ANIM_TOPSPEED_SPRINT		( s_varAnimSpeedSprint.GetFloat( ) )
#define ANIM_TOPSPEED_CROUCH_WALK	( s_varAnimSpeedCrouchWalk.GetFloat( ) )
#define ANIM_TOPSPEED_CROUCH_RUN	( s_varAnimSpeedCrouchRun.GetFloat( ) )
#define ANIM_TOPSPEED_CRAWL			( s_varAnimSpeedCrawl.GetFloat( ) )
#define ANIM_TOPSPEED_JUMP			( s_varAnimSpeedJump.GetFloat( ) )

#else

#define ANIM_TOPSPEED_WALK			40
#define ANIM_TOPSPEED_RUN			140
#define ANIM_TOPSPEED_SPRINT		200
#define ANIM_TOPSPEED_CROUCH_WALK	40
#define ANIM_TOPSPEED_CROUCH_RUN	40
#define ANIM_TOPSPEED_CRAWL			35
#define ANIM_TOPSPEED_JUMP			35

#endif

//=========================================================
//=========================================================

// idle, pattack, sattack, reload
#define ANIMPREFIX_COUNT 4


typedef const char *AnimPrefixTable_t[ ANIMPREFIX_COUNT ];

AnimPrefixTable_t g_AnimPrefixTable[ ] = {
	{ "StandIdle_", "StandIdleFire_", "",  "StandReload_" },		// ACT_INS_STAND
	{ "StandAim_", "StandAimFire_", "",  "StandReload_" },			// ACT_INS_STAND_AIM
	{ "Walk_", "WalkFire_", "", "StandReload_" },					// ACT_INS_STAND_WALK
	{ "Run_", "RunFire_", "", "StandReload_", },					// ACT_INS_STAND_RUN
	{ "StandDeploy_", "StandDeployFire_", "", "StandReload_" },		// ACT_INS_STAND_DEPLOY
	{ "StandLeanR_", "StandLeanRFire_", "", "StandReload_" },		// ACT_INS_STAND_LEANR
	{ "StandLeanL_", "StandLeanLFire_", "", "StandReload_" },		// ACT_INS_STAND_LEANL
	{ "Sprint_", "", "", "", },										// ACT_INS_SPRINT
	{ "CrouchIdle_", "CrouchIdleFire_", "", "CrouchReload_" },		// ACT_INS_CROUCH
	{ "CrouchAim_", "CrouchAimFire_", "", "CrouchReload_" },		// ACT_INS_CROUCH_AIM
	{ "CrouchWalk_", "CrouchWalkFire_", "", "CrouchReload_" },		// ACT_INS_CROUCH_WALK
	{ "CrouchRun_", "CrouchRunFire_", "", "CrouchReload_" },		// ACT_INS_CROUCH_RUN
	{ "CrouchDeploy_", "CrouchDeployFire_", "", "CrouchReload_" },	// ACT_INS_CROUCH_DEPLOY
	{ "CrouchLeanR_", "CrouchLeanRFire_", "", "CrouchReload_" },	// ACT_INS_CROUCH_LEANR
	{ "CrouchLeanL_", "CrouchLeanLFire_", "", "CrouchReload_" },	// ACT_INS_CROUCH_LEANL
	{ "Prone_", "ProneFire_", "", "ProneReload_" },					// ACT_INS_PRONE
	{ "ProneDeploy_", "ProneFire_", "", "ProneReload_" },			// ACT_INS_PRONE_DEPLOY
	{ "Crawl_", "ProneFire_", "", "ProneReload_" },					// ACT_INS_CRAWL
	{ "Jump_", "", "", "" },										// ACT_INS_JUMP
	{ "ragdoll", "", "", "" }										// ACT_TRANS_STAND_CROUCH
};

int UTIL_ConvertActivityToBaseIndex( Activity act )
{
	if( act < ACT_INS_STAND || act > ACT_TRANS_PRONE_CROUCH )
	{
		Assert( false );
		return -1;
	}

	// all the trans are the same past ACT_TRANS_STAND_CROUCH
	if( act > ACT_TRANS_STAND_CROUCH )
		act = ACT_TRANS_STAND_CROUCH;

	return ( act - ACT_INS_STAND );
}

const char *UTIL_FireActionPrefix( Activity act, bool bSecondary = false )
{
	int iIndex = UTIL_ConvertActivityToBaseIndex( act );

    if( iIndex == -1 )
		return NULL;

	return g_AnimPrefixTable[ iIndex ][ bSecondary ? 2 : 1 ];
}

const char *UTIL_ReloadActionPrefix( Activity act )
{
	int iIndex;
	iIndex = UTIL_ConvertActivityToBaseIndex( act );
        
	if( iIndex == -1 )
		return NULL;

	return g_AnimPrefixTable[ iIndex ][ 3 ];
}

const char *UTIL_IdleActionPrefix( Activity act )
{
	int iIndex = UTIL_ConvertActivityToBaseIndex( act );

	if( iIndex == -1 )
		return NULL;   

	return g_AnimPrefixTable[ iIndex ][ 0 ];
}

//=========================================================
//=========================================================
#define GESTURESEQUENCE_LAYER		( AIMSEQUENCE_LAYER + NUM_AIMSEQUENCE_LAYERS )
#define TURNSEQUENCE_LAYER	        ( GESTURESEQUENCE_LAYER + 2 )
#define NUM_LAYERS_WANTED		    ( TURNSEQUENCE_LAYER + 1 )

//=========================================================
//=========================================================
class CINSPlayerAnimState : public CBasePlayerAnimState, public IINSPlayerAnimState
{ 
	DECLARE_CLASS( CINSPlayerAnimState, CBasePlayerAnimState );

public:
	CINSPlayerAnimState( );

	virtual void DoAnimationEvent( PlayerAnimEvent_e event );
	
    virtual int CalcAimLayerSequence( float *flCycle, float *flAimSequenceWeight, bool bForceIdle );
    virtual int CalcTurnLayerSequence( float *flCycle, float *flAimSequenceWeight, bool bForceIdle );
	virtual void ClearAnimationState( void );

	virtual float GetCurrentMaxGroundSpeed( void );
	virtual Activity CalcMainActivity( void );
	virtual void DebugShowAnimState( int iStartLine );
	virtual void ComputeSequences( CStudioHdr *pStudioHdr );
	virtual void ClearAnimationLayers( void );
	virtual void ClearAnimationLayers( bool bForce );

    void UpdateMainSequence( void );

	void InitSDK( CBaseAnimatingOverlay *pPlayer, IINSPlayerAnimStateHelper *pHelper, LegAnimType_t legAnimType, bool bUseAimSequences );
	
private:
	void EnsureValidLayers( void );

    // esture layer
    int  CalcFireAnim( bool bSecondary );
    int  CalcReloadAnim( void );
    void ComputeGestureSequence( CStudioHdr *pStudioHdr );
    void ComputeTurnSequence( CStudioHdr *pStudioHdr );
    
    // aim layer helper.
	const char* GetWeaponSuffix( void );
	bool HandleJumping( void );

	void UpdateLayerSequenceGeneric( CStudioHdr *pStudioHdr, int iLayer, bool &bEnabled, float &flCurCycle, int &iSequence, bool bWaitAtEnd );

	// more Layers
	virtual void ComputePoseParam_BodyYaw( void );

private:
	bool m_bValidLayers;

	// current state variables
	bool m_bJumping;
	float m_flJumpStartTime;
	bool m_bFirstJumpFrame;

	// gesture layer state
	bool m_bGesturing;
	float m_flGestureCycle;
	int m_iGestureSequence;

    // turn layer state
    float m_flTurnCycle;
    float m_flTurnRate;
    int m_iTurnSequence;
	
	IINSPlayerAnimStateHelper *m_pHelper;
};

//=========================================================
//=========================================================
float CINSPlayerAnimState::GetCurrentMaxGroundSpeed( void )
{
	Activity currentActivity = CalcMainActivity( );

    switch( currentActivity )
    {
    case ACT_INS_STAND:
	case ACT_INS_CROUCH:
	case ACT_INS_PRONE:
	case ACT_INS_STAND_AIM:
    case ACT_INS_CROUCH_AIM:
		return 1.0f;

    case ACT_INS_STAND_WALK:
        return ANIM_TOPSPEED_WALK;

    case ACT_INS_STAND_RUN:
        return ANIM_TOPSPEED_RUN;

	case ACT_INS_SPRINT:
		return ANIM_TOPSPEED_SPRINT;
    
    case ACT_INS_CROUCH_WALK:
		return ANIM_TOPSPEED_CROUCH_RUN;

	case ACT_INS_CROUCH_RUN:
        return ANIM_TOPSPEED_CROUCH_WALK;
    
    case ACT_INS_CRAWL:
        return ANIM_TOPSPEED_CRAWL;

	case ACT_INS_JUMP:
		return ANIM_TOPSPEED_JUMP;
	}

    return 0.0f;
}

//=========================================================
//=========================================================
IINSPlayerAnimState* CreatePlayerAnimState( CBaseAnimatingOverlay *pEntity, IINSPlayerAnimStateHelper *pHelper, LegAnimType_t legAnimType, bool bUseAimSequences )
{
	CINSPlayerAnimState *pRet = new CINSPlayerAnimState;
	pRet->InitSDK( pEntity, pHelper, legAnimType, bUseAimSequences );
	return pRet;
}

//=========================================================
//=========================================================
CINSPlayerAnimState::CINSPlayerAnimState( )
{
	m_pOuter = NULL;
	m_bGesturing = false;
	m_bValidLayers = false;

	// NOTE: WTF!!?? this should be prodecurly generated or something
	// something is fishy!!!
	m_flTurnCycle = 0.0f;

#ifdef _DEBUG

    for( int i = ACT_INS_STAND; i <= ACT_TRANS_PRONE_CROUCH; i++ )
	{
		int iIndex = UTIL_ConvertActivityToBaseIndex( ( Activity )i );

		if( iIndex == -1 )
		{
			Assert( false );
			return;
		}

		for( int j = 0; j < ANIMPREFIX_COUNT; j++ )
			Assert( g_AnimPrefixTable[ iIndex ][ j ] );
	}

#endif
}

//=========================================================
//=========================================================
void CINSPlayerAnimState::InitSDK( CBaseAnimatingOverlay *pEntity, IINSPlayerAnimStateHelper *pHelper, LegAnimType_t legAnimType, bool bUseAimSequences )
{
	CModAnimConfig config;
	config.m_flMaxBodyYawDegrees = 90;
	config.m_LegAnimType = legAnimType;
	config.m_bUseAimSequences = bUseAimSequences;

	m_pHelper = pHelper;

	BaseClass::Init( pEntity, config );
}

//=========================================================
//=========================================================
void CINSPlayerAnimState::UpdateMainSequence( void )
{
    CBaseAnimatingOverlay *pPlayer = GetOuter( );

	if( !pPlayer )
		return;

    float flCycle, flAimSequenceWeight;
    int iAnimDesired = CalcAimLayerSequence( &flCycle, &flAimSequenceWeight, false );
    
    if( pPlayer->GetSequenceActivity( pPlayer->GetSequence( ) ) != pPlayer->GetSequenceActivity( iAnimDesired ) )
		pPlayer->ResetSequence( iAnimDesired );
}

//=========================================================
//=========================================================
void CINSPlayerAnimState::ClearAnimationState( void )
{
	m_bJumping = false;
	m_bGesturing = false;
	
	BaseClass::ClearAnimationState( );
}

//=========================================================
//=========================================================
void CINSPlayerAnimState::DoAnimationEvent( PlayerAnimEvent_e event )
{
    switch( event )
    {
		case PLAYERANIMEVENT_WEAP_FIRE1:
	        m_bGesturing = true;
			m_iGestureSequence = CalcFireAnim( false );
			m_flGestureCycle = 0.0f;
	        return;

	    case PLAYERANIMEVENT_WEAP_FIRE2:
			m_bGesturing = true;
			m_iGestureSequence = CalcFireAnim( true );
			m_flGestureCycle = 0.0f;
			return;

		case PLAYERANIMEVENT_WEAP_RELOAD:
			m_bGesturing = true;
	        m_iGestureSequence = CalcReloadAnim( );
			m_flGestureCycle = 0.0f;
			return;

		case PLAYERANIMEVENT_PLAY_JUMP:
			m_bJumping = true;
			m_bFirstJumpFrame = true;
			m_flJumpStartTime = gpGlobals->curtime;
			return;
	}
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

void CINSPlayerAnimState::UpdateLayerSequenceGeneric( CStudioHdr *pStudioHdr, int iLayer, bool &bEnabled, float &flCurCycle, int &iSequence, bool bWaitAtEnd )
{
	if( !bEnabled )
		return;

	// increment the fire sequence's cycle.
	flCurCycle += m_pOuter->GetSequenceCycleRate( pStudioHdr, iSequence ) * gpGlobals->frametime;

	if( flCurCycle > 1 )
	{
		if( bWaitAtEnd )
		{
			flCurCycle = 1;
		}
		else
		{
			// not firing anymore
			bEnabled = false;
			iSequence = 0;
			return;
		}
	}

	// now dump the state into its animation layer
	C_AnimationLayer *pLayer = m_pOuter->GetAnimOverlay( iLayer );

	pLayer->m_flCycle = flCurCycle;
	pLayer->m_nSequence = iSequence;

	pLayer->m_flPlaybackRate = 1.0;
	pLayer->m_flWeight = 1.0f;
	pLayer->m_nOrder = iLayer;
}

#endif

//=========================================================
//=========================================================
extern ConVar mp_facefronttime;
extern ConVar mp_feetyawrate;

void CINSPlayerAnimState::ComputePoseParam_BodyYaw( void )
{
	// dind out which way he's running (m_flEyeYaw is the way he's looking)
	Vector vel;
	GetOuterAbsVelocity( vel );
	bool bIsMoving = vel.Length2D( ) > MOVING_MINIMUM_SPEED;

	// if we just initialized this guy (maybe he just came into the PVS), then immediately
	// set his feet in the right direction, otherwise they'll spin around from 0 to the 
	// right direction every time someone switches spectator targets.
	if( !m_bCurrentFeetYawInitialized )
	{
		m_bCurrentFeetYawInitialized = true;
		m_flGoalFeetYaw = m_flCurrentFeetYaw = m_flEyeYaw;
		m_flLastTurnTime = 0.0f;
	}
	else if( bIsMoving )
	{
		// player is moving, feet yaw = aiming yaw
		if( m_AnimConfig.m_LegAnimType == LEGANIM_9WAY || m_AnimConfig.m_LegAnimType == LEGANIM_8WAY )
		{
			// His feet point in the direction his eyes are, but they can run in any direction.
			m_flGoalFeetYaw = m_flEyeYaw;	
		}
		else
		{
			m_flGoalFeetYaw = RAD2DEG( atan2( vel.y, vel.x ) );

			// If he's running backwards, flip his feet backwards.
			Vector vEyeYaw( cos( DEG2RAD( m_flEyeYaw ) ), sin( DEG2RAD( m_flEyeYaw ) ), 0 );
			Vector vFeetYaw( cos( DEG2RAD( m_flGoalFeetYaw ) ), sin( DEG2RAD( m_flGoalFeetYaw ) ), 0 );

			if( vEyeYaw.Dot( vFeetYaw ) < -0.01 )
			{
				m_flGoalFeetYaw += 180;
			}
		}

	}
	else if( m_pHelper->INSAnim_CurrentStance( ) == STANCE_PRONE )
	{
		// when you're prone, your aim should always equal your feet
		m_flGoalFeetYaw = m_flEyeYaw;
	}
	else if ( ( gpGlobals->curtime - m_flLastTurnTime ) > mp_facefronttime.GetFloat( ) )
	{
		// player didn't move & turn for quite some time
		m_flGoalFeetYaw = m_flEyeYaw;
	}
	else
	{
		// if he's rotated his view further than the model can turn, make him face forward
		float flDiff = AngleNormalize( m_flGoalFeetYaw - m_flEyeYaw );

		if( fabs( flDiff ) > m_AnimConfig.m_flMaxBodyYawDegrees )
		{
			if( flDiff  > 0 )
				m_flGoalFeetYaw -= m_AnimConfig.m_flMaxBodyYawDegrees;
			else
				m_flGoalFeetYaw += m_AnimConfig.m_flMaxBodyYawDegrees;
		}
	}

	m_flGoalFeetYaw = AngleNormalize( m_flGoalFeetYaw );

	if( m_flCurrentFeetYaw != m_flGoalFeetYaw )
	{
        m_flTurnRate = m_flCurrentFeetYaw;

        ConvergeAngles( m_flGoalFeetYaw, mp_feetyawrate.GetFloat( ), m_AnimConfig.m_flMaxBodyYawDegrees,
			 gpGlobals->frametime, m_flCurrentFeetYaw );

		if( gpGlobals->frametime != 0.0f )
	        m_flTurnRate = ( ( m_flCurrentFeetYaw - m_flTurnRate ) / ( mp_feetyawrate.GetFloat() * gpGlobals->frametime ) ) * 90;
		else
			m_flTurnRate = 0.0f;

        m_flLastTurnTime = gpGlobals->curtime;
	}
    else
    {
        m_flTurnRate = 0.0f;
    }

	float flCurrentTorsoYaw = AngleNormalize( m_flEyeYaw - m_flCurrentFeetYaw );

	// rotate entire body into position
	m_angRender[ PITCH ] = m_angRender[ ROLL ] = 0;
	m_angRender[ YAW ] = m_flCurrentFeetYaw;

	// match the rotation to the floor
	/*if( m_pHelper->INSAnim_CurrentStance( ) == STANCE_PRONE && !m_pHelper->INSAnim_InStanceTransition( ) )
	{
		trace_t tr;
		Vector vecStart=pPlayer->GetAbsOrigin();
		Vector vecEnd(vecStart);
		vecEnd.z-=50;
		CTraceFilterSimple traceFilter(pPlayer,COLLISION_GROUP_NONE);

		UTIL_TraceLine(vecStart,vecEnd,MASK_PLAYERSOLID,&traceFilter,&tr);

		if (tr.DidHit())
		{
			QAngle angNorm;

#define ang2rad (2 * M_PI / 360)
			Vector angdir = Vector(
				cos(m_angRender.y * ang2rad) * cos(m_angRender.x * ang2rad),
				sin(m_angRender.y * ang2rad) * cos(m_angRender.x * ang2rad),
				-sin(m_angRender.x * ang2rad));
			Vector angdiry = Vector(
				sin(m_angRender.y * ang2rad) * cos(m_angRender.x * ang2rad),
				cos(m_angRender.y * ang2rad) * cos(m_angRender.x * ang2rad),
				-sin(m_angRender.x * ang2rad));

			VectorAngles(angdir - DotProduct(angdir, tr.plane.normal) * tr.plane.normal, m_angRender);
			VectorAngles(angdiry - DotProduct(angdiry, tr.plane.normal) * tr.plane.normal, angNorm);
			m_angRender.z=-angNorm.x;
		}
	}*/
	
	int yaw_rate = GetOuter()->LookupPoseParameter( "yaw_rate" );

	if( yaw_rate >= 0 )
	    GetOuter( )->SetPoseParameter( yaw_rate, m_flTurnRate );

	SetOuterBodyYaw( flCurrentTorsoYaw );
	g_flLastBodyYaw = flCurrentTorsoYaw;
}

//=========================================================
//=========================================================
void CINSPlayerAnimState::ComputeGestureSequence( CStudioHdr *pStudioHdr )
{
#ifdef CLIENT_DLL
	UpdateLayerSequenceGeneric( pStudioHdr, GESTURESEQUENCE_LAYER, m_bGesturing, m_flGestureCycle, m_iGestureSequence, false );
#endif
}

//=========================================================
//=========================================================
int CINSPlayerAnimState::CalcAimLayerSequence( float *flCycle, float *flAimSequenceWeight, bool bForceIdle )
{
    const char *pszSuffix = GetWeaponSuffix( );

    if( !pszSuffix )
        return 0;

#ifdef TESTING

	if( ignoreplayerseq.GetBool( ) )
		return 0;

#endif

    Activity act = GetCurrentMainSequenceActivity( );
	int iPlayerFlags = m_pHelper->INSAnim_GetPlayerFlags( );

    switch( act )
    {
		case ACT_TRANS_STAND_CROUCH: 
		{
			int iLeaningType = m_pHelper->INSAnim_LeaningType( );

			switch( iLeaningType )
			{
				case PLEANING_RIGHT:
				{
					act = ACT_INS_CROUCH_LEANR;
					break;
				}

				case PLEANING_LEFT:
				{
					act = ACT_INS_CROUCH_LEANL;
					break;
				}

				default:
				{
					if( iPlayerFlags & FL_PLAYER_IRONSIGHTS )
					{
						act = ACT_INS_CROUCH_AIM;
					}
					else
					{
						act = ACT_INS_CROUCH;
					}
				}
			}
		}

		case ACT_TRANS_CROUCH_STAND:
		{
			int iLeaningType = m_pHelper->INSAnim_LeaningType( );

			switch( iLeaningType )
			{
				case PLEANING_RIGHT:
				{
					act = ACT_INS_STAND_LEANR;
					break;
				}

				case PLEANING_LEFT:
				{
					act = ACT_INS_STAND_LEANL;
					break;
				}

				default:
				{
					if( iPlayerFlags & FL_PLAYER_IRONSIGHTS )
					{
						act = ACT_INS_STAND_AIM;
					}
					else
					{
						act = ACT_INS_STAND;
					}
				}
			}

			break;
		}

		case ACT_TRANS_STAND_PRONE:
		{
	        return CalcSequenceIndex( "Stand_to_Prone" );
		}

		case ACT_TRANS_CROUCH_PRONE:
		{
			return CalcSequenceIndex( "Crouch_to_Prone" );
		}

		case ACT_TRANS_PRONE_STAND:
		{
			return CalcSequenceIndex( "Prone_to_Stand" );
		}

		case ACT_TRANS_PRONE_CROUCH:
		{
	        return CalcSequenceIndex( "Prone_to_Crouch" );
		}
    }

	const char *pszPrefix = UTIL_IdleActionPrefix( act );

	if( !pszPrefix )
		return 0;

	return CalcSequenceIndex( "%s%s", pszPrefix, pszSuffix );
}

//=========================================================
//=========================================================
int CINSPlayerAnimState::CalcTurnLayerSequence( float *flCycle, float *flAimSequenceWeight, bool bForceIdle )
{
    switch( m_pHelper->INSAnim_CurrentStance( ) )
    {
		case STANCE_PRONE:
	        return CalcSequenceIndex( "Prone_Turn" );

	    case STANCE_CROUCH:
			return CalcSequenceIndex( "Crouch_Turn" );
    }

    return CalcSequenceIndex( "Stand_Turn" );
}

//=========================================================
//=========================================================
const char* CINSPlayerAnimState::GetWeaponSuffix( void )
{
	static const char *pszDefaultSuffix = "M16";

	CBaseCombatWeapon *pWeapon = m_pHelper->INSAnim_GetActiveWeapon( );

	if( !pWeapon )
		return pszDefaultSuffix;

	const char *pszSuffix = pWeapon->GetAnimSuffix( );

    if( pszSuffix[ 0 ] == '\0' )
        return pszDefaultSuffix;

	return pszSuffix;
}

//=========================================================
//=========================================================
bool CINSPlayerAnimState::HandleJumping( void )
{
	if( m_bJumping )
	{
		if( m_bFirstJumpFrame )
		{
			// reset the animation.
			m_bFirstJumpFrame = false;
			RestartMainSequence();	
		}

		// don't check if he's on the ground for a sec.. sometimes the client still has the
		// on-ground flag set right when the message comes in
		if( gpGlobals->curtime - m_flJumpStartTime > 0.2f )
		{
			if( m_pOuter->GetFlags() & FL_ONGROUND )
			{
				// reset the animation
				m_bJumping = false;
				RestartMainSequence( );
			}
		}
	}

	// are we still jumping? if so, keep playing the jump animation
	return m_bJumping;
}

//=========================================================
//=========================================================
Activity CINSPlayerAnimState::CalcMainActivity( void )
{
    float flOuterSpeed = GetOuterXYSpeed( );

    if( HandleJumping( ) )
        return ACT_INS_JUMP;

	if( m_pHelper->INSAnim_InStanceTransition( ) )
	{
		int iOldStance = m_pHelper->INSAnim_OldStance( );

		switch( m_pHelper->INSAnim_CurrentStance( ) )
		{
			case STANCE_PRONE:
			{
				return ( ( iOldStance == STANCE_CROUCH ) ? ACT_TRANS_CROUCH_PRONE : ACT_TRANS_STAND_PRONE );
			}

			case STANCE_CROUCH:
			{
				return ( ( iOldStance == STANCE_STAND ) ? ACT_TRANS_STAND_CROUCH : ACT_TRANS_PRONE_CROUCH );
			}

			case STANCE_STAND:
			{
				return ( ( iOldStance == STANCE_CROUCH ) ? ACT_TRANS_CROUCH_STAND : ACT_TRANS_PRONE_STAND );
            }
        }
	}

	int iPlayerFlags = m_pHelper->INSAnim_GetPlayerFlags( );

    switch( m_pHelper->INSAnim_CurrentStance( ) )
    {
		case STANCE_PRONE:
		{
			if( flOuterSpeed > MOVING_MINIMUM_SPEED )
				return ACT_INS_CRAWL;

			if( iPlayerFlags & FL_PLAYER_BIPOD )
				return ACT_INS_PRONE_DEPLOY;

			return ACT_INS_PRONE;
		}

		case STANCE_CROUCH:
		{
			bool bIsAiming = ( ( iPlayerFlags & FL_PLAYER_IRONSIGHTS ) != 0 );

			if( iPlayerFlags & FL_PLAYER_BIPOD )
			{
				return ACT_INS_CROUCH_DEPLOY;
			}
			else if( flOuterSpeed > MOVING_MINIMUM_SPEED )
			{
				if( bIsAiming )
					return ACT_INS_CROUCH_WALK;
				
				return ACT_INS_CROUCH_RUN;
			}
			else
			{
				int iLeaningType = m_pHelper->INSAnim_LeaningType( );

				switch( iLeaningType )
				{
					case PLEANING_RIGHT:
						return ACT_INS_CROUCH_LEANR;

					case PLEANING_LEFT:
						return ACT_INS_CROUCH_LEANL;
				}

				if( bIsAiming )
					return ACT_INS_CROUCH_AIM;

				return ACT_INS_CROUCH;
			}
		}

		case STANCE_STAND:
		{
			bool bIsAiming = ( ( iPlayerFlags & FL_PLAYER_IRONSIGHTS ) != 0 );

			if( iPlayerFlags & FL_PLAYER_BIPOD )
			{
				return ACT_INS_STAND_DEPLOY;
			}
			else if( flOuterSpeed > MOVING_MINIMUM_SPEED )
			{
				if( bIsAiming || iPlayerFlags & FL_PLAYER_WALKING )
					return ACT_INS_STAND_WALK;

				if( iPlayerFlags & FL_PLAYER_SPRINTING )
					return ACT_INS_SPRINT;

				return ACT_INS_STAND_RUN;
			}
			else
			{
				int iLeaningType = m_pHelper->INSAnim_LeaningType( );

				switch( iLeaningType )
				{
					case PLEANING_RIGHT:
						return ACT_INS_STAND_LEANR;

					case PLEANING_LEFT:
						return ACT_INS_STAND_LEANL;
				}

				if( bIsAiming )
					return ACT_INS_STAND_AIM;

				return ACT_INS_STAND;
			}
		}

		default:
		{
			Assert( false );
		}
	}

    return ACT_INVALID;
}

//=========================================================
//=========================================================
void CINSPlayerAnimState::DebugShowAnimState( int iStartLine )
{
#ifdef CLIENT_DLL

    engine->Con_NPrintf( iStartLine++, "Base anims speed: %.1f", 1.0f );
	engine->Con_NPrintf( iStartLine++, "gesture: %s, cycle: %.2f\n", m_bGesturing ? GetSequenceName( m_pOuter->GetModelPtr(), m_iGestureSequence ) : "[not gesturing]", m_flGestureCycle );
    engine->Con_NPrintf( iStartLine++, "turnseq: %s, cycle: %.2f\n", GetSequenceName( m_pOuter->GetModelPtr( ), m_iTurnSequence ), m_flTurnCycle );

	BaseClass::DebugShowAnimState( iStartLine );

#endif
}

//=========================================================
//=========================================================
void CINSPlayerAnimState::ComputeSequences( CStudioHdr *pStudioHdr )
{
	BaseClass::ComputeSequences( pStudioHdr );
	ComputeGestureSequence( pStudioHdr );
    ComputeTurnSequence( pStudioHdr );
    UpdateMainSequence( );
}

//=========================================================
//=========================================================
void CINSPlayerAnimState::ComputeTurnSequence( CStudioHdr *pStudioHdr )
{
#ifdef CLIENT_DLL

    bool enabled = true;
    m_iTurnSequence = CalcTurnLayerSequence( &m_flTurnCycle, NULL, false );
    UpdateLayerSequenceGeneric( pStudioHdr, TURNSEQUENCE_LAYER, enabled, m_flTurnCycle, m_iTurnSequence, false );

#endif
}

//=========================================================
//=========================================================
void CINSPlayerAnimState::ClearAnimationLayers( void )
{
	VPROF( "CBasePlayerAnimState::ClearAnimationLayers" );
	ClearAnimationLayers( false );
}

//=========================================================
//=========================================================
void CINSPlayerAnimState::ClearAnimationLayers( bool bForce )
{
	if( !bForce && m_pOuter->GetNumAnimOverlays( ) == 0 )
		return;

	if( !m_pOuter )
		return;

	m_pOuter->SetNumAnimOverlays( NUM_LAYERS_WANTED );

	for( int i = 0; i < m_pOuter->GetNumAnimOverlays( ); i++ )
	{
		m_pOuter->GetAnimOverlay( i )->SetOrder( CBaseAnimatingOverlay::MAX_OVERLAYS );

	#ifdef GAME_DLL

		m_pOuter->GetAnimOverlay( i )->m_fFlags = 0;

	#endif
	}
}

//=========================================================
//=========================================================
int CINSPlayerAnimState::CalcFireAnim( bool bSecondary )
{
    const char *pszSuffix = GetWeaponSuffix( );

    if( !pszSuffix )
        return -1;

	const char *pszPrefix = UTIL_FireActionPrefix( GetCurrentMainSequenceActivity( ), bSecondary );

	if( !pszPrefix )
		return -1;

    return CalcSequenceIndex( "%s%s", pszPrefix, pszSuffix );
}

//=========================================================
//=========================================================
int CINSPlayerAnimState::CalcReloadAnim( void )
{
    const char *pszSuffix = GetWeaponSuffix( );

    if( !pszSuffix )
        return -1;

	const char *pszPrefix = UTIL_ReloadActionPrefix( GetCurrentMainSequenceActivity( ) );

	if( !pszPrefix )
		return -1;

    return CalcSequenceIndex( "%s%s", pszPrefix, pszSuffix );
}

//=========================================================
//=========================================================
void CINSPlayerAnimState::EnsureValidLayers( void )
{
	if( m_bValidLayers )
		return;

	ClearAnimationLayers( true );

	m_bValidLayers = true;
}